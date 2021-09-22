#include "ariaUi.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QListWidget>

#include <aria2/aria2.h>

AriaDlg::AriaDlg()
{
	setMinimumSize(800, 600);
	_mainWidget = new QListWidget;

	auto mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(createToolBar());
	mainLayout->addWidget(_mainWidget);
	mainLayout->addWidget(createStatusBar());


	initAria();
}

AriaDlg::~AriaDlg()
{
	_thread.join();
}

QWidget *AriaDlg::createToolBar()
{
	auto bar = new QToolBar;
	bar->addAction(tr("addTask"));
	return bar;
}

QWidget *AriaDlg::createStatusBar()
{
	auto bar = new QStatusBar;
	return bar;
}

void AriaDlg::initAria()
{
	using namespace aria2;

	KeyVals options;
	SessionConfig config;
	config.keepRunning = false;
	_session = sessionNew(options, config);

	A2Gid grid;
	std::vector<std::string> tmp;
	tmp.push_back("https://www.7-zip.org/a/7z1900-x64.exe");
	KeyVals tmpOpt;
	addUri(_session, &grid, tmp, tmpOpt);

	_thread = std::thread(std::bind(&AriaDlg::download, this));
}

void AriaDlg::download()
{
	using namespace aria2;

	int ret = 1;
	while(ret)
	{
		ret = run(_session, RUN_ONCE);
		if(ret)
		{
			auto tks = getActiveDownload(_session);
			for(int i = 0; i < tks.size(); i++) {
				auto dh = getDownloadHandle(_session, tks[i]);
				printf("");
				deleteDownloadHandle(dh);
			}
		}
	}
}
