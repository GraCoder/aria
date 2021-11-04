#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <memory>
#include <thread>

#include <QDialog>
#include <QHash>

#include "aria2/aria2.h"

#include "frameless.h"
#include "taskInfo.h"

class QListWidget;
class QListWidgetItem;
class QSystemTrayIcon;
class AriaListWidget;
class TaskDatabase;

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
	void removeTaskSig(uint64_t);
	void startTaskSig(uint64_t);
	void pauseTaskSig(uint64_t);

	void failTaskSig(uint64_t);
};

class AriaDlg : public FramelessFrame{
	Q_OBJECT
public:
	AriaDlg();
	~AriaDlg();

	static AriaDlg* getMainDlg();
	AriaListWidget* getDownloadWgt();
	AriaListWidget* getCompleteWgt();
	AriaListWidget* getTrashWgt();

	QWidget* 	createToolBar();
	QWidget* 	createStatusBar();
	void		createTrayIcon();

	void		showSlt(int);
	void 		quitSlt();

	void 		addUri(QString url = nullptr, QString cookie = nullptr);
	void 		addTask(std::unique_ptr<Task> &);
	void 		addTask(std::vector<std::unique_ptr<Task>>&);

	void		startSelected();
	void 		pauseSelected();
	void 		deleteSelected();

	void		deleteCompleteSelected();
	void 		deleteAllCompleteSelected();
	void		explorerCompleteSelected();

	void 		deleteTrashSelected();
	void 		deleteAllTrashSelected();
	void		explorerTrashSelected();

	Emitter* 		getEmitter(){return _emitter;}
	TaskDatabase* 	getDatabase() {return _database;}

	TaskInfo 	getTaskInfo(aria2::A2Gid);
	TaskInfoEx	getTaskInfo(aria2::Session *session, aria2::A2Gid);
signals:
	void 		changeViewSig(int);
private:
	void 		initAria();
	void 		download();

	void 		mergeTask();
	void 		errorTask(aria2::A2Gid);

	void 		addUriTask(UriTask *tsk);
	void 		addBtTask(aria2::A2Gid, Task *);
	void 		updateTask(aria2::A2Gid);
	void 		completeTask(aria2::A2Gid);

	void 		test();
private:
	AriaListWidget *_dnWidget, *_cmWidget, *_trWidget;

	QSystemTrayIcon *_trayIcon;

	bool 			_threadRunning;
	std::thread 	_thread;

	Emitter			*_emitter;
	TaskDatabase	*_database;

	aria2::Session 	*_session;

	SpinLock 			_addLock;
	std::vector<std::unique_ptr<Task>> 	_addTasks;
};
