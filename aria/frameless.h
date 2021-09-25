#ifndef FRAMELESS_H
#define FRAMELESS_H

#include <QDialog>
#include <QBoxLayout>

class QHBoxLayout;
class QVBoxLayout;

class FramelessFrame: public QDialog{
public:
	typedef QDialog Base;

	FramelessFrame(QBoxLayout::Direction dir = QBoxLayout::LeftToRight, QWidget *par = nullptr);

	~FramelessFrame();

	void setWidget(QWidget *);

	void setFixSize(bool);
protected:
	void mousePressEvent(QMouseEvent *event);

	void mouseMoveEvent(QMouseEvent *event);

	void resizeEvent(QResizeEvent *);

	bool nativeEvent(const QByteArray &eventType, void *message, long *result);

protected:
	QBoxLayout *_layout;
private:
	int64_t _area[9];

	QPoint	_lastPt;

	bool	_fixsize;
};

class FramelessDlg : public FramelessFrame{
	Q_OBJECT
public:
	FramelessDlg(QBoxLayout::Direction dir = QBoxLayout::TopToBottom, QWidget *par = nullptr);
	~FramelessDlg();

	void setTitle(const QString &);
signals:
	void miniSig();
	void maxiSig();
	void closeSig();
protected:
private:
	QHBoxLayout *_sysLayout;
};

#endif // URILINK_H
