#include "ariaListWgt.h"

#include <QScrollBar>
#include <QPainter>
#include <QFileIconProvider>
#include <QLocale>
#include <QTime>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QDir>
#include <QDesktopServices>

#include "ariaUi.h"
#include "taskDatabase.h"

const int dnheight = 80;
const int hfheight = dnheight / 5.0 * 2;

void AriaListDelegate::setSize(const QSize &size)
{
	_btnRect = QRect(size.width() - dnheight, 10, hfheight - 10, hfheight - 10);
}

QSize AriaListDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	return QSize(100, dnheight);
}

//--------------------------------------------------------------------------------

DownloadDelegate::DownloadDelegate()
{

}

void DownloadDelegate::setSize(const QSize &size)
{
	_btnRect = QRect(size.width() - dnheight, 10, hfheight - 10, hfheight - 10);
}

void DownloadDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	auto wgt = dynamic_cast<const AriaListWidget *>(opt.widget);
	auto listmodel = static_cast<AriaDownloadListModel*>(wgt->model());
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
	QRect texRect(dnheight, opt.rect.top(), opt.rect.width(), popSize);
	painter->drawText(texRect, info.name, texOpt);

	auto pt = QCursor::pos();
	pt = wgt->mapFromGlobal(pt);

	//RT---------------------------ICON-----------------------------------------------RT
	{
		QRect pixRect = _btnRect.translated(0, opt.rect.top());
		bool hover = !pixRect.contains(pt);
		QPixmap icon;  QString iconUrl = ":/aria/icons/xx/";
		if(info.state == aria2::DOWNLOAD_ACTIVE)
			iconUrl += hover ? "pause.svg" : "pause.png";
		else if(info.state == aria2::DOWNLOAD_ERROR || info.state == aria2::DOWNLOAD_PAUSED)
			iconUrl += hover ? "download.svg" : "download.png";
		else if(info.state == aria2::DOWNLOAD_WAITING)
		{
			//lol, what can i do, just a pargramer.
			pixRect = QRect(pixRect.left(), pixRect.top(), 16, 16);
			iconUrl += "waiting.svg";
		}
		icon = QPixmap(iconUrl);
		painter->drawPixmap(pixRect, icon, icon.rect());
	}

	{
		texOpt.setAlignment(Qt::AlignLeft | Qt::AlignTop);
		painter->setPen(QColor(100, 100, 100));
		texRect.translate(0, dnheight / 5.0 * 3);
		//ERROR---------------------------------------------------------------------------ERROR
		if(info.state == aria2::DOWNLOAD_ERROR)
		{
			painter->setPen(Qt::red);
			painter->drawText(texRect, tr("for unknown reason, failed."));
			return;
		}

		auto filesize = opt.locale.formattedDataSize(info.dnloadLength, 2, opt.locale.DataSizeTraditionalFormat) + "/" +
				opt.locale.formattedDataSize(info.totalLength, 2, opt.locale.DataSizeTraditionalFormat);
		painter->drawText(texRect, filesize, texOpt);
	}

	{
		if(info.state == aria2::DOWNLOAD_ACTIVE) {
			int secs = (info.totalLength - info.dnloadLength) / (info.dnspeed + 0.0001);
			texRect.translate(opt.rect.width() / 5, 0);
			auto tm = QTime(0, 0).addSecs(secs);
			if(tm.isValid())
				painter->drawText(texRect, tr("remain:") + opt.locale.toString(tm), texOpt);
		}
	}
	{
		texRect = QRect(opt.rect.width() / 2.0, dnheight - popSize + opt.rect.top(), opt.rect.width() / 2.0 - 20, hfheight);
		texOpt.setAlignment(Qt::AlignRight | Qt::AlignTop);
		if(info.state == aria2::DOWNLOAD_ACTIVE) {
			auto dnspeed = opt.locale.formattedDataSize(info.dnspeed, 2, opt.locale.DataSizeTraditionalFormat);
			painter->drawText(texRect, dnspeed + "/s", texOpt);
		}else if(info.state == aria2::DOWNLOAD_PAUSED)
			painter->drawText(texRect, tr("paused"), texOpt);
		else if(info.state == aria2::DOWNLOAD_WAITING)
			painter->drawText(texRect, tr("waiting"), texOpt);
	}
	{
		QRect prgRect(dnheight, dnheight / 2.0, opt.rect.width() - dnheight, 4);
		prgRect.translate(0, opt.rect.top());
		painter->fillRect(prgRect, Qt::gray);
		if(info.dnloadLength > 0 && info.totalLength > info.dnloadLength){
			QLinearGradient gradient(prgRect.left(), 0, prgRect.right(), 0);
			gradient.setColorAt(0, QColor(255, 212, 102));
			gradient.setColorAt(1, QColor(255, 160, 120));
			float progress = info.dnloadLength / float(info.totalLength + 1);
			prgRect.setWidth(prgRect.width() * progress);
			painter->fillRect(prgRect, gradient);
		}
	}
}

void DownloadDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QAbstractItemDelegate::updateEditorGeometry(editor, option, index);
}

//--------------------------------------------------------------------------------

void FinishListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	auto wgt = dynamic_cast<const AriaListWidget *>(opt.widget);
	auto listmodel = static_cast<AriaFinishListModel*>(wgt->model());
	auto &info = listmodel->_taskInfos[index.row()];

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
	QRect texRect(dnheight, opt.rect.top(), opt.rect.width(), popSize);
	painter->drawText(texRect, info.name, texOpt);

	auto pt = QCursor::pos();
	pt = wgt->mapFromGlobal(pt);

	{
		texRect.translate(0, hfheight);
		auto filesize = opt.locale.formattedDataSize(info.size, 2, opt.locale.DataSizeTraditionalFormat);
		painter->drawText(texRect, filesize, texOpt);
	}

	{
		QRect pixRect = _btnRect.translated(0, opt.rect.top());
		QPixmap icon;
		if(wgt->type() == COMPLETED )
			icon = QPixmap(":/aria/icons/xx/folder.png");
		painter->drawPixmap(pixRect, icon, icon.rect());
	}
}

//--------------------------------------------------------------------------------

void CompleteDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	FinishListDelegate::paint(painter, opt,  index);
}

//--------------------------------------------------------------------------------

void TrashDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	FinishListDelegate::paint(painter, opt,  index);
}

//--------------------------------------------------------------------------------

AriaListWidget::AriaListWidget(AriListViewType type)
	: _type(type)
{
	switch (type){
	case DOWNLOADING:
		setItemDelegate(new DownloadDelegate);
		setModel(new AriaDownloadListModel);
		break;
	case COMPLETED:
		setItemDelegate(new CompleteDelegate);
		setModel(new AriaFinishListModel);
		break;
	case TRASHCAN:
		setItemDelegate(new TrashDelegate);
		setModel(new AriaFinishListModel);
		break;
	}
	setSelectionMode(QAbstractItemView::ExtendedSelection);

	setMouseTracking(true);
	setStyleSheet("QListView{border:none;}");
	//verticalScrollBar()->hide();
	verticalScrollBar()->setStyleSheet( R"(QScrollBar:vertical {
					   border: none;
					   background: white;
					   width: 10px;
					  }
					  QScrollBar::handle:vertical{
					   background: #4e72b8;
					  }
					  QScrollBar::add-line:vertical{
					   width: 10px;
					   height: 0px;
					  }

					  QScrollBar::sub-line:vertical{
					   width: 10px;
					   height: 0px;
					  })");
}

void AriaListWidget::addTaskSlt(uint64_t aid, QString name)
{
	auto listmodel = static_cast<AriaDownloadListModel*>(model());
	if(listmodel->_taskInfos.find(aid) != listmodel->_taskInfos.end())
		return;

	int count = listmodel->rowCount();
	listmodel->beginInsertRows(QModelIndex(), count, count);
	listmodel->_tasks.push_back(aid);
	TaskInfo tskInfo; tskInfo.name = name;
	tskInfo.state = aria2::DOWNLOAD_WAITING;
	listmodel->_taskInfos.insert(aid, tskInfo);
	listmodel->endInsertRows();
}

void AriaListWidget::updateTaskSlt(uint64_t aid, TaskInfo tskInfo)
{
	auto listmodel = static_cast<AriaDownloadListModel*>(model());
	if(listmodel->_taskInfos.find(aid) == listmodel->_taskInfos.end())
		return;
	listmodel->_taskInfos[aid] = tskInfo;
	auto idx = listmodel->createIndex(listmodel->_tasks.indexOf(aid), 0);
	listmodel->dataChanged(idx, idx);
}

void AriaListWidget::removeTaskSlt(uint64_t aid)
{
	if(_type == DOWNLOADING)
	{
		auto listmodel = static_cast<AriaDownloadListModel*>(model());
		if(listmodel->_taskInfos.find(aid) == listmodel->_taskInfos.end())
			return;
		int idx = listmodel->_tasks.indexOf(aid);
		listmodel->beginRemoveRows(QModelIndex(), idx, idx);
		listmodel->_tasks.removeAt(idx);
		listmodel->_taskInfos.remove(aid);
		listmodel->endRemoveRows();
	}
	else if(_type ==COMPLETED || _type == TRASHCAN)
	{
		auto listmodel = static_cast<AriaFinishListModel*>(model());

		int count = listmodel->rowCount();
		listmodel->beginInsertRows(QModelIndex(), count, count);
		for(int i = 0; i < count; i++)
		{
			if(listmodel->_taskInfos[i].id == aid)
			{
				listmodel->_taskInfos.remove(i);
				break;
			}
		}
		listmodel->endInsertRows();
	}
}

