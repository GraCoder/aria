#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <memory>
#include <thread>

#include <QDialog>
#include <QHash>

#include <aria2/aria2.h>

#include "frameless.h"
#include "taskInfo.h"

class QListWidget;
class QListWidgetItem;
class AriaListWidget;

class SpinLock
{
public:
	SpinLock() : _lock(ATOMIC_FLAG_INIT)
	{}

	void lock()
	{
		while (_lock.test_and_set(std::memory_order_acquire));
	}

	bool try_lock()
	{
		return !_lock.test_and_set(std::memory_order_acquire);
	}

	void unlock()
	{
		_lock.clear(std::memory_order_release);
	}

private:
	std::atomic_flag _lock;
};

class Emitter : public QObject{
	Q_OBJECT
public:
signals:
	void addTaskSig(uint64_t, QString);
	void updateTaskSig(uint64_t, TaskInfo);
	void completeTaskSig(uint64_t);
};

class AriaDlg : public FramelessFrame{
	Q_OBJECT
public:
	AriaDlg();
	~AriaDlg();

	QWidget* 	createToolBar();

	QWidget* 	createStatusBar();

	void 		addUri(QString url = nullptr, QString name = nullptr, QString cookie = nullptr);
	void 		addUriTask(std::unique_ptr<UriTask> &);

	Emitter* 	getEmitter(){return _emitter;};
signals:
	void 		changeViewSig(int);
private:
	void 		initAria();
	void 		download();

	void 		mergeTask();
	void 		errorTask(aria2::A2Gid);

	void 		addTask(aria2::A2Gid, const QString &name);
	void 		updateTask(aria2::A2Gid);
	void 		completeTask(aria2::A2Gid);
	TaskInfo 	getTaskInfo(aria2::A2Gid);

	void 		test();
private:
	AriaListWidget *_dnWidget, *_cmWidget, *_trWidget;

	bool 			_threadRunning;
	std::thread 	_thread;

	Emitter			*_emitter;

	aria2::Session 	*_session;

	SpinLock 			_addLock;
	std::vector<std::unique_ptr<Task>> 	_addTasks;
};
