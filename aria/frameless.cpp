#include "frameless.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>

#include <windows.h>

const int warea = 20, harea = 20;

FramelessDlg::FramelessDlg()
	: QDialog(nullptr, Qt::FramelessWindowHint)
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

	auto layout = new QHBoxLayout(this);
	layout->setContentsMargins(warea / 2, harea / 2, warea / 2, harea / 2);
	auto wgt = new QLabel;
	wgt->setStyleSheet("QLabel{background-color: rgba(255, 255, 255, 255);}");
	layout->addWidget(wgt);
	_layout = new QHBoxLayout(wgt);
}

FramelessDlg::~FramelessDlg()
{

}

void FramelessDlg::mousePressEvent(QMouseEvent *event)
{
	_lastPt = event->globalPos();
	Base::mousePressEvent(event);
}

void FramelessDlg::mouseMoveEvent(QMouseEvent *event)
{
	auto rect = geometry();
	rect.translate(event->globalPos() - _lastPt);
	setGeometry(rect);
	_lastPt = event->globalPos();
	Base::mouseMoveEvent(event);
}

void FramelessDlg::resizeEvent(QResizeEvent *ev)
{
	Base::resizeEvent(ev);
}

bool FramelessDlg::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
	if(eventType == "windows_generic_MSG"){
		auto msg = (MSG*)message;
		if (msg->message == WM_NCHITTEST){
			int x = msg->lParam & 0xffff;
			int y = msg->lParam >> 16;
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
