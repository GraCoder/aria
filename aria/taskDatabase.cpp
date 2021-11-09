#include "taskDatabase.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <sqlite3.h>
#include <filesystem>

#include "boost/algorithm/string.hpp"

#include "ariaUi.h"
#include "ariaListWgt.h"
#include "ariaSetting.h"

#include "aria2.h"

char exeLang[8192];

std::string optToString(const aria2::KeyVals &kvs)
{
	std::vector<std::string> opts;
	for(auto &kv : kvs)
		opts.push_back(kv.first + "=" + kv.second);
	return boost::algorithm::join(opts, "|");
}

aria2::KeyVals optToKV(const std::string &opts)
{
	aria2::KeyVals ret;
	std::vector<std::string> ops;
	boost::algorithm::split(ops, opts, boost::is_any_of("|"));
	for(auto &optmp : ops){
		std::vector<std::string> opkv;
		boost::algorithm::split(opkv, optmp, boost::is_any_of("="));
		if(opkv.size() != 2) continue;
		ret.push_back(std::make_pair(opkv[0], opkv[1]));
	}
	return ret;
}

TaskDatabase::TaskDatabase()
{
	_sql = nullptr;
	auto appPath = ariaSetting::instance().appPath();
#ifdef NDEBUG
	sqlite3_open((appPath + "/aria.db").c_str(), &_sql);
#else
	sqlite3_open("d:/dev/aria/aria.db", &_sql);
#endif
	char lang[] = "PRAGMA foreign_keys=1";
	sqlite3_exec(_sql, lang, nullptr, nullptr, nullptr);
}

TaskDatabase::~TaskDatabase()
{
	sqlite3_close(_sql);
}

aria2::A2Gid TaskDatabase::addTask(Task *tsk)
{
	if(_sql == nullptr)
		return 0;
	auto uri = tsk->getUri();
	auto opts = optToString(tsk->opts);
	const char lang1[] = "insert into task_table (gid, type, name, state, link_path, start_time, state, options) values (%lld, %d, '%s', %d, '%s', '%s', 0, '%s');";
	QString datatime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	sprintf(exeLang, lang1, tsk->id, tsk->type, tsk->name.c_str(), tsk->state, uri.c_str(), datatime.toLocal8Bit().data(), opts.c_str());
	if(sqlite3_exec(_sql, exeLang, nullptr, nullptr, nullptr) != SQLITE_OK)
		return 0;
	return tsk->id;
}

void TaskDatabase::completeTask(aria2::A2Gid gid)
{
	if(_sql == nullptr)
		return;

	setState(gid, aria2::DOWNLOAD_COMPLETE);
	addLocalTask(gid, COMPLETED);
}

void TaskDatabase::trashTask(aria2::A2Gid gid)
{
	if(_sql == nullptr)
		return;

	setState(gid, aria2::DOWNLOAD_REMOVED);
	addLocalTask(gid, TRASHCAN);
}

void TaskDatabase::deleteTask(aria2::A2Gid gid)
{
	if(_sql == nullptr)
		return;

	const char lang[] = "delete from task_table where gid = %lld;";
	sprintf(exeLang, lang, gid);
	sqlite3_exec(_sql, exeLang, 0, 0, 0);
}

void TaskDatabase::removeLocalFile(uint64_t gid)
{
	const char lang1[] = "select type, local_path from task_table where gid = %lld;";
	sprintf(exeLang, lang1, gid);
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(_sql, exeLang, -1, &stmt, 0);
	if(sqlite3_step(stmt) == SQLITE_ROW){
		auto type = sqlite3_column_int(stmt, 0);
		auto localf = sqlite3_column_text(stmt, 1);
		QString localfile;
		if(localf) localfile = QString::fromLocal8Bit((const char *)localf);
		switch(type)
		{
		case 1:
			QFile(localfile).remove();
			break;
		}
	}
}

void TaskDatabase::deleteFinishTask(aria2::A2Gid gid, bool rmLocalFile)
{
	if(_sql == nullptr)
		return;

	if(rmLocalFile)
		removeLocalFile(gid);

	const char lang2[] = "delete from task_table where gid = %lld;";
	sprintf(exeLang, lang2, gid);
	sqlite3_exec(_sql, exeLang, 0, 0, 0);
}

