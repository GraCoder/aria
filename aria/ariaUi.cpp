#include "ariaUi.h"
#include <iostream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QListWidget>
#include <QListWidgetItem>
#include <QCoreApplication>
#include <QToolButton>
#include <QMenu>
#include <QFile>
#include <QStackedWidget>
#include <QSystemTrayIcon>

#include "ariaSys.h"
#include "ariaPanel.h"
#include "uriLink.h"
#include "ariaListWgt.h"
#include "taskDatabase.h"
#include "ariaSetting.h"

int downloadEventCallback(aria2::Session* session, aria2::DownloadEvent event,
			  aria2::A2Gid gid, void* userData)
{
	switch(event) {
	case aria2::EVENT_ON_DOWNLOAD_START:
		std::cout << "START" << " [" << aria2::gidToHex(gid) << "] " << std::endl;
	{
		auto dlg = (AriaDlg*)userData;
		auto tskInfo = dlg->getTaskInfo(gid);
		dlg->getDatabase()->updateTaskInfo(gid, tskInfo);
		dlg->getEmitter()->startTaskSig(gid);
		break;
	}
	case aria2::EVENT_ON_DOWNLOAD_PAUSE:
	{
		auto dlg = (AriaDlg*)userData;
		auto tskInfo = dlg->getTaskInfo(gid);
		dlg->getDatabase()->updateTaskInfo(gid, tskInfo);
		dlg->getEmitter()->pauseTaskSig(gid);
		std::cout << "PAUSE" << " [" << aria2::gidToHex(gid) << "] " << std::endl;
		break;
	}
	case aria2::EVENT_ON_DOWNLOAD_STOP:
	{
		auto dlg = (AriaDlg*)userData;
		auto tskInfo = dlg->getTaskInfo(gid);
		dlg->getDatabase()->updateTaskInfo(gid, tskInfo);
		//dlg->getEmitter()->removeTaskSig(gid);
		std::cout << "STOP" << " [" << aria2::gidToHex(gid) << "] " << std::endl;
		break;
	}
	case aria2::EVENT_ON_BT_DOWNLOAD_COMPLETE:
	case aria2::EVENT_ON_DOWNLOAD_COMPLETE:
	{
		auto dlg = (AriaDlg*)userData;
		auto tskInfo = dlg->getTaskInfo(gid);
		dlg->getDatabase()->updateTaskInfo(gid, tskInfo);
		dlg->getEmitter()->completeTaskSig(gid);
		std::cout << "COMPLETE" << " [" << aria2::gidToHex(gid) << "] " << std::endl;
		break;
	}
	case aria2::EVENT_ON_DOWNLOAD_ERROR:
	{
		auto dlg = (AriaDlg*)userData;
		//dlg->errorTask(gid);
		dlg->getEmitter()->failTaskSig(gid);
		dlg->getDatabase()->failTask(gid);
		std::cerr << "ERROR" << " [" << aria2::gidToHex(gid) << "] ";
		break;
	}
	default:
		return 0;
	}
	return 1;
}

AriaDlg *mainDlg = nullptr;

AriaDlg::AriaDlg()
	:_threadRunning(true)
{
	mainDlg = this;

	setMinimumSize(1000, 600);
	setContentsMargins(0, 0, 0, 0);
	{
		setStyleSheet("QWidget{font-family:\"Microsoft YaHei UI Light\"; font-size:16px;}");
	}

	_dnWidget = new AriaListWidget(DOWNLOADING);
	_cmWidget = new AriaListWidget(COMPLETED);
	_trWidget = new AriaListWidget(TRASHCAN);

	auto stackWgt = new QStackedWidget;
	connect(this, &AriaDlg::changeViewSig, stackWgt, &QStackedWidget::setCurrentIndex);
	stackWgt->setStyleSheet("QStackedWidget{border-top:1px solid gray;}");
	stackWgt->addWidget(_dnWidget);
	stackWgt->addWidget(_cmWidget);
	stackWgt->addWidget(_trWidget);

	auto panel = new AriaPanel(this);

	_layout->setSpacing(0);
	_layout->setContentsMargins(0, 0, 0, 0);
	_layout->addWidget(panel);
	auto mainLayout = new QVBoxLayout;
	mainLayout->addWidget(new AriaSysMenu);
	mainLayout->addWidget(createToolBar());
	mainLayout->addWidget(stackWgt, 10);
	mainLayout->addWidget(createStatusBar());
	_layout->addLayout(mainLayout);
	createTrayIcon();

	initAria();
	_emitter = new Emitter;

	qRegisterMetaType<uint64_t>("uint64_t");
	qRegisterMetaType<TaskInfo>("TaskInfo");
	connect(_emitter, &Emitter::addTaskSig, _dnWidget, &AriaListWidget::addTaskSlt, Qt::BlockingQueuedConnection);
	connect(_emitter, &Emitter::updateTaskSig, _dnWidget, &AriaListWidget::updateTaskSlt, Qt::QueuedConnection);
	connect(_emitter, &Emitter::startTaskSig, _dnWidget, &AriaListWidget::startTaskSlt, Qt::QueuedConnection);
	connect(_emitter, &Emitter::pauseTaskSig, _dnWidget, &AriaListWidget::pauseTaskSlt, Qt::QueuedConnection);
	connect(_emitter, &Emitter::failTaskSig, _dnWidget, &AriaListWidget::failTaskSlt, Qt::QueuedConnection);
	connect(_emitter, &Emitter::completeTaskSig, _dnWidget, &AriaListWidget::removeTaskSlt, Qt::QueuedConnection);
	connect(_emitter, &Emitter::removeTaskSig, _dnWidget, &AriaListWidget::removeTaskSlt, Qt::QueuedConnection);


	_database = new TaskDatabase;
	_database->initDownloadTask();

	connect(_emitter, &Emitter::completeTaskSig, _database, &TaskDatabase::completeTask, Qt::QueuedConnection);
	//connect(_emitter, &Emitter::removeTaskSig, _database, &TaskDatabase::deleteTask, Qt::QueuedConnection);

	_thread = std::thread(std::bind(&AriaDlg::download, this));
}

