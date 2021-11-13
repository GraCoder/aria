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

#include "json.hpp"

#include "ariaSetting.h"

#include "ValueBase.h"
#include "bitfield.h"
#include "ValueBaseBencodeParser.h"
#include "GenericParser.h"

URILinkWgt::URILinkWgt()
{
	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
	createWidgets();
	connect(_edit, &QPlainTextEdit::textChanged, this, &URILinkWgt::uriChangedSlt);
}

URILinkWgt::~URILinkWgt()
{

}

void URILinkWgt::initUrl(const QString &url, const QString &agent)
{
	if(url.isEmpty())
		return;
	QStringList urls;
	auto jss = nlohmann::json::parse(url.toUtf8().toStdString());
	if(jss.count("url_links")){
		auto uriArray = jss["url_links"];
		for(auto iter = uriArray.begin(); iter!= uriArray.end(); iter++)
		{
			std::string url, cookie, agent, filename, mime;
			uint64_t filesize;
			if(iter->contains("url"))
				url = (*iter)["url"];
			else
				continue;
			if(iter->contains("cookies"))
				cookie = (*iter)["cookies"];
			if(iter->contains("useragent"))
				agent = (*iter)["useragent"];
			if(iter->contains("filename"))
				filename = (*iter)["filename"];
			if(iter->contains("fileSize"))
				filesize = (*iter)["fileSize"];
			if(iter->contains("mime"))
				mime = (*iter)["mime"];

			if(url.empty())
				continue;

			QString qurl = QString::fromUtf8(url.c_str());
			urls << qurl;

			auto tsk = std::make_unique<UriTask>();
			tsk->url = url;
			tsk->name = filename;
			tsk->to_size = filesize;
			tsk->cookie = cookie;
			tsk->agent = agent;
			_uriTask[qurl] = std::move(tsk);
		}
	}
	_edit->setPlainText(urls.join('\n'));
}

std::vector<std::unique_ptr<Task> >
URILinkWgt::getTasks()
{
	std::vector<std::unique_ptr<Task> > ret;

	for(int i = 0; i < _downList->rowCount(); i++)
	{
		auto item = _downList->item(i, 0);
		QString url = item->data(Qt::UserRole).toUrl().url();
		std::unique_ptr<UriTask> tsk;
		if(_uriTask.find(url) != _uriTask.end())
		{
			tsk = std::move(_uriTask[url]);
			_uriTask.erase(url);
		} else{
			tsk = std::make_unique<UriTask>();
			tsk->url = item->data(Qt::UserRole).toUrl().toString().toUtf8().toStdString();
			tsk->name = item->text().toUtf8().toStdString();
		}
		tsk->id = aria2::genId();
		tsk->type = 1;
		tsk->state = aria2::DOWNLOAD_WAITING;
		tsk->dir = _downdir->text().toUtf8().toStdString();

		{
			tsk->opts.push_back(std::make_pair("gid", aria2::gidToHex(tsk->id)));
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

QString URILinkWgt::getFileName(QUrl &url)
{
	QMap<QString, QString> kv;
	auto fun = [&kv](QString &str)
	{
		auto slist = str.split(';');
		for(auto &s : slist)
		{
			auto kvs = s.split('=');
			if(kvs.size() != 2)
				continue;
			kv[kvs[0].trimmed()] = kvs[1].trimmed();
		}
	};

	QString filename;
	QString query = url.query(QUrl::FullyDecoded);
	if(query.isEmpty())
		return filename;
	auto list = query.split('&');
	for(auto &ql : list)
	{
		bool ret = ql.startsWith("response-content-disposition", Qt::CaseInsensitive);

		if(ret)
		{
			fun(ql);
			filename = kv["filename"];
			break;
		}
	}
	return filename;
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
		if(p.startsWith("https://") || p.startsWith("http://") ||
				p.startsWith("ftp://"))
			return true;
		return false;
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

		QString fileName, fileSize, fileType;

		QString surl = iter.value().url();
		if(_uriTask.find(surl) != _uriTask.end())
		{
			auto &tsk = _uriTask[surl];
			fileName = QString::fromUtf8(tsk->name.c_str());
			fileSize = locale().formattedDataSize(tsk->to_size, 0, QLocale::DataSizeSIFormat);
			fileType = "";
		}
		else{
			fileName = getFileName(iter.value());
			if(fileName.isEmpty())
				fileName = iter.value().fileName();
			fileSize = "0";
			fileType = "";
		}


		auto item = new QTableWidgetItem(fileName);
		item->setCheckState(Qt::Checked);
		item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
		item->setData(Qt::UserRole, iter.value());
		_downList->setItem(pos, 0, item);

		item = new QTableWidgetItem(fileSize);
		item->setFlags(Qt::NoItemFlags);
		_downList->setItem(pos, 1, item);

		item = new QTableWidgetItem(fileType);
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
	std::vector<std::tuple<QString, uint64_t>> fileList;
	auto pFiles = downcast<List>(torrentInfo->get("files"));
	if(pFiles){

	}else{
		uint64_t sz = downcast<Integer>(torrentInfo->get("length"))->i();
		fileList.push_back(std::make_tuple(taskName, sz));
	}

	auto tsk = std::make_unique<BtTask>();
	tsk->id = aria2::genId();
	tsk->type = 2;
	tsk->name = taskName.toUtf8().toStdString();
	tsk->torrent = file.toUtf8().toStdString();
	tsk->state = aria2::DOWNLOAD_WAITING;
	{
		tsk->opts.push_back(std::make_pair("gid", aria2::gidToHex(tsk->id)));
		tsk->opts.push_back(std::make_pair("dir", _downdir->text().toUtf8().toStdString()));
	}
	return tsk;
}

void URILinkWgt::createWidgets()
{
	setTitle("new link task");

	_edit = new QPlainTextEdit;
	auto btn = new QPushButton(tr("download"));

	_downList = new QTableWidget;
	_downList->setColumnCount(3);
	_downList->horizontalHeader()->setStretchLastSection(true);
	_downList->verticalHeader()->hide();
	_downList->setSelectionBehavior(QAbstractItemView::SelectRows);
	_downList->setFocusPolicy(Qt::NoFocus);

	_downdir = new QLineEdit;
	_dnDirBtn = new QPushButton(QIcon(":/aria/icons/folder.svg"), "");
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
