#ifndef FRAMELESS_H
#define FRAMELESS_H

#include <QDialog>

class QHBoxLayout;

class FramelessDlg: public QDialog{
public:
	typedef QDialog Base;

	FramelessDlg();

	~FramelessDlg();
protected:
	QHBoxLayout *_layout;
protected:
	void mousePressEvent(QMouseEvent *event);

	void mouseMoveEvent(QMouseEvent *event);

	void resizeEvent(QResizeEvent *);

	bool nativeEvent(const QByteArray &eventType, void *message, long *result);

private:
	int64_t _area[9];

	QPoint	_lastPt;
};

#endif // URILINK_H
