#ifndef FRAMELESS_H
#define FRAMELESS_H

#include <QDialog>
#include <QBoxLayout>

class QHBoxLayout;
class QVBoxLayout;

class FramelessFrame: public QDialog{
public:
	typedef QDialog Base;

	FramelessFrame(QWidget *par = nullptr);

	~FramelessFrame();

	void setWidget(QWidget *);

	void setFixSize(bool);
protected:
	QRect clientRect();

	void mousePressEvent(QMouseEvent *event);

	void mouseMoveEvent(QMouseEvent *event);

	void resizeEvent(QResizeEvent *);

	void paintEvent(QPaintEvent *ev);

	bool nativeEvent(const QByteArray &eventType, void *message, long *result);

protected:
	QWidget *_widget;
private:
	int64_t _area[9];

	QPoint	_lastPt;

	bool	_fixsize;
};

class FramelessDlg : public FramelessFrame{
	Q_OBJECT
public:
	FramelessDlg(QWidget *par = nullptr);
	~FramelessDlg();

	void setTitle(const QString &);
signals:
	void miniSig();
	void maxiSig();
	void closeSig();
protected:
	QVBoxLayout *_layout;
private:
	QHBoxLayout *_sysLayout;
};

#endif // URILINK_H
