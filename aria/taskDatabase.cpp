#include "taskDatabase.h"

#include <sqlite3.h>

char exeLang[8192];

taskDatabase::taskDatabase()
{
	sqlite3 *_sql = nullptr;
	sqlite3_open("./aria.db", &_sql);
}

taskDatabase::~taskDatabase()
{
	sqlite3_close(_sql);
}

void taskDatabase::addToTask(Task *tsk)
{
	if(_sql == nullptr)
		return;
	auto uri = tsk->getUri();
	const char lang[] = "insert into task_table (link = '%s')";
	sprintf(exeLang, lang, uri.c_str());
	sqlite3_exec(_sql, exeLang, nullptr, nullptr, nullptr);
}

void taskDatabase::addToDownloading(aria2::A2Gid)
{

}

void taskDatabase::addToCompleted(aria2::A2Gid)
{

}

bool taskDatabase::findUrl(const std::string &)
{
	return false;
}
