#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <memory>
#include <thread>

#include <QDialog>
#include <QHash>

#include <aria2/aria2.h>

#include "frameless.h"

class QListWidget;
class QListWidgetItem;

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

struct Task{
	int type; //1-url
	std::vector<std::string> uri;
};

class Emitter : public QObject{
	Q_OBJECT
public:
signals:
	void addTaskSig(uint64_t);
};

class AriaDlg : public FramelessFrame{
	friend int downloadEventCallback(aria2::Session* session,
		aria2::DownloadEvent event, aria2::A2Gid gid, void* userData);
	Q_OBJECT
public:
	AriaDlg();
	~AriaDlg();

	QWidget* createToolBar();

	QWidget* createStatusBar();

	void 	addUri();

	void 	addTaskSlt(uint64_t);

signals:
	void 	changeViewSig(int);
private:
	void 	initAria();
	void 	download();

	void 	addTask();
	void 	errorProcedure(aria2::A2Gid);

	void 	test();
private:
	QListWidget *_dnWidget, *_cmWidget, *_trWidget;

	QHash<uint64_t, QListWidgetItem*>		 _items;

	bool 			_threadRunning;
	std::thread 	_thread;

	Emitter			*_emitter;

	aria2::Session 	*_session;

	SpinLock 			_addLock;
	std::vector<Task> 	_addTasks;
};
