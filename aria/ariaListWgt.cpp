#include "ariaListWgt.h"

#include <QPainter>
#include <QApplication>
#include <QLocale>

const int dnheight = 80;

DownloadDelegate::DownloadDelegate()
{

}

void DownloadDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	auto wgt = dynamic_cast<const AriaListWidget *>(opt.widget);
	auto listmodel = static_cast<AriaListModel*>(wgt->model());
	auto &info = listmodel->_taskInfos[listmodel->_tasks[index.row()]];

	//painter->fillRect(opt.rect, QColor(255, 0, 0));

	auto ft = opt.font; ft.setPixelSize(dnheight / 5.0 * 2);
	painter->setFont(ft);

	const int hfheight = dnheight / 5.0 * 2;
	QRect texRect(dnheight, 0, opt.rect.width() / 2 - dnheight, dnheight / 5.0 * 2);
	painter->drawText(texRect, info.name);
	texRect.setTop(dnheight / 5.0 * 3);
	painter->drawText(texRect, opt.locale.formattedDataSize(info.totalLength));

	auto dnspeed = opt.locale.formattedDataSize(info.dnspeed);
	texRect = QRect(opt.rect.width() - 3 * dnheight, dnheight	/ 5.0 * 3, dnheight * 3, hfheight);
	painter->drawText(texRect, dnspeed);

	double progress = info.dnloadLength / double(info.totalLength) * 100;
}

QSize DownloadDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	return QSize(100, dnheight);
}

//--------------------------------------------------------------------------------

AriaListWidget::AriaListWidget(AriListViewType type)
{
	switch (type){
	case DOWNLOADING:
		setItemDelegate(new DownloadDelegate);
		break;
	case COMPLETED:
		break;
	case TRACHCAN:
		break;
	}
	setModel(new AriaListModel);

	setStyleSheet("QListView{border:none;}");
}

void AriaListWidget::addTaskSlt(uint64_t aid, QString name)
{
	auto listmodel = static_cast<AriaListModel*>(model());
	if(listmodel->_taskInfos.find(aid) != listmodel->_taskInfos.end())
		return;

	int count = listmodel->rowCount();
	listmodel->beginInsertRows(QModelIndex(), count, count);
	listmodel->_tasks.push_back(aid);
	TaskInfo tskInfo; tskInfo.name = name;
	listmodel->_taskInfos.insert(aid, tskInfo);
	listmodel->endInsertRows();
}

void AriaListWidget::updateTaskSlt(uint64_t aid, TaskInfo tskInfo)
{
	auto listmodel = static_cast<AriaListModel*>(model());
	if(listmodel->_taskInfos.find(aid) == listmodel->_taskInfos.end())
		return;
	listmodel->_taskInfos[aid] = tskInfo;
	auto idx = listmodel->createIndex(listmodel->_tasks.indexOf(aid), 0);
	listmodel->dataChanged(idx, idx);
}

void AriaListWidget::completeTaskSlt(uint64_t aid)
{
	auto listmodel = static_cast<AriaListModel*>(model());
	if(listmodel->_taskInfos.find(aid) == listmodel->_taskInfos.end())
		return;
	int idx = listmodel->_tasks.indexOf(aid);
	listmodel->beginRemoveRows(QModelIndex(), idx, idx);
	listmodel->_tasks.removeAt(idx);
	listmodel->_taskInfos.remove(aid);
	listmodel->endRemoveRows();
}

//--------------------------------------------------------------------------------

QVariant AriaListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();
	return _taskInfos[_tasks[index.row()]].name;
}

int AriaListModel::rowCount(const QModelIndex &parent) const
{
	return _tasks.size();
}

Qt::ItemFlags AriaListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	return QAbstractItemModel::flags(index);
}
