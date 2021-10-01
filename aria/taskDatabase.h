#ifndef TASKDATABASE_H
#define TASKDATABASE_H

#include "taskInfo.h"

class sqlite3;

class TaskDatabase
{
public:
	TaskDatabase();

	~TaskDatabase();

	uint64_t findTask(Task *);

	uint64_t addTask(Task *);

	void addToDownloading(uint64_t, aria2::A2Gid);

	void addToCompleted(aria2::A2Gid);

protected:
private:
	sqlite3 *_sql;
};

#endif // TASKDATABASE_H
