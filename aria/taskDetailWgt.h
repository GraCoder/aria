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

private:
	uint64_t _id;

	QLabel 	*_name, *_hashlabel, *_hash;

	QLabel *_picSize, *_picNum, *_picCom, *_connections;
};

#endif // TASKDETAILWGT_H
