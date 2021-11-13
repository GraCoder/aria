#include "ariaPanel.h"

#include <QPainter>
#include <QToolButton>
#include <QVBoxLayout>
#include <QToolBar>
#include <QButtonGroup>
#include <QFontMetrics>
#include <QLocale>

#include "ariaButton.h"
#include "ariaUi.h"

AriaPanel::AriaPanel(AriaDlg *dlg)
{
	setFixedWidth(300);
	setContentsMargins(0, 0, 0, 0);	

	auto layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addSpacing(200);

	auto grp = new QButtonGroup;
	grp->setExclusive(true);

	{
		auto btn = new AriaPanelButton(tr("download"), ":/aria/icons/downloading.svg");
		btn->setCheckable(true);
		layout->addWidget(btn);
		grp->addButton(btn);
		btn->setChecked(true);
		connect(btn, &QAbstractButton::clicked, std::bind(&AriaDlg::changeViewSig, dlg, 0));
	}

	{
		auto btn = new AriaPanelButton(tr("completed"), ":/aria/icons/complete.svg");
		btn->setCheckable(true);
		layout->addWidget(btn);
		grp->addButton(btn);
		connect(btn, &QAbstractButton::clicked, std::bind(&AriaDlg::changeViewSig, dlg, 1));
	}

	{
		auto btn = new AriaPanelButton(tr("trashcan"), ":/aria/icons/trash.svg");
		btn->setCheckable(true);
		layout->addWidget(btn);
		grp->addButton(btn);
		connect(btn, &QAbstractButton::clicked, std::bind(&AriaDlg::changeViewSig, dlg, 2));
	}
	layout->addStretch(10);

	qRegisterMetaType<aria2::GlobalStat>("GlobalStat");
	connect(dlg, &AriaDlg::updateGlobalStat, this, &AriaPanel::updateGlobalStatSlt);

	_dnSpeed = 0;
	_upSpeed = 0;
	_active = _stop = _wait = 0;
}

AriaPanel::~AriaPanel()
{
}

void AriaPanel::updateGlobalStatSlt(aria2::GlobalStat &stat)
{
	_dnSpeed = stat.downloadSpeed;
	_upSpeed = stat.uploadSpeed;
	_active = stat.numActive;
	_stop = stat.numStopped;
	_wait = stat.numWaiting;

	update();
}

void AriaPanel::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	QLinearGradient gradient(0, 0, width(), 0);
	gradient.setColorAt(0, QColor(37, 68, 163));
	gradient.setColorAt(1, QColor(52, 76, 181));
	painter.setBrush(gradient);
	painter.drawRect(rect());

	painter.setPen(Qt::white);
	QFont ft = painter.font();
	ft.setPixelSize(16);
	painter.setFont(ft);
	int h = QFontMetrics(ft).height();
	QRect rt(width() / 4, height() / 6 * 5, width(), h);

	auto lo = locale();
	QString dnString;
	if(_dnSpeed > 1024)
	{
		dnString = lo.formattedDataSize(_dnSpeed, 0, lo.DataSizeSIFormat);
		dnString = dnString.rightJustified(7);
	}else
		dnString = " <1 KB";
	painter.drawText(rt, Qt::AlignLeft, tr("dnload:") + dnString + "/s");

	QString upString;
	if(_upSpeed > 1024)
	{
		upString  = lo.formattedDataSize(_upSpeed, 0, lo.DataSizeSIFormat);
		upString = upString.rightJustified(7);
	}else
		upString = " <1 KB";

	rt.translate(0, h + 20);
	painter.drawText(rt, Qt::AlignLeft, tr("upload:") + upString + "/s");

	rt.translate(0, h + 20);
	QString numInfo = QString(" ac:%1  st:%2  wa:%3")
			.arg(QString::number(_active), QString::number(_stop), QString::number(_wait));
	painter.drawText(rt, Qt::AlignLeft, numInfo);

	Base::paintEvent(ev);
}
