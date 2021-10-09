#include "uriLink.h"

#include <QPlainTextEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QListWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <regex>

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

//	std::vector<std::string> tmp;
//	tmp.push_back("http://www.sqliteexpert.com/v5/SQLiteExpertPersSetup64.exe");
//	tmp.push_back("http://ftp.dlut.edu.cn/centos/2/centos2-scripts-v1.tar");
//	tsk->url = tmp;

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
	auto uris = text.split('\n');
	if(uris.empty()){
		setMinimumHeight(300);
		return;
	}
	QSet<QUrl> currentUrls;
	for(auto iter = _items.begin(); iter != _items.end(); iter++)
		currentUrls.insert(iter.key());

	auto validUrl = [](QUrl &url)->bool{
		if(!url.isValid())
			return false;
		auto p = url.path();
		if(p == "http" || p == "https")
			return false;
		std::regex url_regex (
		  R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
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
		_items.insert(uri, item);
		_downList->setItem(count, 0, item);
	}

	setMinimumHeight(500);
}

void URILinkWgt::addBtSlt()
{
	QFileDialog::getOpenFileNames(this, tr("open torrent files."), "", "Torrent (*.torrent)");
}

void URILinkWgt::createWidgets()
{
	setTitle("new link task");

	setFixedWidth(500);
	setMinimumHeight(300);

	_edit = new QPlainTextEdit;
	auto btn = new QPushButton(tr("download"));

	_downList = new QTableWidget;
	_downList->setColumnCount(3);
	QStringList headrs; headrs << tr("name") << tr("size") << tr("type");
	_downList->setHorizontalHeaderLabels(headrs);
	_downList->setColumnWidth(0, 200);
	_downList->horizontalHeader()->stretchLastSection();

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
	_layout->addStretch(1);
	_layout->addLayout(btnLayout);
	_layout->addSpacing(20);

	setStyleSheet("QWidget{font-family:\"Microsoft YaHei UI Light\"; font-size:16px;}");

	connect(_dnBtn, &QPushButton::clicked, this, &URILinkWgt::downloadSlt);
	connect(_btBtn, &QPushButton::clicked, this, &URILinkWgt::addBtSlt);
}
