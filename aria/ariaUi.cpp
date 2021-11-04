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
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QMessageBox>

#include <algorithm>

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
		//auto tskInfo = dlg->getTaskInfo(session, gid);
		//dlg->getDatabase()->updateTaskInfo(gid, tskInfo);
		dlg->getEmitter()->startTaskSig(gid);
		break;
	}
	case aria2::EVENT_ON_DOWNLOAD_PAUSE:
	{
		auto dlg = (AriaDlg*)userData;
		auto tskInfo = dlg->getTaskInfo(session, gid);
		dlg->getDatabase()->updateTaskInfo(gid, tskInfo);
		dlg->getEmitter()->pauseTaskSig(gid);
		std::cout << "PAUSE" << " [" << aria2::gidToHex(gid) << "] " << std::endl;
		break;
	}
	case aria2::EVENT_ON_DOWNLOAD_STOP:
	{
		auto dlg = (AriaDlg*)userData;
		auto tskInfo = dlg->getTaskInfo(session, gid);
		dlg->getDatabase()->updateTaskInfo(gid, tskInfo);
		//dlg->getEmitter()->removeTaskSig(gid);
		std::cout << "STOP" << " [" << aria2::gidToHex(gid) << "] " << std::endl;
		break;
	}
	case aria2::EVENT_ON_BT_DOWNLOAD_COMPLETE:
	case aria2::EVENT_ON_DOWNLOAD_COMPLETE:
	{
		auto dlg = (AriaDlg*)userData;
		auto tskInfo = dlg->getTaskInfo(session, gid);
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
	case aria2::EVENT_ON_SAVE_SESSION:
	{
		auto dlg = (AriaDlg*)userData;
		auto tskInfo = dlg->getTaskInfo(session, gid);
		dlg->getDatabase()->updateTaskInfo(gid, tskInfo);
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
	setWindowIcon(QIcon(":/aria/icons/qbittorrent.ico"));
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
	_database->initCompleteTask();
	_database->initTrashTask();

	connect(_emitter, &Emitter::completeTaskSig, _database, &TaskDatabase::completeTask, Qt::QueuedConnection);
	//connect(_emitter, &Emitter::removeTaskSig, _database, &TaskDatabase::deleteTask, Qt::QueuedConnection);

	_thread = std::thread(std::bind(&AriaDlg::download, this));
}

AriaDlg::~AriaDlg()
{
	_threadRunning = false;
	_thread.join();

	aria2::sessionFinal(_session);
	aria2::libraryDeinit();

	if(_database)
		delete _database;
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
		bar->setAttribute(Qt::WA_TranslucentBackground, false);
		bar->setIconSize(QSize(32, 32));
		bar->setStyleSheet("QToolBar{spacing:10px; padding-left:20px;}");
		{
			bar->addAction(QIcon(":/aria/icons/insert-link.svg"), tr("addUri"), std::bind(&AriaDlg::addUri, this, QString(), QString()));
		}
		{
			bar->addAction(QIcon(":/aria/icons/download.svg"), tr("start"), std::bind(&AriaDlg::startSelected, this));
			bar->addAction(QIcon(":/aria/icons/pause.svg"), tr("pause"), std::bind(&AriaDlg::pauseSelected, this));
			bar->addAction(QIcon(":/aria/icons/delete.svg"), tr("delete"), std::bind(&AriaDlg::deleteSelected, this));
		}
		bar->addAction(tr("test"), this, &AriaDlg::test);

		stackWgt->addWidget(bar);
	}
	{
		auto bar = new QToolBar;
		bar->setAttribute(Qt::WA_TranslucentBackground, false);
		bar->setIconSize(QSize(32, 32));
		bar->setStyleSheet("QToolBar{spacing:20px; padding-left:20px;}");
		bar->addAction(QIcon(":/aria/icons/delete.svg"), tr("delete"), std::bind(&AriaDlg::deleteCompleteSelected, this));
		bar->addAction(QIcon(":/aria/icons/delete-all.svg"), tr("deleteAll"), std::bind(&AriaDlg::deleteAllCompleteSelected, this));
		bar->addAction(QIcon(":/aria/icons/folder.svg"), tr("explorer"), std::bind(&AriaDlg::explorerCompleteSelected, this));
		stackWgt->addWidget(bar);
	}
	{
		auto bar = new QToolBar;
		bar->setAttribute(Qt::WA_TranslucentBackground, false);
		bar->setIconSize(QSize(32, 32));
		bar->setStyleSheet("QToolBar{spacing:20px; padding-left:20px;}");
		bar->addAction(QIcon(":/aria/icons/delete.svg"), tr("delete"), std::bind(&AriaDlg::deleteTrashSelected, this));
		bar->addAction(QIcon(":/aria/icons/delete-all.svg"), tr("deleteAll"), std::bind(&AriaDlg::deleteAllTrashSelected, this));
		bar->addAction(QIcon(":/aria/icons/folder.svg"), tr("explorer"), std::bind(&AriaDlg::explorerTrashSelected, this));
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

	auto menu = new QMenu;
	_trayIcon->setContextMenu(menu);
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
	using namespace aria2;

	URILinkWgt wgt(url);
	if(wgt.exec() != QDialog::Accepted)
		return;

	auto tsks = wgt.getTasks();
	for(auto &tsk : tsks)
	{
		auto localTask = _database->findTask(tsk->getLocal());

		if(localTask == nullptr) { addTask(tsk); continue; }

		if(localTask->state == DOWNLOAD_ACTIVE || localTask->state == DOWNLOAD_PAUSED ||
				localTask->state == DOWNLOAD_WAITING)
		{
			QMessageBox::warning(this, "", tr("already have a same task is running."));
			continue;
		}

		if(QMessageBox::question(this, "", tr("already have a same task, override?")) == QMessageBox::No)
			continue;

		auto filePath = QString::fromStdString(tsk->getLocal());
		QFileInfo fileInfo(filePath);
		if(fileInfo.isFile())
		{
			QFile(filePath).remove();
			QFile(filePath + ".aria2").remove();
		}
		else
			QDir(filePath).removeRecursively();

		_database->deleteTask(localTask->id);
		if(localTask->state == DOWNLOAD_ERROR)
			_dnWidget->removeTaskSlt(localTask->id);
		else if(localTask->state == DOWNLOAD_COMPLETE)
			_cmWidget->removeTaskSlt(localTask->id);
		else if(localTask->state == DOWNLOAD_REMOVED)
			_trWidget->removeTaskSlt(localTask->id);

		addTask(tsk);
	}
}

void AriaDlg::addTask(std::unique_ptr<Task> &tsk)
{
	_addLock.lock();
	_addTasks.push_back(std::move(tsk));
	_addLock.unlock();
}

void AriaDlg::addTask(std::vector<std::unique_ptr<Task> > &tsks)
{
	_addLock.lock();
	for(auto &tsk : tsks){
		_addTasks.push_back(std::move(tsk));
	}
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
		_database->trashTask(id);
	}
}