void TaskDatabase::failTask(aria2::A2Gid gid)
{
	if(_sql == nullptr)
		return;

	const char lang[] = "update task_table set state=%d,end_time='%s' where gid=%lld;";
	QString datatime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	sprintf(exeLang, lang, aria2::DOWNLOAD_ERROR , datatime.toLocal8Bit().data(), gid);
	sqlite3_exec(_sql, exeLang, 0, 0, 0);
}

void TaskDatabase::updateTaskInfo(aria2::A2Gid gid, TaskInfoEx &taskInfo)
{
	if(_sql == nullptr)
		return;

	char *errmsg = nullptr;
	sqlite3_exec(_sql, "begin transaction", 0, 0, &errmsg);

	{
		std::string opts = optToString(taskInfo.opts);
		const char lang[] = "update task_table set state=%d, options='%s' where gid = %lld;";
		sprintf(exeLang, lang, taskInfo.state, opts.c_str(), gid);
		sqlite3_exec(_sql, exeLang, 0, 0, 0);
	}

	if(taskInfo.totalLength > 0) {
		const char lang[] = "update task_table set total_size=%lld, down_size=%lld where gid=%lld;";
		sprintf(exeLang, lang, taskInfo.totalLength, taskInfo.dnloadLength, gid);
		sqlite3_exec(_sql, exeLang, 0, 0, 0);
	}

	{
		auto &filedata = taskInfo.fileData;
		std::string filepath;
		if(filedata.size() == 1) {
			filepath = filedata[0].path;
		}else{
			filepath = filedata[0].path;
			filepath = std::filesystem::path(filepath).parent_path().string();
		}

		if(!filepath.empty()) {
			QString datatime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
			const char lang[] = "update task_table set end_time='%s', local_path='%s' where gid=%lld;";
			sprintf(exeLang, lang, datatime.toLocal8Bit().data(), filepath.c_str(), gid);
			sqlite3_exec(_sql, exeLang, 0, 0, 0);
		}
	}

	sqlite3_exec(_sql, "commit transaction" , 0, 0, &errmsg);
}

void TaskDatabase::initDownloadTask()
{
	if(_sql == nullptr)
		return;
	const char lang[] = "select gid, state from task_table where state = 0 or state = 1 or state = 2 or state = 4;";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(_sql, lang, -1, &stmt, 0);
	std::vector<uint64_t> ids;
	while(sqlite3_step(stmt) == SQLITE_ROW){
		auto id = sqlite3_column_int64(stmt, 0);
		ids.push_back(id);
	}
	sqlite3_finalize(stmt);
	std::vector<std::unique_ptr<Task>> tasks;
	for(auto id : ids){
		auto tsk = createTask(id);
		if(tsk == nullptr)
			continue;
		tasks.push_back(std::move(tsk));
	}
	AriaDlg::getMainDlg()->addTask(tasks);
}

std::unique_ptr<Task>
TaskDatabase::createTask(aria2::A2Gid id, bool fresh)
{
	const char lang[] = "select type, name, state, total_size, down_size, up_size,"
						"link_path, local_path, options from task_table where gid=%lld;";
	sprintf(exeLang, lang, id);
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(_sql, exeLang, -1, &stmt, 0);
	std::unique_ptr<Task> tsk;
	if(sqlite3_step(stmt) == SQLITE_ROW){
		auto ty = sqlite3_column_int(stmt, 0);
		auto na = sqlite3_column_text(stmt, 1);
		auto st = sqlite3_column_int(stmt, 2);
		auto sz = sqlite3_column_int64(stmt, 3);
		auto dz = sqlite3_column_int64(stmt, 4);
		auto uz = sqlite3_column_int64(stmt, 5);
		std::string lk, lp, op;
		auto chlk = sqlite3_column_text(stmt, 6);
		if(chlk) lk = (const char *)chlk;
		auto chlp = sqlite3_column_text(stmt, 7);
		if(chlp) lp = (const char *)chlp;
		auto chop = sqlite3_column_text(stmt, 8);
		if(chop) op = (const char *)chop;
		switch(ty){
		case 1:
		{
			auto task = std::make_unique<UriTask>();
			task->url = lk;
			tsk = std::move(task);
			break;
		}
		case 2:
			auto task = std::make_unique<BtTask>();
			task->torrent = lk;
			tsk = std::move(task);
			break;
		}
		tsk->id = id;
		tsk->name = (const char *)na;
		tsk->type = ty;
		tsk->to_size = sz;
		tsk->dn_size = dz;
		tsk->up_size = uz;
		tsk->opts = optToKV(op);

		if(fresh) {
			tsk->id = 0;
			tsk->state = aria2::DOWNLOAD_WAITING;
		}else {
			tsk->state = st;
			tsk->opts.push_back(std::make_pair("gid", aria2::gidToHex(id)));
		}
	}
	sqlite3_finalize(stmt);
	return tsk;
}

