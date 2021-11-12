#ifndef URIPANEL_H
#define URIPANEL_H

#include <QWidget>
#include <QPaintEvent>

class AriaDlg;

namespace aria2 {
	struct GlobalStat;
}

class AriaPanel : public QWidget{
	typedef QWidget Base;
public:
	AriaPanel(AriaDlg *);
	~AriaPanel();

	void updateGlobalStatSlt(aria2::GlobalStat &);

protected:
	void paintEvent(QPaintEvent *ev);

private:
	int _upSpeed;
	int _dnSpeed;
	int _active, _wait, _stop;
};

#endif // URILINK_H
