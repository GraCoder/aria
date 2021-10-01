#include "taskDatabase.h"

#include <sqlite3.h>
#include <QDateTime>

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
	if(rc == SQLITE_ROW){
		auto rowid = sqlite3_column_int64(stmt, 0);
		return rowid;
	}
	else
		return 0;
}

uint64_t TaskDatabase::addTask(Task *tsk)
{
	if(_sql == nullptr)
		return 0;
	auto uri = tsk->getUri();
	const char lang1[] = "insert into task_table (link, start_time) values ('%s', '%s');";
	QString datatime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	sprintf(exeLang, lang1, uri.c_str(), datatime.toLocal8Bit().data());
	if(sqlite3_exec(_sql, exeLang, nullptr, nullptr, nullptr) != SQLITE_OK)
		return 0;

	sqlite3_stmt *stmt;
	const char lang2[] = "select last_insert_rowid();";
	sqlite3_prepare_v2(_sql, lang2, -1, &stmt, 0);
	if(sqlite3_step(stmt) != SQLITE_ROW)
		return 0;
	auto rowid = sqlite3_column_int64(stmt, 0);
	return rowid;
}

void TaskDatabase::addToDownloading(uint64_t id, aria2::A2Gid gid)
{
	if(_sql == nullptr)
		return;
	const char lang1[] = "update task_table set gid = %lld where rowid = %d;";
	sprintf(exeLang, lang1, gid, id);
	sqlite3_exec(_sql, exeLang, 0, 0, 0);

	const char lang2[] = "insert into dn_table(gid, status) values(%lld, true);";
	sprintf(exeLang, lang2, gid);
	sqlite3_exec(_sql, exeLang, 0, 0, 0);
}

void TaskDatabase::addToCompleted(aria2::A2Gid gid)
{
	if(_sql == nullptr)
		return;
	const char lang1[] = "delete from dn_table where gid = %lld;";
	sprintf(exeLang, lang1, gid);
	sqlite3_exec(_sql, exeLang, 0, 0, 0);

	const char lang2[] = "insert into cm_table values (%lld)";
	sprintf(exeLang, lang2, gid);
	sqlite3_exec(_sql, exeLang, 0, 0, 0);
}