void AriaListWidget::failTaskSlt(uint64_t aid)
{
	setTaskState(aid, aria2::DOWNLOAD_ERROR);
}

void AriaListWidget::startTaskSlt(uint64_t aid)
{
	setTaskState(aid, aria2::DOWNLOAD_ACTIVE);
}

void AriaListWidget::pauseTaskSlt(uint64_t aid)
{
	setTaskState(aid, aria2::DOWNLOAD_PAUSED);
}

void AriaListWidget::addFinishTaskSlt(uint64_t, FinishTaskInfo &taskInfo)
{
	auto listmodel = static_cast<AriaFinishListModel*>(model());

	int count = listmodel->rowCount();
	listmodel->beginInsertRows(QModelIndex(), count, count);
	listmodel->_taskInfos.push_back(taskInfo);
	listmodel->endInsertRows();
}

void AriaListWidget::setTaskState(uint64_t aid, int state)
{
	auto listmodel = static_cast<AriaDownloadListModel*>(model());
	if(listmodel->_taskInfos.find(aid) == listmodel->_taskInfos.end())
		return;
	listmodel->_taskInfos[aid].state = state;
	auto idx = listmodel->createIndex(listmodel->_tasks.indexOf(aid), 0);
	listmodel->dataChanged(idx, idx);
}

QVector<uint64_t>
AriaListWidget::getSelected()
{
	QVector<uint64_t> ids;
	auto idxs = selectedIndexes();
	auto listmodel = static_cast<AriaDownloadListModel*>(model());
	for(auto &idx : idxs){
		auto id = listmodel->_tasks[idx.row()];
		ids.push_back(id);
	}
	return ids;
}

void AriaListWidget::explorerSelected()
{
	auto idxs = selectedIndexes();
	for(auto &idx : idxs)
	{
		explorerIndexAt(idx.row());
	}
}

void AriaListWidget::explorerIndexAt(int i)
{
	auto &taskInfo = ((AriaFinishListModel*)model())->_taskInfos[i];
	QFileInfo fileInfo(taskInfo.localPath);
	QString dir = fileInfo.absoluteDir().absolutePath();
	QDesktopServices::openUrl(QUrl(dir));
}

void AriaListWidget::resizeEvent(QResizeEvent *ev)
{
	auto delegate = (AriaListDelegate*)itemDelegate();
	delegate->setSize(ev->size());
	Base::resizeEvent(ev);
}

void AriaListWidget::mouseMoveEvent(QMouseEvent *ev)
{
	update();
	Base::mouseMoveEvent(ev);
}

void AriaListWidget::mousePressEvent(QMouseEvent *ev)
{
	Base::mousePressEvent(ev);

	auto pt = ev->pos();
	auto dngate = dynamic_cast<AriaListDelegate*>(itemDelegate());
	int i = pt.y() / dnheight;
	int y = pt.y() % dnheight;
	if(_type == DOWNLOADING) {
		if(!dngate->_btnRect.contains(pt.x(), y))
			return;
		auto gid = ((AriaDownloadListModel*)model())->_tasks[i];
		changeTaskState(gid);
	}
	else if(_type == COMPLETED) {
		if(!dngate->_btnRect.contains(pt.x(), y))
			return;
		explorerIndexAt(i);
	}
	else{
		if(!dngate->_btnRect.contains(pt.x(), y))
			return;
	}
}

void AriaListWidget::showEvent(QShowEvent *ev)
{
	Base::showEvent(ev);
	if(model()->rowCount())
		return;

	if(_type == COMPLETED) {
		AriaDlg::getMainDlg()->getDatabase()->initCompleteTask();
	}
	else if(_type == TRASHCAN) {
		AriaDlg::getMainDlg()->getDatabase()->initTrashTask();
	}
}

void AriaListWidget::changeTaskState(uint64_t id)
{
	auto &taskInfo = ((AriaDownloadListModel*)model())->_taskInfos[id];
	if(taskInfo.state == aria2::DOWNLOAD_PAUSED)  {
		AriaDlg::getMainDlg()->startSelected();
	}
	else if(taskInfo.state == aria2::DOWNLOAD_ACTIVE) {
		AriaDlg::getMainDlg()->pauseSelected();
	}else if(taskInfo.state == aria2::DOWNLOAD_ERROR) {
		printf("1111");
	}
}

//--------------------------------------------------------------------------------

QVariant AriaDownloadListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();
	return _taskInfos[_tasks[index.row()]].name;
}

int AriaDownloadListModel::rowCount(const QModelIndex &parent) const
{
	return _tasks.size();
}

Qt::ItemFlags AriaDownloadListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	return QAbstractItemModel::flags(index);
}

//--------------------------------------------------------------------------------

QVariant AriaFinishListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();
	return "";
}

int AriaFinishListModel::rowCount(const QModelIndex &parent) const
{
	return _taskInfos.size();
}

Qt::ItemFlags AriaFinishListModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	return QAbstractItemModel::flags(index);
}
