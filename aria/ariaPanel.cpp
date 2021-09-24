#include "ariaPanel.h"

#include <QPainter>

AriaPanel::AriaPanel()
{
	setFixedWidth(200);
	setContentsMargins(0, 0, 0, 0);
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
}
