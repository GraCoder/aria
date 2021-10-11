#include "uriLink.h"

#include <QPlainTextEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QListWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <regex>

#include "ariaSetting.h"

URILinkWgt::URILinkWgt(const QString &url)
{
	createWidgets();
	_edit->setPlainText(url);

	connect(_edit, &QPlainTextEdit::textChanged, this, &URILinkWgt::uriChangedSlt);
}

std::vector<std::unique_ptr<UriTask> >
URILinkWgt::getTasks()
{
	std::vector<std::unique_ptr<UriTask> > ret;

	auto text = _edit->toPlainText();
	auto uris = text.split('\n');
	auto tsk = std::make_unique<UriTask>();
	for(auto &uri : uris)
	{
		tsk->url = uri.toStdString();
		tsk->name = QUrl(uri).fileName().toStdString();
	}

	tsk->type = 1;
	ret.push_back(std::move(tsk));
	return ret;
}

void URILinkWgt::downloadSlt()
{
	accept();
}

void URILinkWgt::uriChangedSlt()
{
	auto text = _edit->toPlainText();
	if(text.isEmpty()){
		hideDownloads();
		return;
	}
	auto uris = text.split('\n');
	QSet<QUrl> currentUrls;
	for(auto iter = _items.begin(); iter != _items.end(); iter++)
		currentUrls.insert(iter.key());

	auto validUrl = [](QUrl &url)->bool{
		if(!url.isValid())
			return false;
		auto p = url.url();
		if(QString("https://").contains(p) || QString("http://").contains(p) ||
				QString("ftp://").contains(p))
			return false;
		std::regex url_regex (
		  R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
		  //R"((http|ftp)://(([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
		  std::regex::extended );
		std::smatch regex_result;
		std::string spath = p.toStdString();
		if(!std::regex_match(spath, regex_result, url_regex))
			return false;
		return true;
	};

	QSet<QUrl> newUrls;
	for(auto &uri : uris){
		QUrl quri(uri);
		if(validUrl(quri))
			newUrls.insert(quri);
	}
	if(currentUrls == newUrls)
		return;

	_downList->clear();

	int count = 0;
	for(auto &uri : newUrls) {
		_downList->setRowCount(count + 1);
		auto item = new QTableWidgetItem(uri.fileName());
		item->setCheckState(Qt::Checked);
		item->setFlags(Qt::NoItemFlags);
		_items.insert(uri, item);
		_downList->setItem(count, 0, item);
	}
	showDownloads();
}

void URILinkWgt::addBtSlt()
{
	QFileDialog::getOpenFileNames(this, tr("open torrent files."), "", "Torrent (*.torrent)");
}

void URILinkWgt::downloadDirSlt()
{
	auto dir = QFileDialog::getExistingDirectory();
	if(dir.isEmpty())
	{
		auto dir = QString::fromStdString(ariaSetting::instance().downloadPath());
	}
	_downdir->setText(dir);
}

void URILinkWgt::createWidgets()
{
	setTitle("new link task");

	setFixedWidth(500);

	_edit = new QPlainTextEdit;
	auto btn = new QPushButton(tr("download"));

	_downList = new QTableWidget;
	_downList->setColumnCount(3);
	_downList->setColumnWidth(0, 250);
	_downList->horizontalHeader()->setStretchLastSection(true);
	_downList->verticalHeader()->hide();
	_downList->setSelectionMode(QAbstractItemView::NoSelection);
	_downList->setSelectionBehavior(QAbstractItemView::SelectRows);

	_downdir = new QLineEdit;
	_dnDirBtn = new QPushButton(QIcon(":/aria/icons/xx/folder.png"), "");
	auto dirLayout = new QHBoxLayout;
	dirLayout->addWidget(_downdir);
	dirLayout->addWidget(_dnDirBtn);

	btn->setMaximumWidth(100);
	_dnBtn = btn;
	btn = new QPushButton(tr("bt"));
	btn->setMaximumWidth(100);
	_btBtn = btn;
	auto btnLayout = new QHBoxLayout;
	btnLayout->addWidget(_btBtn, Qt::AlignLeft);
	btnLayout->addStretch();
	btnLayout->addWidget(_dnBtn, Qt::AlignRight);

	_layout->addSpacing(20);
	_layout->addWidget(_edit, 10);
	_layout->addWidget(_downList, 6);
	_layout->addLayout(dirLayout);
	_layout->addStretch(1);
	_layout->addLayout(btnLayout);
	_layout->addSpacing(20);

	hideDownloads();

	setStyleSheet("QWidget{font-family:\"Microsoft YaHei UI Light\"; font-size:16px;}");

	connect(_dnDirBtn, &QPushButton::clicked, this, &URILinkWgt::downloadDirSlt);
	connect(_dnBtn, &QPushButton::clicked, this, &URILinkWgt::downloadSlt);
	connect(_btBtn, &QPushButton::clicked, this, &URILinkWgt::addBtSlt);

	auto dir = ariaSetting::instance().downloadPath();
	_downdir->setText(QString::fromStdString(dir));
}

void URILinkWgt::showDownloads()
{
	setMinimumHeight(500);
	QStringList headrs; headrs << tr("name") << tr("size") << tr("type");
	_downList->setHorizontalHeaderLabels(headrs);
	_downList->show();
	_downdir->show();
	_dnDirBtn->show();
}

void URILinkWgt::hideDownloads()
{
	setMinimumHeight(300);
	_downList->hide();
	_downdir->hide();
	_dnDirBtn->hide();
	resize(500, 300);
}
