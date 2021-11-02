#ifndef TASKDATABASE_H
#define TASKDATABASE_H

#include "taskInfo.h"
#include <QObject>

#include <map>
#include <memory>

class sqlite3;

class Task;
class AriaUi;

class TaskDatabase : public QObject
{
	Q_OBJECT
public:
	TaskDatabase();

	~TaskDatabase();

	uint64_t findTask(Task *);

	uint64_t addTask(Task *);

	aria2::A2Gid getGid(uint64_t);

	void downloadTask(uint64_t, aria2::A2Gid);

	void completeTask(aria2::A2Gid);

	void trashTask(aria2::A2Gid);

	void deleteTask(aria2::A2Gid);

	void restartTask(uint64_t);

	void removeLocalFile(uint64_t ide);

	void deleteCompleteTask(uint64_t, bool removeLocalFile = false);

	void deleteTrashTask(uint64_t, bool removeLocalFile = false);

	void failTask(aria2::A2Gid);

	void updateTaskInfo(aria2::A2Gid, TaskInfoEx &);

	void initDownloadTask();

	TaskInfoEx getTaskInfo(uint64_t);

	std::unique_ptr<Task> createTask(uint64_t);

	void initCompleteTask();

	void initTrashTask();

	void addLocalTask(uint64_t, int);

protected:
private:
	sqlite3 *_sql;

	std::map<uint64_t, uint64_t> _idTable;
};

#endif // TASKDATABASE_H
