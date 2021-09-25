#include "ariaPanel.h"

#include <QPainter>
#include <QToolButton>
#include <QVBoxLayout>
#include <QToolBar>
#include <QButtonGroup>

#include "ariaButton.h"
#include "ariaUi.h"

AriaPanel::AriaPanel(AriaDlg *dlg)
{
	setFixedWidth(250);
	setContentsMargins(0, 0, 0, 0);	

	auto layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addSpacing(200);

	auto grp = new QButtonGroup;
	grp->setExclusive(true);

	{
		auto btn = new AriaPanelButton(tr("download"), ":/aria/icons/downloading.png");
		btn->setCheckable(true);
		layout->addWidget(btn);
		grp->addButton(btn);
		btn->setChecked(true);
		connect(btn, &QAbstractButton::clicked, std::bind(&AriaDlg::changeViewSig, dlg, 0));
	}

	{
		auto btn = new AriaPanelButton(tr("completed"), ":/aria/icons/completed.png");
		btn->setCheckable(true);
		layout->addWidget(btn);
		grp->addButton(btn);
		connect(btn, &QAbstractButton::clicked, std::bind(&AriaDlg::changeViewSig, dlg, 1));
	}

	{
		auto btn = new AriaPanelButton(tr("trashcan"), ":/aria/icons/list-remove.png");
		btn->setCheckable(true);
		layout->addWidget(btn);
		grp->addButton(btn);
		connect(btn, &QAbstractButton::clicked, std::bind(&AriaDlg::changeViewSig, dlg, 2));
	}


	layout->addStretch(10);
}

AriaPanel::~AriaPanel()
{
}

void AriaPanel::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	QLinearGradient gradient(0, 0, width(), 0);
	gradient.setColorAt(0, QColor(37, 68, 163));
	gradient.setColorAt(1, QColor(52, 76, 181));
	painter.setBrush(gradient);
	painter.drawRect(rect());

	Base::paintEvent(ev);
}
