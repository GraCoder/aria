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

#include "ariaSys.h"
#include "ariaPanel.h"
#include "uriLink.h"

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
		dlg->errorProcedure(gid);
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
	setMinimumSize(800, 600);
	setContentsMargins(0, 0, 0, 0);
	_mainWidget = new QListWidget;
	_mainWidget->setStyleSheet("QListWidget{border:none;}");

	auto panel = new AriaPanel;

	_layout->setSpacing(0);
	_layout->setContentsMargins(0, 0, 0, 0);
	_layout->addWidget(panel);
	auto mainLayout = new QVBoxLayout;
	mainLayout->addWidget(new AriaSysMenu);
	mainLayout->addWidget(createToolBar());
	mainLayout->addWidget(_mainWidget);
	mainLayout->addWidget(createStatusBar());
	_layout->addLayout(mainLayout);

	int ret = aria2::libraryInit();
	_thread = std::thread(std::bind(&AriaDlg::download, this));
	_emitter = new Emitter;

	connect(_emitter, &Emitter::addTaskSig, this, &AriaDlg::addTaskSlt, Qt::QueuedConnection);
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
	bar->setAttribute(Qt::WA_TranslucentBackground, false);
	bar->setIconSize(QSize(60, 60));
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
	Task tsk; tsk.type = 1;

	tsk.uri.push_back("https://download.visualstudio.microsoft.com/download/pr/78fa839b-2d86-4ece-9d97-5b9fe6fb66fa/10d406c0d247470daa80691d3b3460a6/windowsdesktop-runtime-5.0.10-win-x64.exe");
	//tsk.uri.push_back("http://localhost:8080");
	//tsk.uri.push_back("http://ftp.dlut.edu.cn/centos/2/centos2-scripts-v1.tar");
	_addLock.lock();
	_addTasks.push_back(tsk);
	_addLock.unlock();

	URILinkWgt wgt;
	wgt.exec();
}

void AriaDlg::addTaskSlt(uint64_t aid)
{
	auto item = new QListWidgetItem;
	item->setData(Qt::UserRole, aid);
	_mainWidget->addItem(item);

	_items.insert(aid, item);
}

void AriaDlg::initAria()
{
	using namespace aria2;

	KeyVals options;
	{
		options.push_back(std::make_pair("user-agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) \
						 AppleWebKit/537.36 (KHTML, like Gecko) Chrome/93.0.4577.82 Safari/537.36 Edg/93.0.961.52"));
		options.push_back(std::make_pair("dir", "d:/thunder"));
		options.push_back(std::make_pair("disable-ipv6", "true"));
		options.push_back(std::make_pair("check-certificate", "false"));
		options.push_back(std::make_pair("disk-cache", "64M"));
		options.push_back(std::make_pair("no-file-allocation-limit", "64M"));
		options.push_back(std::make_pair("continue", "true"));
		options.push_back(std::make_pair("remote-time", "true"));

	{
		auto path = QCoreApplication::applicationDirPath();
		path += "/aria.session";
		QFile file(path);
		if(!file.exists())
		{
			file.open(QIODevice::WriteOnly);
			file.close();
		}
		options.push_back(std::make_pair("input-file", path.toStdString()));
		options.push_back(std::make_pair("save-session", path.toStdString()));
	}

		options.push_back(std::make_pair("save-session-interval", "1"));
		options.push_back(std::make_pair("auto-save-interval", "20"));

		options.push_back(std::make_pair("max-file-not-found", "10"));
		options.push_back(std::make_pair("max-tries", "30"));
		options.push_back(std::make_pair("retry-wait", "10"));
		options.push_back(std::make_pair("connect-timeout", "10"));
		options.push_back(std::make_pair("timeout", "10"));
		options.push_back(std::make_pair("max-concurrent-downloads", "5"));
		options.push_back(std::make_pair("split", "64"));
		options.push_back(std::make_pair("min-split-size", "4M"));
		options.push_back(std::make_pair("piece-length", "1M"));
		options.push_back(std::make_pair("allow-piece-length-change", "true"));
		options.push_back(std::make_pair("", ""));
		options.push_back(std::make_pair("", ""));
		options.push_back(std::make_pair("", ""));
		options.push_back(std::make_pair("", ""));
		options.push_back(std::make_pair("", ""));
	}

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
			addTask();

		int ret = run(_session, RUN_ONCE);
		if(ret)
		{
			auto tks = getActiveDownload(_session);
			for(int i = 0; i < tks.size(); i++) {
				auto dh = getDownloadHandle(_session, tks[i]);
				printf("");
				auto speed = dh->getDownloadSpeed();
				dh->getBitfield();
				auto numPices = dh->getNumPieces();
				auto picLen = dh->getPieceLength();
				deleteDownloadHandle(dh);
			}
		}else
			std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void AriaDlg::addTask()
{
	using namespace aria2;

	if(_addLock.try_lock()) {
		for(int i = 0; i < _addTasks.size(); i++){
			auto &tsk = _addTasks[i];
			switch(tsk.type){
			case 1:
			{
				A2Gid grid; KeyVals tmpOpt;// = getGlobalOptions(_session);
				auto ret = aria2::addUri(_session, &grid, tsk.uri, tmpOpt);
				_emitter->addTaskSig(grid);
				break;
			}
			default:
				break;
			}
		}
		_addTasks.clear();
		_addLock.unlock();
	}
}

void AriaDlg::errorProcedure(aria2::A2Gid id)
{
	auto dh = aria2::getDownloadHandle(_session, id);
	auto code = dh->getErrorCode();
	aria2::deleteDownloadHandle(dh);
}

void AriaDlg::test()
{
	auto tks = getActiveDownload(_session);
	printf("");
}
