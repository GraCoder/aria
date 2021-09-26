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
#include <QFile>
#include <QStackedWidget>

#include "ariaSys.h"
#include "ariaPanel.h"
#include "uriLink.h"
#include "ariaListWgt.h"

int downloadEventCallback(aria2::Session* session, aria2::DownloadEvent event,
			  aria2::A2Gid gid, void* userData)
{
	switch(event) {
	case aria2::EVENT_ON_DOWNLOAD_START:
		printf("");
		break;
	case aria2::EVENT_ON_DOWNLOAD_COMPLETE:
		std::cerr << "COMPLETE";
		break;
	case aria2::EVENT_ON_DOWNLOAD_ERROR:
	{
		auto dlg = (AriaDlg*)userData;
		dlg->errorTask(gid);
		std::cerr << "ERROR" << " [" << aria2::gidToHex(gid) << "] ";
		break;
	}
	default:
		return 0;
	}
	return 1;
}


AriaDlg::AriaDlg()
	:_threadRunning(true)
{
	setMinimumSize(1000, 600);
	setContentsMargins(0, 0, 0, 0);
	{
		setStyleSheet("QWidget{font-family:\"Microsoft YaHei UI Light\"; font-size:16px; font-weight:100;}");
	}

	_dnWidget = new AriaListWidget(DOWNLOADING);
	_cmWidget = new AriaListWidget(DOWNLOADING);
	_trWidget = new AriaListWidget(DOWNLOADING);

	auto stackWgt = new QStackedWidget;
	connect(this, &AriaDlg::changeViewSig, stackWgt, &QStackedWidget::setCurrentIndex);
	stackWgt->setStyleSheet("QListWidget{border-top:1px solid gray;}");
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
	mainLayout->addWidget(stackWgt);
	mainLayout->addWidget(createStatusBar());
	_layout->addLayout(mainLayout);

	int ret = aria2::libraryInit();
	_thread = std::thread(std::bind(&AriaDlg::download, this));
	_emitter = new Emitter;

	qRegisterMetaType<uint64_t>("uint64_t");
	qRegisterMetaType<TaskInfo>("TaskInfo");
	connect(_emitter, &Emitter::addTaskSig, _dnWidget, &AriaListWidget::addTaskSlt, Qt::QueuedConnection);
	connect(_emitter, &Emitter::updateTaskSig, _dnWidget, &AriaListWidget::updateTaskSlt, Qt::QueuedConnection);
	connect(_emitter, &Emitter::completeTaskSig, _dnWidget, &AriaListWidget::completeTaskSlt, Qt::QueuedConnection);
}

AriaDlg::~AriaDlg()
{
	_threadRunning = false;
	_thread.join();

	aria2::sessionFinal(_session);
	aria2::libraryDeinit();
}

QWidget *AriaDlg::createToolBar()
{
	auto bar = new QToolBar;
	bar->setContentsMargins(30, 0, 0, 0);
	bar->setAttribute(Qt::WA_TranslucentBackground, false);
	bar->setIconSize(QSize(40, 40));
	{
		bar->addAction(QIcon(":/aria/icons/insert-link.svg"), tr("addUri"), this, &AriaDlg::addUri);
	}
	{
	}
	bar->addAction(tr("test"), this, &AriaDlg::test);
	return bar;
}

QWidget *AriaDlg::createStatusBar()
{
	auto bar = new QStatusBar;
	return bar;
}

void AriaDlg::addUri()
{	
#ifdef NDEBUG
	URILinkWgt wgt;
	if(wgt.exec() != QDialog::Accepted)
		return;
	Task tsk; tsk.type = 1;
	tsk.uri = wgt.getUris();
#else
	Task tsk; tsk.type = 1;
	tsk.uri.push_back("http://ftp.dlut.edu.cn/centos/2/centos2-scripts-v1.tar");
#endif

	_addLock.lock();
	_addTasks.push_back(tsk);
	_addLock.unlock();
}

void AriaDlg::initAria()
{
	using namespace aria2;

	std::map<std::string, std::string> opTmps;
	{
		opTmps["user-agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) \
				AppleWebKit/537.36 (KHTML, like Gecko) Chrome/93.0.4577.82 Safari/537.36 Edg/93.0.961.52";
				opTmps["dir"] = "d:/thunder";
		opTmps["disable-ipv6"] = "true";
		opTmps["check-certificate"] = "false";
		opTmps["disk-cache"] = "64M";
		opTmps["no-file-allocation-limit"] = "64M";
		opTmps["continue"] = "true";
		opTmps["remote-time"] = "true";

		{
			auto path = QCoreApplication::applicationDirPath();
			path += "/aria.session";
			QFile file(path);
			if(!file.exists())
			{
				file.open(QIODevice::WriteOnly);
				file.close();
			}
			opTmps["input-file"]= path.toStdString();
			opTmps["save-session"]= path.toStdString();
		}

		opTmps["save-session-interval"]= "1";
		opTmps["auto-save-interval"]= "20";

		opTmps["max-file-not-found"]= "10";
		opTmps["max-tries"]= "30";
		opTmps["retry-wait"]= "10";
		opTmps["connect-timeout"]= "10";
		opTmps["timeout"]= "10";
		opTmps["max-concurrent-downloads"]= "5";
		opTmps["split"]= "64";
		opTmps["min-split-size"]= "4M";
		opTmps["piece-length"]= "1M";
		opTmps["allow-piece-length-change"]= "true";
		opTmps["max-overall-download-limit"] = "0";
		opTmps["max-download-limit"] = "30k";
//		opTmps[""]= "";
//		opTmps[""]= "";
//		opTmps[""]= "";
//		opTmps[""]= "";
//		opTmps[""]= "";
	}

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

	initAria();

	while(_threadRunning)
	{
		if(!_addTasks.empty())
			mergeTask();

		int ret = run(_session, RUN_ONCE);

		if(ret)
		{
			auto tks = getActiveDownload(_session);
			for(int i = 0; i < tks.size(); i++) {
				updateTask(tks[i]);
			}
		}else
			std::this_thread::sleep_for(std::chrono::seconds(1));
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
			auto &tsk = _addTasks[i];
			switch(tsk.type){
			case 1:
			{
				KeyVals tmpOpt;// = getGlobalOptions(_session);
				ret = aria2::addUri(_session, &gid, tsk.uri, tmpOpt);
				name = QUrl(QString::fromStdString(tsk.uri.front())).fileName();
				break;
			}
			default:
				break;
			}
			if(ret == 0)
				addTask(gid, name);
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

	tskInfo.picNums = dh->getNumPieces();
	tskInfo.picLength = dh->getPieceLength();
	deleteDownloadHandle(dh);
	return tskInfo;
}

void AriaDlg::test()
{
	auto tks = getActiveDownload(_session);
	printf("");
}
