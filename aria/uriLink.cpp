#include "uriLink.h"

#include <QPlainTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

URILinkWgt::URILinkWgt()
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

std::vector<std::string>
URILinkWgt::getUris()
{
	auto text = _edit->toPlainText();
	auto uris = text.split('\n');
	std::vector<std::string> ret;
	for(auto &uri : uris)
		ret.push_back(uri.toStdString());
	return ret;
}

void URILinkWgt::downloadSlt()
{
	accept();
}
