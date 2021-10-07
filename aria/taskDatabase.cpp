#include "taskDatabase.h"

#include <sqlite3.h>
#include <QDateTime>

#include "ariaUi.h"
#include "ariaListWgt.h"

char exeLang[8192];

TaskDatabase::TaskDatabase()
{
	_sql = nullptr;
	sqlite3_open("d:/dev/aria/aria.db", &_sql);
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
	const char lang1[] = "insert into task_table (type, name, state, link, start_time, state) values (%d, '%s', %d, '%s', '%s', 0);";
	QString datatime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	sprintf(exeLang, lang1, tsk->type, tsk->name.c_str(), tsk->state, uri.c_str(), datatime.toLocal8Bit().data());
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

void TaskDatabase::updateTaskInfo(aria2::A2Gid gid, TaskInfo &taskInfo)
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


	QString datatime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	const char lang[] = "update task_table set state=%d, total_size=%lld, end_time='%s', local_path='%s' where id=%lld;";
	sprintf(exeLang, lang, taskInfo.state, taskInfo.totalLength, datatime.toLocal8Bit().data(), filepath.c_str(), id);
	sqlite3_exec(_sql, exeLang, 0, 0, 0);
}

void TaskDatabase::initDownloadTask()
{
	if(_sql == nullptr)
		return;
	auto wgt = AriaDlg::getMainDlg()->getDownloadWgt();
	const char lang[] = "select task_table.id, gid, name, state, link, local_path from task_table inner join dn_table on dn_table.id = task_table.id;";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(_sql, lang, -1, &stmt, 0);
	while(sqlite3_step(stmt) == SQLITE_ROW){
		auto id = sqlite3_column_int64(stmt, 0);
		auto gid = sqlite3_column_int64(stmt, 1);
		auto na = sqlite3_column_text(stmt, 2);
		auto st = sqlite3_column_int(stmt, 3);
		QString name = QString::fromLocal8Bit((char *)na);
		wgt->addTaskSlt(gid, name);
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
