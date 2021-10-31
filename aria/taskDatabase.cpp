#include "taskDatabase.h"

#include <QDateTime>
#include <sqlite3.h>
#include <filesystem>

#include "boost/algorithm/string.hpp"

#include "ariaUi.h"
#include "ariaListWgt.h"
#include "ariaSetting.h"

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
}

TaskDatabase::~TaskDatabase()
{
	sqlite3_close(_sql);
}

uint64_t TaskDatabase::findTask(Task *task)
{
	if(_sql == nullptr)
		return 0;
	const char lang[] = "select rowid from task_table where (link = '%s');";
	sprintf(exeLang, lang, task->getUri().c_str());
	sqlite3_stmt *stmt; const char *tail;
	auto rc = sqlite3_prepare_v2(_sql, exeLang, -1, &stmt, &tail);
	if(rc != SQLITE_OK)
		return 0;
	rc = sqlite3_step(stmt);
	uint64_t ret = 0;
	if(rc == SQLITE_ROW)
		ret = sqlite3_column_int64(stmt, 0);
	sqlite3_finalize(stmt);
	return ret;
}

uint64_t TaskDatabase::addTask(Task *tsk)
{
	if(_sql == nullptr)
		return 0;
	auto uri = tsk->getUri();
	auto opts = optToString(tsk->opts);
	const char lang1[] = "insert into task_table (type, name, state, link, start_time, state, options) values (%d, '%s', %d, '%s', '%s', 0, '%s');";
	QString datatime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	sprintf(exeLang, lang1, tsk->type, tsk->name.c_str(), tsk->state, uri.c_str(), datatime.toLocal8Bit().data(), opts.c_str());
	if(sqlite3_exec(_sql, exeLang, nullptr, nullptr, nullptr) != SQLITE_OK)
		return 0;

	sqlite3_stmt *stmt;
	const char lang2[] = "select last_insert_rowid();";
	sqlite3_prepare_v2(_sql, lang2, -1, &stmt, 0);
	uint64_t ret = 0;
	if(sqlite3_step(stmt) == SQLITE_ROW)
		ret = sqlite3_column_int64(stmt, 0);
	sqlite3_finalize(stmt);
	return ret;
}

void TaskDatabase::downloadTask(uint64_t id, aria2::A2Gid gid)
{
	if(_sql == nullptr)
		return;
	const char lang1[] = "update task_table set gid = %lld, state=%d where id = %d;";
	sprintf(exeLang, lang1, gid, aria2::DOWNLOAD_WAITING, id);
	sqlite3_exec(_sql, exeLang, 0, 0, 0);

	const char lang2[] = "insert into dn_table(id) values(%lld);";
	sprintf(exeLang, lang2, id);
	sqlite3_exec(_sql, exeLang, 0, 0, 0);

	_idTable[gid] = id;
}

void TaskDatabase::completeTask(aria2::A2Gid gid)
{
	if(_sql == nullptr)
		return;
	auto id = _idTable[gid];

	const char lang1[] = "delete from dn_table where id = %lld;";
	sprintf(exeLang, lang1, id);
	sqlite3_exec(_sql, exeLang, 0, 0, 0);

	const char lang2[] = "insert into cm_table values (%lld);";
	sprintf(exeLang, lang2, id);
	sqlite3_exec(_sql, exeLang, 0, 0, 0);

	addLocalTask(id, COMPLETED);
}

void TaskDatabase::deleteTask(aria2::A2Gid gid)
{
	if(_sql == nullptr)
		return;
	auto id = _idTable[gid];

	const char lang1[] = "delete from dn_table where id = %lld;";
	sprintf(exeLang, lang1, id);
	sqlite3_exec(_sql, exeLang, 0, 0, 0);

	const char lang2[] = "insert into tr_table values (%lld);";
	sprintf(exeLang, lang2, id);
	sqlite3_exec(_sql, exeLang, 0, 0, 0);

	addLocalTask(id, TRASHCAN);
}

void TaskDatabase::failTask(aria2::A2Gid gid)
{
	if(_sql == nullptr)
		return;
	auto id = _idTable[gid];

	const char lang[] = "update task_table set state=%d,end_time='%s' where id=%lld;";
	QString datatime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	sprintf(exeLang, lang, aria2::DOWNLOAD_ERROR , datatime.toLocal8Bit().data(), id);
	sqlite3_exec(_sql, exeLang, 0, 0, 0);
}

