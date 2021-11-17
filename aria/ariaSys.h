#ifndef ARIASYS_H
#define ARIASYS_H

#include <QWidget>

class AriaSysMenu : public QWidget
{
public:
	AriaSysMenu();

protected:
	void mouseDoubleClickEvent(QMouseEvent *ev);
public:
	void miniSlt();
};

#endif // ARIASYS_H