AriaDlg::~AriaDlg()
{
	_threadRunning = false;
	_thread.join();

	if(_database)
		delete _database;

	aria2::sessionFinal(_session);
	aria2::libraryDeinit();
}

AriaDlg *AriaDlg::getMainDlg()
{
	return mainDlg;
}

AriaListWidget *AriaDlg::getDownloadWgt()
{
	return _dnWidget;
}

AriaListWidget *AriaDlg::getCompleteWgt()
{
	return _cmWidget;
}

AriaListWidget *AriaDlg::getTrashWgt()
{
	return _trWidget;
}

QWidget *AriaDlg::createToolBar()
{
	auto stackWgt = new QStackedWidget;
	connect(this, &AriaDlg::changeViewSig, stackWgt, &QStackedWidget::setCurrentIndex);
	{
		auto bar = new QToolBar;
		bar->setContentsMargins(30, 0, 0, 0);
		bar->setAttribute(Qt::WA_TranslucentBackground, false);
		bar->setIconSize(QSize(32, 32));
		bar->setStyleSheet("QToolBar{spacing:10px; padding-left:20px;}");
		{
			bar->addAction(QIcon(":/aria/icons/insert-link.svg"), tr("addUri"), std::bind(&AriaDlg::addUri, this, QString(), QString()));
		}
		{
			bar->addAction(QIcon(":/aria/icons/xx/download.svg"), tr("start"), std::bind(&AriaDlg::startSelected, this));
			bar->addAction(QIcon(":/aria/icons/xx/pause.svg"), tr("pause"), std::bind(&AriaDlg::pauseSelected, this));
			bar->addAction(QIcon(":/aria/icons/xx/delete.svg"), tr("delete"), std::bind(&AriaDlg::deleteSelected, this));
		}
		bar->addAction(tr("test"), this, &AriaDlg::test);

		stackWgt->addWidget(bar);
	}
	{
		auto bar = new QToolBar;
		stackWgt->addWidget(bar);
	}
	{
		auto bar = new QToolBar;
		stackWgt->addWidget(bar);
	}
	return stackWgt;
}

QWidget *AriaDlg::createStatusBar()
{
	auto bar = new QStatusBar;
	return bar;
}

void AriaDlg::createTrayIcon()
{
	_trayIcon = new QSystemTrayIcon(QIcon(":/aria/icons/qbittorrent.ico"));
	_trayIcon->show();

	auto menu = _trayIcon->contextMenu();
	menu->addAction(tr("quit"), this, &AriaDlg::quitSlt);

	connect(_trayIcon, &QSystemTrayIcon::activated, this, &AriaDlg::showSlt);
}

void AriaDlg::showSlt(int ret)
{
	if(ret == QSystemTrayIcon::DoubleClick)
		show();
}

void AriaDlg::quitSlt()
{
	QCoreApplication::quit();
}

void AriaDlg::addUri(const QString url, const QString cookie)
{
	URILinkWgt wgt(url);
	if(wgt.exec() != QDialog::Accepted)
		return;

	auto tsks = wgt.getTasks();
	//auto tsk = std::make_unique<UriTask>();
	//tsk->type = 1;
	//tsk->url.push_back("http://ftp.dlut.edu.cn/centos/2/centos2-scripts-v1.tar");
	for(auto &tsk : tsks)
		addUriTask(tsk);
}