void AriaDlg::deleteCompleteSelected()
{
	auto ids = _cmWidget->getSelected();
	for(auto id : ids) {
		_cmWidget->removeTaskSlt(id);
		_database->deleteFinishTask(id);
	}
}

void AriaDlg::deleteAllCompleteSelected()
{
	auto ids = _cmWidget->getSelected();
	for(auto id : ids) {
		_cmWidget->removeTaskSlt(id);
		_database->deleteFinishTask(id);
	}
}

void AriaDlg::explorerCompleteSelected()
{
	_cmWidget->explorerSelected();
}

void AriaDlg::deleteTrashSelected()
{
	auto ids = _trWidget->getSelected();
	for(auto id : ids){
		_trWidget->removeTaskSlt(id);
		_database->deleteFinishTask(id);
	}
}

void AriaDlg::deleteAllTrashSelected()
{
	auto ids = _trWidget->getSelected();
	for(auto id : ids){
		_trWidget->removeTaskSlt(id);
		_database->deleteFinishTask(id, true);
	}
}

void AriaDlg::explorerTrashSelected()
{
	_trWidget->explorerSelected();
}

TaskInfo AriaDlg::getTaskInfo(aria2::A2Gid id)
{
	TaskInfo tskInfo;
	auto dh = getDownloadHandle(_session, id);
	if(dh)
	{
		tskInfo.dnspeed = dh->getDownloadSpeed();
		tskInfo.upspeed = dh->getUploadSpeed();
		tskInfo.dnloadLength = dh->getCompletedLength();
		tskInfo.totalLength = dh->getTotalLength();
		tskInfo.uploadLength = dh->getUploadLength();

		tskInfo.state = dh->getStatus();
		tskInfo.picNums = dh->getNumPieces();
		tskInfo.picLength = dh->getPieceLength();
		deleteDownloadHandle(dh);
	}
	return tskInfo;
}

