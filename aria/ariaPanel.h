#ifndef URIPANEL_H
#define URIPANEL_H

#include <QWidget>
#include <QPaintEvent>

class AriaDlg;

class AriaPanel : public QWidget{
	typedef QWidget Base;
public:
	AriaPanel(AriaDlg *);
	~AriaPanel();

protected:
	void paintEvent(QPaintEvent *ev);

};

#endif // URILINK_H
