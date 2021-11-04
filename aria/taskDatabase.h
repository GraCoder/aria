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

	aria2::A2Gid addTask(Task *);

	void completeTask(aria2::A2Gid);

	void trashTask(aria2::A2Gid);

	void deleteTask(aria2::A2Gid);

	void restartTask(aria2::A2Gid);

	void removeLocalFile(aria2::A2Gid ide);

	void deleteFinishTask(aria2::A2Gid, bool removeLocalFile = false);

	void failTask(aria2::A2Gid);

	void updateTaskInfo(aria2::A2Gid, TaskInfoEx &);

	void initDownloadTask();

	TaskInfoEx getTaskInfo(aria2::A2Gid);

	std::unique_ptr<Task> createTask(aria2::A2Gid);

	void initCompleteTask();

	void initTrashTask();

	void addLocalTask(aria2::A2Gid, int);


	std::unique_ptr<Task> findTask(const std::string &local, const std::string &uri = "");

protected:
private:
	sqlite3 *_sql;
};

#endif // TASKDATABASE_H
