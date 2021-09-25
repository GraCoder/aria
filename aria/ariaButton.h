#ifndef ARIABUTTON_H
#define ARIABUTTON_H

#include <QAbstractButton>

class AriaButton : public QAbstractButton{
public:
	typedef QAbstractButton Base;

protected:
	void enterEvent(QEvent *ev);

	void leaveEvent(QEvent *ev);
};

class AriaSysButton : public AriaButton
{
public:

	AriaSysButton(const QString &uri);

protected:
	void paintEvent(QPaintEvent *ev);

	QSize sizeHint() const;

private:
};

class AriaPanelButton : public AriaButton
{
public:
	typedef QAbstractButton Base;

	AriaPanelButton(const QString tex, const QString &uri);

protected:
	void paintEvent(QPaintEvent *ev);

	QSize sizeHint() const;

private:
	QString	_tex;
	QPixmap _pix;
};

#endif // ARIABUTTON_H
