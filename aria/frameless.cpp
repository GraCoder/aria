#include "frameless.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>

#include <QPainter>

#include "ariaButton.h"

#include <windows.h>

const int warea = 10, harea = 10;

const int lmargin = 1, rmargin = 3, tmargin = 1, bmargin = 3;

FramelessFrame::FramelessFrame(QWidget *par)
	: QDialog(par, Qt::FramelessWindowHint)
	, _widget(nullptr)
	, _fixsize(false)
{
	_area[0] = HTTOPLEFT;
	_area[1] = HTTOP;
	_area[2] = HTTOPRIGHT;
	_area[3] = HTLEFT;
	_area[4] = HTCLIENT;
	_area[5] = HTRIGHT;
	_area[6] = HTBOTTOMLEFT;
	_area[7] = HTBOTTOM;
	_area[8] = HTBOTTOMRIGHT;

	setAttribute(Qt::WA_TranslucentBackground, true);
}

FramelessFrame::~FramelessFrame()
{

}

void FramelessFrame::setWidget(QWidget *wgt)
{
	_widget = wgt;
	wgt->setParent(this);
	wgt->setGeometry(clientRect());
}

void FramelessFrame::setFixSize(bool fixsize)
{
	_fixsize = fixsize;
}

QRect FramelessFrame::clientRect()
{
	return QRect(lmargin, tmargin, width() - lmargin - rmargin, height() - tmargin - bmargin);
}

void FramelessFrame::mousePressEvent(QMouseEvent *event)
{
	_lastPt = event->globalPos();
	Base::mousePressEvent(event);
}

void FramelessFrame::mouseMoveEvent(QMouseEvent *event)
{
	auto rect = geometry();
	rect.translate(event->globalPos() - _lastPt);
	setGeometry(rect);
	_lastPt = event->globalPos();
	Base::mouseMoveEvent(event);
}

void FramelessFrame::resizeEvent(QResizeEvent *ev)
{
	Base::resizeEvent(ev);
	if(_widget)
		_widget->setGeometry(clientRect());
}

void FramelessFrame::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);

	{
		painter.fillRect(rect(), QColor(0, 0, 0, 18));
	}

	painter.fillRect(clientRect(), Qt::white);

	Base::paintEvent(ev);
}

bool FramelessFrame::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	if(eventType == "windows_generic_MSG"){
		auto msg = (MSG*)message;
		if (msg->message == WM_NCHITTEST){
			if(_fixsize) {
				*result = HTCLIENT;
				return true;
			}
			int x = msg->pt.x;
			int y = msg->pt.y;
			int w = width(), h = height();
			auto pt = mapFromGlobal(QPoint(x, y));
			x = pt.x(); y = pt.y();
			int xidx = x < warea ? 0 : x > w - warea ? 2 : 1;
			int yidx = y < harea ? 0 : y > h - harea ? 2 : 1;
			*result = _area[yidx * 3 + xidx];
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------------------------------------------------

FramelessDlg::FramelessDlg(QWidget *par)
	: FramelessFrame(par)
{
	auto btn = new AriaSysButton(":/aria/icons/application-exit.svg");
	connect(btn, &QAbstractButton::clicked, this, &FramelessDlg::close);

	auto clientWgt = new QWidget;
	setWidget(clientWgt);
	_layout = new QVBoxLayout(clientWgt);

	auto layout = new QHBoxLayout;
	layout->addStretch(1);
	layout->addWidget(btn, Qt::AlignTop);
	_sysLayout = layout;

	_layout->addLayout(layout);
	_layout->addStretch();
}

FramelessDlg::~FramelessDlg()
{

}

void FramelessDlg::setTitle(const QString &title)
{
	_sysLayout->insertWidget(0, new QLabel(title));
}
