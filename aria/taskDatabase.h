#ifndef TASKDATABASE_H
#define TASKDATABASE_H

#include "taskInfo.h"
#include <QObject>

#include <map>

class sqlite3;

class AriaUi;

class TaskDatabase : public QObject
{
	Q_OBJECT
public:
	TaskDatabase();

	~TaskDatabase();

	uint64_t findTask(Task *);

	uint64_t addTask(Task *);

	void downloadTask(uint64_t, aria2::A2Gid);

	void completeTask(aria2::A2Gid);

	void deleteTask(aria2::A2Gid);

	void removeLocalFile(uint64_t ide);

	void deleteCompleteTask(uint64_t, bool removeLocalFile = false);

	void deleteTrashTask(uint64_t, bool removeLocalFile = false);

	void failTask(aria2::A2Gid);

	void updateTaskInfo(aria2::A2Gid, TaskInfoEx &);

	void initDownloadTask();

	void initCompleteTask();

	void initTrashTask();

	void addLocalTask(uint64_t, int);

protected:
private:
	sqlite3 *_sql;

	std::map<uint64_t, uint64_t> _idTable;
};

#endif // TASKDATABASE_H
