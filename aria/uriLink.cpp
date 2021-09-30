#include "uriLink.h"

#include <QPlainTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>


URILinkWgt::URILinkWgt(const QString &url)
{
	createWidgets();
	_edit->setPlainText(url);
}

std::vector<std::unique_ptr<UriTask> >
URILinkWgt::getTasks()
{
	std::vector<std::unique_ptr<UriTask> > ret;

	auto text = _edit->toPlainText();
	auto uris = text.split('\n');
	auto tsk = std::make_unique<UriTask>();
	for(auto &uri : uris)
		tsk->url.push_back(uri.toStdString());

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

void URILinkWgt::createWidgets()
{
	setTitle("new link task");

	_edit = new QPlainTextEdit;
	auto btn = new QPushButton(tr("download"));
	btn->setMaximumSize(100, 40);
	btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	_dnBtn = btn;

	auto btnLayout = new QHBoxLayout;
	btnLayout->addStretch();
	btnLayout->addWidget(btn, Qt::AlignRight);

	_layout->addSpacing(20);
	_layout->addWidget(_edit);
	_layout->addSpacing(20);
	_layout->addLayout(btnLayout);
	_layout->addSpacing(20);

	setStyleSheet("QWidget{font-family:\"Microsoft YaHei UI Light\"; font-size:16px; font-weight:100;}");

	connect(_dnBtn, &QPushButton::clicked, this, &URILinkWgt::downloadSlt);
}