void TaskDatabase::initCompleteTask()
{
	if(_sql == nullptr)
		return;

	const char lang[] = "select gid from task_table where state = 3;";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(_sql, lang, -1, &stmt, 0);
	while(sqlite3_step(stmt) == SQLITE_ROW){
		auto id = sqlite3_column_int64(stmt, 0);
		aria2::holdPlace(id);
		addLocalTask(id, COMPLETED);
	}
	sqlite3_finalize(stmt);
}

void TaskDatabase::initTrashTask()
{
	if(_sql == nullptr)
		return;

	const char lang[] = "select gid from task_table where state = 5;";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(_sql, lang, -1, &stmt, 0);
	while(sqlite3_step(stmt) == SQLITE_ROW){
		auto id = sqlite3_column_int64(stmt, 0);
		aria2::holdPlace(id);
		addLocalTask(id, TRASHCAN);
	}
	sqlite3_finalize(stmt);
}

void TaskDatabase::addLocalTask(uint64_t id, int wgtType)
{
	const char lang[] = "select name, type, total_size, end_time, local_path from task_table where gid = %lld;";
	sprintf(exeLang, lang, id);
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(_sql, exeLang, -1, &stmt, 0);
	while(sqlite3_step(stmt) == SQLITE_ROW){
		auto na = sqlite3_column_text(stmt, 0);
		auto ty = sqlite3_column_int(stmt, 1);
		auto sz = sqlite3_column_int64(stmt, 2);
		auto tm = sqlite3_column_text(stmt, 3);
		auto lp = sqlite3_column_text(stmt, 4);
		QString name = QString::fromLocal8Bit((char *)na);
		FinishTaskInfo info; info.id = id;
		info.name = name; info.size = sz;
		info.localPath = (const char *)lp;
		info.datetime = (const char *)tm;
		if(ty == 1)
			info.iconType = TaskInfo::surfixToInt(QFileInfo(name).suffix());
		else
			info.iconType = ty;
		if(wgtType == COMPLETED)
		{
			auto wgt = AriaDlg::getMainDlg()->getCompleteWgt();
			wgt->addFinishTaskSlt(id, info);
		}else if(wgtType == TRASHCAN)
		{
			auto wgt = AriaDlg::getMainDlg()->getTrashWgt();
			wgt->addFinishTaskSlt(id, info);
		}
	}
	sqlite3_finalize(stmt);
}

std::unique_ptr<Task>
TaskDatabase::findTask(const std::string &local, const std::string &uri)
{
	if(_sql == nullptr)
		return 0;
	const char lang[] = "select gid from task_table where (local_path = '%s');";
	sprintf(exeLang, lang, local.c_str());
	sqlite3_stmt *stmt; const char *tail;
	auto rc = sqlite3_prepare_v2(_sql, exeLang, -1, &stmt, &tail);
	if(rc != SQLITE_OK)
		return 0;
	rc = sqlite3_step(stmt);
	uint64_t ret = 0;
	if(rc == SQLITE_ROW)
		ret = sqlite3_column_int64(stmt, 0);
	sqlite3_finalize(stmt);

	return createTask(ret);
}

std::map<std::string, std::string> TaskDatabase::getSettings()
{
	std::map<std::string, std::string> ret;
	if(_sql == nullptr)
		return ret;
	const char lang[] = "select * from setting;";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(_sql, lang, -1, &stmt, 0);
	while(sqlite3_step(stmt) == SQLITE_ROW){
		std::string ky = (const char *)sqlite3_column_text(stmt, 0);
		std::string ve = (const char *)sqlite3_column_text(stmt, 1);
		//sqlite3_column_text(stmt, 2);
		ret[ky] = ve;
	}
	sqlite3_finalize(stmt);
	return ret;
}

void TaskDatabase::setState(aria2::A2Gid id, int state)
{
	const char lang[] = "update task_table set state = %d where gid = %lld";
	sprintf(exeLang, lang, state, id);
	sqlite3_exec(_sql, exeLang, nullptr, nullptr, nullptr);
}