void AriaDlg::addUriTask(std::unique_ptr<UriTask> &tsk)
{
	auto rid = _database->findTask(tsk.get());
	if(rid == 0)
		tsk->rid = _database->addTask(tsk.get());
	else{
		return;
	}
	_addLock.lock();
	_addTasks.push_back(std::move(tsk));
	_addLock.unlock();
}

void AriaDlg::startSelected()
{
	auto ids = _dnWidget->getSelected();
	for(auto id : ids){
		unpauseDownload(_session, id);
	}
}

void AriaDlg::pauseSelected()
{
	auto ids = _dnWidget->getSelected();
	for(auto id : ids){
		pauseDownload(_session, id);
	}
}

void AriaDlg::deleteSelected()
{
	auto ids = _dnWidget->getSelected();
	for(auto id : ids){
		removeDownload(_session, id);
		_dnWidget->removeTaskSlt(id);
		_database->deleteTask(id);
	}
}

TaskInfo AriaDlg::getTaskInfo(aria2::A2Gid id)
{
	TaskInfo tskInfo;
	auto dh = getDownloadHandle(_session, id);
	tskInfo.fileData = dh->getFiles();

	tskInfo.dnspeed = dh->getDownloadSpeed();
	tskInfo.upspeed = dh->getUploadSpeed();
	tskInfo.dnloadLength = dh->getCompletedLength();
	tskInfo.totalLength = dh->getTotalLength();
	tskInfo.uploadLength = dh->getUploadLength();

	tskInfo.state = dh->getStatus();
	tskInfo.picNums = dh->getNumPieces();
	tskInfo.picLength = dh->getPieceLength();
	deleteDownloadHandle(dh);

	return tskInfo;
}

void AriaDlg::initAria()
{
	aria2::libraryInit();

	using namespace aria2;	

	auto &opTmps = ariaSetting::instance().setting();
	KeyVals options;
	for(auto &iter : opTmps)
		options.push_back(std::make_pair(iter.first, iter.second));

	SessionConfig config;
	config.keepRunning = true;
	config.downloadEventCallback = downloadEventCallback;
	config.userData = this;
	_session = sessionNew(options, config);
	auto opts = getGlobalOptions(_session);
}

void AriaDlg::download()
{
	using namespace aria2;

	auto prev = std::chrono::system_clock().now();

	while(_threadRunning)
	{
		if(!_addTasks.empty())
			mergeTask();

		int ret = run(_session, RUN_ONCE);

		//if(ret)
		{
			auto curr = std::chrono::system_clock().now();
			auto sec = std::chrono::duration_cast<std::chrono::milliseconds>(curr - prev);
			if(sec.count() < 2000)
				std::this_thread::sleep_for(std::chrono::milliseconds(2000) - sec);
			prev = std::chrono::system_clock().now();
			auto tks = getActiveDownload(_session);
			for(int i = 0; i < tks.size(); i++) {
				updateTask(tks[i]);
			}
		}
	}
}

void AriaDlg::mergeTask()
{
	using namespace aria2;

	if(_addLock.try_lock()) {
		for(int i = 0; i < _addTasks.size(); i++){
			int ret = 0;
			QString name;
			A2Gid gid;
			auto tsk = std::move(_addTasks[i]);
			switch(tsk->type){
			case 1:
			{
				auto ptask = static_cast<UriTask*>(tsk.get());
				KeyVals tmpOpt;// = getGlobalOptions(_session);
				std::vector<std::string> url(1, ptask->url);
				ret = aria2::addUri(_session, &gid, url, tmpOpt);
				name = QString::fromStdString(ptask->name);
				getTaskInfo(gid);
				_database->downloadTask(ptask->rid, gid);
				break;
			}
			default:
				break;
			}
			if(ret == 0)
			{
				addTask(gid, name);
			}
			else
			{
				//errTask();
			}
		}
		_addTasks.clear();
		_addLock.unlock();
	}
}

void AriaDlg::errorTask(aria2::A2Gid id)
{
	auto dh = aria2::getDownloadHandle(_session, id);
	auto code = dh->getErrorCode();
	aria2::deleteDownloadHandle(dh);
}

void AriaDlg::addTask(aria2::A2Gid id, const QString &name)
{
	_emitter->addTaskSig(id, name);
}

void AriaDlg::updateTask(aria2::A2Gid id)
{
	auto tksInfo = getTaskInfo(id);
	_emitter->updateTaskSig(id, std::move(tksInfo));
}

void AriaDlg::completeTask(aria2::A2Gid id)
{
	_emitter->completeTaskSig(id);
}

void AriaDlg::test()
{
	auto tks = getActiveDownload(_session);
	printf("");
}
