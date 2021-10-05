#ifndef ARIALISTWGT_H
#define ARIALISTWGT_H

#include <QListWidget>
#include "taskInfo.h"

enum AriListViewType{
	DOWNLOADING,
	COMPLETED,
	TRASHCAN
};

class AriaListDelegate: public QAbstractItemDelegate{
public:
	virtual void setSize(const QSize &);
protected:
	QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const;
};

//--------------------------------------------------------------------------------

class DownloadDelegate : public AriaListDelegate{
	friend class AriaListWidget;
public:
	DownloadDelegate();

	void setSize(const QSize &);
protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const;

	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
	QRect _btnRect;
};

//--------------------------------------------------------------------------------

class FinishListDelegate : public AriaListDelegate{
protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const;
};

//--------------------------------------------------------------------------------

class CompleteDelegate : public FinishListDelegate{

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const;
};

//--------------------------------------------------------------------------------

class TrashDelegate : public FinishListDelegate{

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const;
};

//--------------------------------------------------------------------------------

class AriaListWidget : public QListView
{
	Q_OBJECT
public:
	typedef QListView Base;

	AriaListWidget(AriListViewType type);

	void 	addTaskSlt(uint64_t, QString);
	void 	updateTaskSlt(uint64_t, TaskInfo);
	void 	removeTaskSlt(uint64_t);
	void 	failTaskSlt(uint64_t);
	void 	startTaskSlt(uint64_t);
	void 	pauseTaskSlt(uint64_t);

	void 	addFinishTaskSlt(uint64_t, FinishTaskInfo &);

	void 	setTaskState(uint64_t, int);

	QVector<uint64_t>	 getSelected();
//signals:
//	void 	startTasksSig(uint64_t &);
//	void 	pauseTasksSig(uint64_t &);
//	void 	deleteTasksSig(uint64_t &);
protected:
	void 	resizeEvent(QResizeEvent *ev);
	void 	mouseMoveEvent(QMouseEvent *ev);
	void 	mousePressEvent(QMouseEvent *ev);
	void 	showEvent(QShowEvent *ev);

	void 	changeTaskState(uint64_t);
private:
	AriListViewType _type;
};

class AriaDownloadListModel : public  QAbstractListModel{
	friend class AriaListWidget;
	friend class DownloadDelegate;
public:
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
private:
	QVector<uint64_t>			_tasks;
	QHash<uint64_t, TaskInfo> 	_taskInfos;
};

class AriaFinishListModel : public QAbstractListModel{
	friend class AriaListWidget;
	friend class FinishListDelegate;
	friend class CompleteDelegate;
	friend class TrashDelegate;
public:
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;

private:
	QVector<FinishTaskInfo> _taskInfos;
};

#endif // ARIALISTWGT_H
