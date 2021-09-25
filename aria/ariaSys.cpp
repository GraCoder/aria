#include "ariaSys.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>

AriaSysMenu::AriaSysMenu()
{
	setFixedHeight(40);

	auto layout = new QHBoxLayout(this);
	auto btn = new QToolButton;
	btn->setIcon(QIcon(":/aria/icons/application-exit.svg"));
	layout->addStretch();
	layout->addWidget(btn, Qt::AlignTop);

	connect(btn, &QToolButton::clicked, this, &AriaSysMenu::miniSlt);
}

void AriaSysMenu::miniSlt()
{
	topLevelWidget()->showMinimized();
}
