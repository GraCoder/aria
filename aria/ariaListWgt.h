#ifndef ARIALISTWGT_H
#define ARIALISTWGT_H

#include <QListWidget>
#include "taskInfo.h"

enum AriListViewType{
	DOWNLOADING,
	COMPLETED,
	TRACHCAN
};

class DownloadDelegate : public QAbstractItemDelegate{
	friend class AriaListWidget;
public:
	DownloadDelegate();

protected:
	void paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const;

	QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const;
};

//--------------------------------------------------------------------------------

class AriaListWidget : public QListView
{
	Q_OBJECT
public:
	AriaListWidget(AriListViewType type);

	void 	addTaskSlt(uint64_t, QString);
	void 	updateTaskSlt(uint64_t, TaskInfo);
	void 	completeTaskSlt(uint64_t);
private:
};

class AriaListModel : public  QAbstractListModel{
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

#endif // ARIALISTWGT_H
