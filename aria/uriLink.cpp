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
#include <QFileInfo>

#include <regex>
#include <filesystem>

#include "ariaSetting.h"

#include "ValueBase.h"
#include "bitfield.h"
#include "ValueBaseBencodeParser.h"
#include "GenericParser.h"

URILinkWgt::URILinkWgt(const QString &url)
{
	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
	createWidgets();
	connect(_edit, &QPlainTextEdit::textChanged, this, &URILinkWgt::uriChangedSlt);

	_edit->setPlainText(url);
}

URILinkWgt::~URILinkWgt()
{

}

std::vector<std::unique_ptr<Task> >
URILinkWgt::getTasks()
{
	std::vector<std::unique_ptr<Task> > ret;

	for(int i = 0; i < _downList->rowCount(); i++)
	{
		auto item = _downList->item(i, 0);
		auto tsk = std::make_unique<UriTask>();
		tsk->id = 0;
		tsk->url = item->data(Qt::UserRole).toUrl().toString().toUtf8().toStdString();
		tsk->name = item->text().toUtf8().toStdString();
		tsk->type = 1;
		tsk->dir = _downdir->text().toUtf8().toStdString();

		{
			tsk->opts.push_back(std::make_pair("out", tsk->name));
			tsk->opts.push_back(std::make_pair("dir", tsk->dir));
		}
		ret.push_back(std::move(tsk));
	}

	for(auto &tsk : _btTasks)
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

	QMap<QUrl, int> newUrls;
	for(int i = 0; i < uris.size(); i++){
		QUrl quri(uris[i]);
		if(validUrl(quri))
			newUrls.insert(quri, i);
	}

	for(int i = 0; i < _downList->rowCount(); i++)
	{
		auto url = _downList->item(i, 0)->data(Qt::UserRole).toUrl();
		if(newUrls.contains(url))
			newUrls.remove(url);
		else{
			_downList->removeRow(i);
			i--;
		}
	}
	QMap<int, QUrl> resultUrls;
	for(auto iter = newUrls.begin(); iter!=newUrls.end(); iter++)
		resultUrls.insert(iter.value(), iter.key());

	for(auto iter = resultUrls.begin(); iter != resultUrls.end(); iter++) {
		int pos = iter.key();
		_downList->insertRow(pos);
		auto item = new QTableWidgetItem(iter.value().fileName());
		item->setCheckState(Qt::Checked);
		item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
		item->setData(Qt::UserRole, iter.value());
		_downList->setItem(pos, 0, item);

		item = new QTableWidgetItem("0");
		item->setFlags(Qt::NoItemFlags);
		_downList->setItem(pos, 1, item);

		item = new QTableWidgetItem("0");
		item->setFlags(Qt::NoItemFlags);
		_downList->setItem(pos, 2, item);
	}
	showDownloads();
}

void URILinkWgt::addBtSlt()
{
	auto files = QFileDialog::getOpenFileNames(this, tr("open torrent files."), "", "Torrent (*.torrent)");
	if(files.isEmpty())
		return;

	hide();

	for(auto &file : files)
	{
		auto tsk = parseBtFile(file);
		_btTasks.push_back(std::move(tsk));
	}

	accept();
}

void URILinkWgt::downloadDirSlt()
{
	auto dir = QFileDialog::getExistingDirectory();
	if(dir.isEmpty())
	{
		dir = QString::fromStdString(ariaSetting::instance().downloadPath());
	}
	_downdir->setText(dir);
}

std::unique_ptr<Task>
URILinkWgt::parseBtFile(const QString &file)
{
	using namespace aria2;

	std::unique_ptr<ValueBase> torrent;
	bittorrent::ValueBaseBencodeParser parser;
	torrent = parseFile(parser, file.toUtf8().toStdString());
	const Dict* torrentDict = downcast<Dict>(torrent);
	const Dict* torrentInfo = downcast<Dict>(torrentDict->get("info"));
	if(torrentInfo == nullptr)
		return nullptr;
	auto pName = downcast<String>(torrentInfo->get("name"));
	QString taskName;
	if(pName)
		taskName = QString::fromUtf8(pName->s().c_str());
	else
		taskName = QFileInfo(file).fileName();
	QStringList fileList;
	auto pFiles = downcast<List>(torrentInfo->get("files"));
	if(pFiles){

	}else{

	}

	auto tsk = std::make_unique<BtTask>();
	tsk->id = 0;
	tsk->type = 2;
	tsk->name = taskName.toUtf8().toStdString();
	tsk->torrent = file.toUtf8().toStdString();
	{
		tsk->opts.push_back(std::make_pair("dir", _downdir->text().toUtf8().toStdString()));
	}
	return tsk;
}

void URILinkWgt::createWidgets()
{
	setTitle("new link task");

	setFixedWidth(500);

	_edit = new QPlainTextEdit;
	auto btn = new QPushButton(tr("download"));

	_downList = new QTableWidget;
	_downList->setColumnCount(3);
	_downList->horizontalHeader()->setStretchLastSection(true);
	_downList->verticalHeader()->hide();
	_downList->setSelectionBehavior(QAbstractItemView::SelectRows);
	_downList->setFocusPolicy(Qt::NoFocus);

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
