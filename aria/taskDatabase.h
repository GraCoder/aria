#ifndef TASKDATABASE_H
#define TASKDATABASE_H

#include "taskInfo.h"

class sqlite3;

class taskDatabase
{
public:
	taskDatabase();

	~taskDatabase();

	void addToTask(Task *);

	void addToDownloading(aria2::A2Gid);

	void addToCompleted(aria2::A2Gid);

	bool findUrl(const std::string &);
protected:
private:
	sqlite3 *_sql;
};

#endif // TASKDATABASE_H