void TaskDatabase::updateTaskInfo(aria2::A2Gid gid, TaskInfoEx &taskInfo)
{
	if(_sql == nullptr)
		return;

	auto id = _idTable[gid];
	auto &filedata = taskInfo.fileData;
	std::string filepath;
	if(filedata.size() == 1) {
		filepath = filedata[0].path;
	}else{

	}

	std::vector<std::string> vals;
	for(auto &keyval : taskInfo.opts)
		vals.push_back(keyval.first + "=" + keyval.second);
	std::string opts = boost::algorithm::join(vals, "|");

	if(filepath.empty())
	{
		const char lang[] = "update task_table set state=%d, total_size=%lld, options='%s' where id=%lld;";
		sprintf(exeLang, lang, taskInfo.state, taskInfo.totalLength, opts.c_str(), id);
	} else{
		std::filesystem::path lpath(filepath);
		std::string filename = lpath.filename().string();
		QString datatime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
		const char lang[] = "update task_table set name='%s', state=%d, total_size=%lld, end_time='%s', local_path='%s', options='%s' where id=%lld;";
		sprintf(exeLang, lang, filename.c_str(), taskInfo.state, taskInfo.totalLength, datatime.toLocal8Bit().data(), filepath.c_str(), opts.c_str(), id);

	}
	sqlite3_exec(_sql, exeLang, 0, 0, 0);
}

void TaskDatabase::initDownloadTask()
{
	if(_sql == nullptr)
		return;
	const char lang[] = "select task_table.id, gid, type, name, state, link, local_path, options from task_table inner join dn_table on dn_table.id = task_table.id;";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(_sql, lang, -1, &stmt, 0);
	while(sqlite3_step(stmt) == SQLITE_ROW){
		auto id = sqlite3_column_int64(stmt, 0);
		auto gid = sqlite3_column_int64(stmt, 1);
		auto ty = sqlite3_column_int(stmt, 2);
		auto na = sqlite3_column_text(stmt, 3);
		auto st = sqlite3_column_int(stmt, 4);
		std::string lk, lp, op;
		auto chlk = sqlite3_column_text(stmt, 5);
		if(chlk) lk = (const char *)chlk;
		auto chlp = sqlite3_column_text(stmt, 6);
		if(chlp) lp = (const char *)chlp;
		auto chop = sqlite3_column_text(stmt, 7);
		if(chop) op = (const char *)chop;
		std::unique_ptr<Task> tsk;
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
		tsk->name = (const char *)na;
		tsk->rid = id;
		tsk->state = st;
		tsk->type = ty;
		tsk->opts = optToKV(op);
		{
			char chgid[17] = {0}; sprintf(chgid, "%llX", gid);
			tsk->opts.push_back(std::make_pair("gid", chgid));
		}
		AriaDlg::getMainDlg()->addUriTask(tsk);
		{
			TaskInfo tskInfo;
			//tskInfo.totalLength = ;
			//tskInfo.dnloadLength = ;
			//AriaDlg::getMainDlg()->getEmitter()->updateTaskSig(id, tskInfo);
		}
		_idTable[gid] = id;
	}
	sqlite3_finalize(stmt);
}

void TaskDatabase::initCompleteTask()
{
	if(_sql == nullptr)
		return;

	const char lang[] = "select id from cm_table;";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(_sql, lang, -1, &stmt, 0);
	while(sqlite3_step(stmt) == SQLITE_ROW){
		auto id = sqlite3_column_int64(stmt, 0);
		addLocalTask(id, COMPLETED);
	}
	sqlite3_finalize(stmt);
}

void TaskDatabase::initTrashTask()
{
	if(_sql == nullptr)
		return;

	const char lang[] = "select id from tr_table;";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(_sql, lang, -1, &stmt, 0);
	while(sqlite3_step(stmt) == SQLITE_ROW){
		auto id = sqlite3_column_int64(stmt, 0);
		addLocalTask(id, TRASHCAN);
	}
	sqlite3_finalize(stmt);
}

void TaskDatabase::addLocalTask(uint64_t id, int wgtType)
{
	const char lang[] = "select name, total_size, end_time, local_path from task_table where id = %lld;";
	sprintf(exeLang, lang, id);
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(_sql, exeLang, -1, &stmt, 0);
	while(sqlite3_step(stmt) == SQLITE_ROW){
		auto na = sqlite3_column_text(stmt, 0);
		auto sz = sqlite3_column_int64(stmt, 1);
		auto tm = sqlite3_column_text(stmt, 2);
		auto lp = sqlite3_column_text(stmt, 3);
		QString name = QString::fromLocal8Bit((char *)na);
		QDateTime dt = QDateTime::fromString((char *)tm);
		FinishTaskInfo info; info.id = id;
		info.name = name; info.size = sz;
		info.localPath = QString((const char *)lp);
		info.datetime = AriaDlg::getMainDlg()->locale().toString(dt);
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
