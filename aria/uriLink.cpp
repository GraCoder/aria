#include "uriLink.h"

#include <QPlainTextEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>


URILinkWgt::URILinkWgt(const QString &url)
{
	setMinimumSize(600, 400);
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

}

void URILinkWgt::addBtSlt()
{
	QFileDialog::getOpenFileNames(this, tr("open torrent files."), "", "Torrent (*.torrent)");
}

void URILinkWgt::createWidgets()
{
	setTitle("new link task");

	_edit = new QPlainTextEdit;
	auto btn = new QPushButton(tr("download"));
	btn->setMaximumSize(100, 40);
	btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_dnBtn = btn;

	btn = new QPushButton(tr("bt"));
	btn->setMaximumSize(100, 40);
	_btBtn = btn;

	auto btnLayout = new QHBoxLayout;
	btnLayout->addWidget(_btBtn, Qt::AlignLeft);
	btnLayout->addStretch();
	btnLayout->addWidget(_dnBtn, Qt::AlignRight);

	_layout->addSpacing(20);
	_layout->addWidget(_edit);
	_layout->addSpacing(20);
	_layout->addLayout(btnLayout);
	_layout->addSpacing(20);

	setStyleSheet("QWidget{font-family:\"Microsoft YaHei UI Light\"; font-size:16px;}");

	connect(_dnBtn, &QPushButton::clicked, this, &URILinkWgt::downloadSlt);
	connect(_btBtn, &QPushButton::clicked, this, &URILinkWgt::addBtSlt);
}
