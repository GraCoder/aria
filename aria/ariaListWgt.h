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
	friend class AriaListWidget;
public:
	AriaListDelegate();
	virtual void setSize(const QSize &);
protected:
	QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const;

	QPixmap &getPixmap(int) const;

protected:
	QRect _btnRect;
	mutable QMap<int, QPixmap> 		_pixMap;
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

	QRect _tskRect;
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

class TaskDetailWgt;

class AriaListWidget : public QListView
{
	Q_OBJECT
public:
	typedef QListView Base;

	AriaListWidget(AriListViewType type);
	AriListViewType type() const {return _type;}

	void 	addTaskSlt(uint64_t, Task*);
	void 	updateTaskSlt(uint64_t, TaskUpdateInfo);
	void 	removeTaskSlt(uint64_t);
	void 	failTaskSlt(uint64_t);
	void 	startTaskSlt(uint64_t);
	void 	pauseTaskSlt(uint64_t);

	void 	addFinishTaskSlt(uint64_t, FinishTaskInfo &);

	void 	setTaskState(uint64_t, int);

	QVector<uint64_t>	 getSelected();

	void 	explorerSelected();
	void 	explorerIndexAt(int);
	void 	restartTask(int);

	TaskInfoEx getTaskInfo(uint64_t);
	void 	showTaskDetail(uint64_t);
	void 	hideTaskDetail();
signals:
	void 	selectionChange(const QItemSelection &selected, const QItemSelection &deselected);

protected:
	QStyleOptionViewItem viewOptions() const;
	void 	resizeEvent(QResizeEvent *ev);
	void 	mouseMoveEvent(QMouseEvent *ev);
	void 	mousePressEvent(QMouseEvent *ev);
	void 	showEvent(QShowEvent *ev);

	void 	changeTaskState(uint64_t);

	void 	setTaskIcon(TaskInfo *);

	void 	selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
private:
	AriListViewType _type;

	TaskDetailWgt	*_taskDetailWgt;
};

class AriaDownloadListModel : public  QAbstractListModel{
	friend class AriaListWidget;
	friend class DownloadDelegate;
public:
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
private:
	QVector<uint64_t>				_tasks;
	QHash<uint64_t, TaskInfoEx> 	_taskInfos;
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
