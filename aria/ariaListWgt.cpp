#include "ariaListWgt.h"

#include <QPainter>
#include <QFileIconProvider>
#include <QLocale>
#include <QTime>

const int dnheight = 80;

DownloadDelegate::DownloadDelegate()
{

}

void DownloadDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	auto wgt = dynamic_cast<const AriaListWidget *>(opt.widget);
	auto listmodel = static_cast<AriaListModel*>(wgt->model());
	auto &info = listmodel->_taskInfos[listmodel->_tasks[index.row()]];

	if(opt.state & QStyle::State_Selected || opt.state & QStyle::State_MouseOver)
		painter->fillRect(opt.rect, QColor(227, 230, 228));
	painter->setRenderHint(QPainter::Antialiasing);

	const int popSize = dnheight / 5.0 * 2;

	auto ico = QFileIconProvider().icon(QFileIconProvider::File).pixmap(32, 32);
	painter->drawPixmap(QRect(dnheight / 5, dnheight / 5 + opt.rect.top(), dnheight - popSize , dnheight - popSize), ico, QRect(0, 0, 32, 32));

	auto ft = opt.font; ft.setPixelSize(dnheight / 6.0);
	ft.setBold(true);
	painter->setFont(ft);

	QTextOption texOpt;
	texOpt.setAlignment(Qt::AlignLeft | Qt::AlignBottom);
	painter->setPen(QColor(25, 25, 25));
	const int hfheight = dnheight / 5.0 * 2;
	QRect texRect(dnheight, opt.rect.top(), opt.rect.width() / 2 - dnheight, popSize);
	painter->drawText(texRect, info.name, texOpt);

	texOpt.setAlignment(Qt::AlignLeft | Qt::AlignTop);
	painter->setPen(QColor(100, 100, 100));
	texRect.translate(0, dnheight / 5.0 * 3);
	auto filesize = opt.locale.formattedDataSize(info.dnloadLength, 2, opt.locale.DataSizeTraditionalFormat) + "/" +
			opt.locale.formattedDataSize(info.totalLength, 2, opt.locale.DataSizeTraditionalFormat);
	painter->drawText(texRect, filesize, texOpt);

	int secs = (info.totalLength - info.dnloadLength) / (info.dnspeed + 0.0001);
	texRect.translate(opt.rect.width() / 5, 0);
	auto tm = QTime(0, 0).addSecs(secs);
	if(tm.isValid())
		painter->drawText(texRect, tr("remain:") + opt.locale.toString(tm), texOpt);

	auto dnspeed = opt.locale.formattedDataSize(info.dnspeed, 2, opt.locale.DataSizeTraditionalFormat);
	texRect = QRect(opt.rect.width() - dnheight, dnheight - popSize + opt.rect.top(), dnheight, hfheight);
	painter->drawText(texRect, dnspeed + "/s", texOpt);

	if(info.dnloadLength > 0 && info.totalLength > info.dnloadLength){
		QRect prgRect(dnheight, dnheight / 2.0, opt.rect.width() - dnheight, 4);
		prgRect.translate(0, opt.rect.top());
		QLinearGradient gradient(prgRect.left(), 0, prgRect.right(), 0);
		gradient.setColorAt(0, QColor(255, 212, 102));
		gradient.setColorAt(1, QColor(255, 160, 120));
		float progress = info.dnloadLength / float(info.totalLength + 1);
		prgRect.setWidth(prgRect.width() * progress);
		painter->fillRect(prgRect, gradient);
	}
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
	setSelectionMode(QAbstractItemView::ExtendedSelection);

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

void AriaListWidget::addCompleteTaskSlt(uint64_t)
{

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
