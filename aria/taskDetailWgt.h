#ifndef TASKDETAILWGT_H
#define TASKDETAILWGT_H

#include <set>

#include <QWidget>

#include "taskInfo.h"

class QLabel;

class TaskDetailWgt : public QWidget
{
	Q_OBJECT
public:
	TaskDetailWgt(QWidget *);

	void fillTaskDetail(TaskInfoEx &);

	std::set<int> getPieces(const std::string &);
protected:
	void paintEvent(QPaintEvent *ev);
	void showEvent(QShowEvent *ev);
	void hideEvent(QHideEvent *ev);

	void updateTaskInfo();
private:

	QLabel 	*_name;

	QLabel *_picSize, *_picNum, *_picCom, *_connections;

	QTimer *m_timer;

	TaskInfoEx _tskInfo;
};

#endif // TASKDETAILWGT_H
