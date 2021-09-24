#ifndef URIPANEL_H
#define URIPANEL_H

#include <QWidget>
#include <QPaintEvent>

class AriaPanel : public QWidget{
public:
	AriaPanel();
	~AriaPanel();

protected:
	void paintEvent(QPaintEvent *ev);
};

#endif // URILINK_H