TaskInfoEx AriaDlg::getTaskInfo(aria2::Session *session, aria2::A2Gid id)
{
	TaskInfoEx tskInfo;
	auto dh = getDownloadHandle(session, id);
	if(dh){
		tskInfo.fileData = dh->getFiles();

		tskInfo.dnspeed = dh->getDownloadSpeed();
		tskInfo.upspeed = dh->getUploadSpeed();
		tskInfo.dnloadLength = dh->getCompletedLength();
		tskInfo.totalLength = dh->getTotalLength();
		tskInfo.uploadLength = dh->getUploadLength();

		tskInfo.state = dh->getStatus();
		tskInfo.picNums = dh->getNumPieces();
		tskInfo.picLength = dh->getPieceLength();
		tskInfo.fileData = dh->getFiles();
		tskInfo.opts = dh->getOptions();
		deleteDownloadHandle(dh);
	}

	return tskInfo;
}

void AriaDlg::initAria()
{
	using namespace aria2;

	aria2::libraryInit();

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

		if(ret)
		{
			auto curr = std::chrono::system_clock().now();
			auto sec = std::chrono::duration_cast<std::chrono::milliseconds>(curr - prev);
			if(sec.count() < 2000)
				continue;
			prev = curr;
			auto tks = getActiveDownload(_session);
			for(int i = 0; i < tks.size(); i++) {
				updateTask(tks[i]);
			}
		}
	}
}

void AriaDlg::mergeTask()
{
	if(_addLock.try_lock()) {
		for(int i = 0; i < _addTasks.size(); i++){
			auto &tsk = _addTasks[i];
			if(tsk->type == 1){
				auto ptsk = static_cast<UriTask*>(tsk.get());
				addUriTask(ptsk);
			}
			else if(tsk->type == 2) {
				//auto ptask = static_cast<BtTask*>(tsk.get());
				//ret = aria2::addTorrent(_session, &gid, ptask->torrent, tmpOpts);
				//name = QString::fromStdString(ptask->name);
				//break;
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

void AriaDlg::addUriTask(UriTask *tsk)
{
	using namespace aria2;

	A2Gid gid = tsk->id;
	QString name = QString::fromStdString(tsk->name);
	if(tsk->state == DOWNLOAD_ERROR)
	{
		_emitter->addTaskSig(gid, name);
		auto tskInfo = getTaskInfo(gid);
		_emitter->updateTaskSig(gid, tskInfo);
		return;
	}

	KeyVals tmpOpts = tsk->opts;
	std::vector<std::string> url(1, tsk->url);
	int ret = aria2::addUri(_session, &gid, url, tmpOpts);
	if(ret != 0)
		return;

	if(tsk->id != gid) {
		tsk->id = gid;
		_database->addTask(tsk);
	}
	_emitter->addTaskSig(gid, name);
	auto taskInfo = getTaskInfo(gid);
	taskInfo.state = tsk->state;
	taskInfo.totalLength = tsk->to_size;
	taskInfo.dnloadLength = tsk->dn_size;
	taskInfo.uploadLength = tsk->up_size;
	_emitter->updateTaskSig(gid, taskInfo);

}

void AriaDlg::addBtTask(aria2::A2Gid, Task *)
{

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
