#include "ariaButton.h"
#include <QPainter>
#include <QColorTransform>


void AriaButton::enterEvent(QEvent *ev)
{
	update();
	Base::enterEvent(ev);
}

void AriaButton::leaveEvent(QEvent *ev)
{
	update();
	Base::leaveEvent(ev);
}

//--------------------------------------------------------------------------------

AriaSysButton::AriaSysButton(const QString &uri)
{
	setIcon(QPixmap(uri));
	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	setMinimumSize(16, 16);
}

void AriaSysButton::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	QRect rt(0, 0, iconSize().width(), iconSize().height());
	painter.drawPixmap(rect(), icon().pixmap(rt.width(), rt.height()), rt);
	if(!underMouse())
	{
		painter.fillRect(rect(), QColor(255, 255, 255, 100));
	}
}

QSize AriaSysButton::sizeHint() const
{
	return QSize(16, 16);
}

//--------------------------------------------------------------------------------

const int panelHeight = 60;

AriaPanelButton::AriaPanelButton(const QString tex, const QString &uri)
{
	_tex = tex;
	_pix = QPixmap(uri);
}

void AriaPanelButton::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
	if(isChecked())
	{
		QLinearGradient gradient(0, 0, width(), 0);
		gradient.setColorAt(0, QColor(19, 44, 113));
		gradient.setColorAt(1, QColor(61, 80, 191));
		painter.fillRect(rect(),gradient);
	}

	int x = width() / 6.0, h = 20, y = (height() - h) / 2.0;
	painter.drawPixmap(QRect(x, y, h, h), _pix, _pix.rect());
	x = x + h + 10;
	QTextOption opt; opt.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	if(isChecked() || underMouse())
	{
		painter.setPen(QColor(237, 148, 120));
	}
	else
	{
		painter.setPen(Qt::white);
	}
	QFont ft("Microsoft YaHei");
	//ft.setBold(true);
	ft.setPixelSize(h);
	painter.setFont(ft);
	painter.drawText(QRect(x, 0, width() - x, height()), _tex, opt);
}

QSize AriaPanelButton::sizeHint() const
{
	return QSize(400, panelHeight);
}
