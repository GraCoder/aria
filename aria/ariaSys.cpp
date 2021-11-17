#include "ariaSys.h"
#include <QHBoxLayout>


#include "ariaButton.h"

AriaSysMenu::AriaSysMenu()
{
	setFixedHeight(40);

	auto layout = new QHBoxLayout(this);
	auto btn = new AriaSysButton(":/aria/icons/application-exit.svg");
	layout->addStretch(1);
	layout->addWidget(btn, Qt::AlignTop);

	connect(btn, &QAbstractButton::clicked, this, &AriaSysMenu::miniSlt);
}

void AriaSysMenu::mouseDoubleClickEvent(QMouseEvent *ev)
{
	QWidget::mouseDoubleClickEvent(ev);

	auto wgt = topLevelWidget();
	if(wgt->isMaximized())
		wgt->showNormal();
	else
		wgt->showMaximized();
}

void AriaSysMenu::miniSlt()
{
	topLevelWidget()->hide();
}
