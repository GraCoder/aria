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
#include <QProcess>

#include <filesystem>

#include "ariaUi.h"
#include "taskDatabase.h"
#include "taskDetailWgt.h"

const int dnheight = 80;
const int hfheight = dnheight / 5.0 * 2;
const int iconSize = 24;
const int fontSize = 14;

const QString iconPrefix = ":/aria/icons/filetype2";

AriaListDelegate::AriaListDelegate()
{
	_pixMap[enTaskType_Bt] = QPixmap(iconPrefix + "/bt.png");
}

void AriaListDelegate::setSize(const QSize &size)
{
	_btnRect = QRect(size.width() - dnheight, 10, iconSize, iconSize);
}

QSize AriaListDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	return QSize(100, dnheight);
}

QPixmap &AriaListDelegate::getPixmap(int iconType) const
{
	auto iter = _pixMap.find(iconType);
	if(iter != _pixMap.end())
		return *iter;

	auto suf = QString::fromUtf8(TaskUpdateInfo::intToSurfix(iconType).c_str());
	QString filepath = iconPrefix + "/" + suf + ".png";
	auto &px = _pixMap[iconType];
	if(QFile(filepath).exists())
		px = QPixmap(filepath);
	else
		px = QPixmap(iconPrefix + "/file.png");
	return px;
}

//--------------------------------------------------------------------------------

DownloadDelegate::DownloadDelegate()
{

}

void DownloadDelegate::setSize(const QSize &size)
{
	_btnRect = QRect(size.width() - dnheight, 10, iconSize, iconSize);

	auto ft = QFont(); ft.setPixelSize(fontSize);
	int w = QFontMetrics(ft).boundingRect(tr("task info")).width() + 10;
	_tskRect = QRect(size.width() / 5.0 * 3, dnheight / 5.0 * 3, w, hfheight);
}

void DownloadDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
	auto wgt = static_cast<const AriaListWidget *>(opt.widget);
	auto listmodel = static_cast<AriaDownloadListModel*>(wgt->model());
	auto &info = listmodel->_taskInfos[listmodel->_tasks[index.row()]];

	if(opt.state & QStyle::State_Selected || opt.state & QStyle::State_MouseOver)
		painter->fillRect(opt.rect, QColor(227, 230, 228));
	painter->setRenderHint(QPainter::Antialiasing);

	const int popSize = hfheight;

	const QPixmap& ico = getPixmap(info.iconType);
	painter->drawPixmap(QRect(dnheight / 5, dnheight / 5 + opt.rect.top(), dnheight - popSize , dnheight - popSize), ico);

	auto ft = opt.font;
	ft.setBold(true);
	painter->setFont(ft);

	QTextOption texOpt;
	texOpt.setAlignment(Qt::AlignLeft | Qt::AlignBottom);
	texOpt.setWrapMode(QTextOption::NoWrap);
	painter->setPen(QColor(25, 25, 25));
	QRect texRect(dnheight, opt.rect.top(), opt.rect.width() / 3.0 * 2, popSize);
	painter->drawText(texRect, info.name, texOpt);

	auto pt = QCursor::pos();
	pt = wgt->mapFromGlobal(pt);

	//RT---------------------------ICON-----------------------------------------------RT
	{
		QRect pixRect = _btnRect.translated(0, opt.rect.top());
		bool hover = pixRect.contains(pt);
		QString iconUrl = ":/aria/icons/";
		if(info.state == aria2::DOWNLOAD_ACTIVE)
			iconUrl += hover ? "pause-24blue.svg" : "pause-24black.svg";
		else if(info.state == aria2::DOWNLOAD_ERROR || info.state == aria2::DOWNLOAD_PAUSED)
			iconUrl += hover ? "download-24blue.svg" : "download-24black.svg";
		else if(info.state == aria2::DOWNLOAD_WAITING || info.state == aria2::DOWNLOAD_REMOVING || info.state == aria2::DOWNLOAD_PAUSING)
		{
			iconUrl += "waiting.svg";
		}
		QPixmap icon = QPixmap(iconUrl);
		painter->drawPixmap(pixRect, icon, icon.rect());
	}

	{
		texOpt.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
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
			float secs = (info.totalLength - info.dnloadLength) / (info.dnspeed + 0.0001);
			texRect.translate(opt.rect.width() / 5, 0);
			QString tmTex;;
			if(secs > 1e6)
				tmTex = "more than 2 days";
			else {
				auto tm = QTime(0, 0).addSecs(secs);
				tmTex = opt.locale.toString(tm);
			}
			painter->drawText(texRect, tr("remain:") + tmTex, texOpt);
		}
	}
	{
		texRect = QRect(opt.rect.width() / 2.0, dnheight - popSize + opt.rect.top(), opt.rect.width() / 2.0 - 40, hfheight);
		texOpt.setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		if(info.state == aria2::DOWNLOAD_ACTIVE) {
			auto dnspeed = opt.locale.formattedDataSize(info.dnspeed, 2, opt.locale.DataSizeTraditionalFormat);
			painter->drawText(texRect, dnspeed + "/s", texOpt);
		}else if(info.state == aria2::DOWNLOAD_PAUSED)
			painter->drawText(texRect, tr("paused"), texOpt);
		else if(info.state == aria2::DOWNLOAD_WAITING)
			painter->drawText(texRect, tr("waiting"), texOpt);
	}
	{
		texOpt.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		QFont ft = painter->font();
		ft.setItalic(true); painter->setFont(ft);
		auto rect = _tskRect.translated(0, opt.rect.top());
		if(rect.contains(pt))
			painter->setPen(QColor(8, 138, 203));
		painter->drawText(rect, tr("task info"), texOpt);
		//painter->drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
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
	auto wgt = static_cast<const AriaListWidget *>(opt.widget);
	auto listmodel = static_cast<AriaFinishListModel*>(wgt->model());
	auto &info = listmodel->_taskInfos[index.row()];

	if(opt.state & QStyle::State_Selected || opt.state & QStyle::State_MouseOver)
		painter->fillRect(opt.rect, QColor(227, 230, 228));
	painter->setRenderHint(QPainter::Antialiasing);

	const int popSize = dnheight / 5.0 * 2;

	const QPixmap& ico = getPixmap(info.iconType);
	painter->drawPixmap(QRect(dnheight / 5, dnheight / 5 + opt.rect.top(), dnheight - popSize , dnheight - popSize), ico);

	auto ft = opt.font;
	ft.setBold(true);
	painter->setFont(ft);

	QTextOption texOpt;
	texOpt.setAlignment(Qt::AlignLeft | Qt::AlignBottom);
	texOpt.setWrapMode(QTextOption::NoWrap);
	painter->setPen(QColor(25, 25, 25));
	QRect texRect(dnheight, opt.rect.top(), opt.rect.width() / 3.0 * 2, popSize);
	painter->drawText(texRect, info.name, texOpt);

	auto pt = QCursor::pos();
	pt = wgt->mapFromGlobal(pt);

	{
		texRect.translate(0, hfheight);
		auto filesize = opt.locale.formattedDataSize(info.size, 2, opt.locale.DataSizeTraditionalFormat);
		painter->drawText(texRect, filesize, texOpt);
	}

	{
		texRect.translate(opt.rect.width() / 5.0, 0);
		painter->drawText(texRect, info.datetime, texOpt);
	}

	{
		bool hover = false;
		QRect pixRect = _btnRect.translated(0, opt.rect.top());
		if(pixRect.contains(pt))
			hover = true;
		QString iconUrl = ":/aria/icons/";
		if(wgt->type() == COMPLETED )
		{
			hover ? iconUrl += "folder-24blue.svg" : iconUrl += "folder-24black.svg";
		}
		else if(wgt->type() == TRASHCAN)
		{
			hover ? iconUrl += "reload-24blue.svg" : iconUrl += "reload-24black.svg";
		}
		QPixmap icon(iconUrl);
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

	_taskDetailWgt = nullptr;
}

void AriaListWidget::addTaskSlt(uint64_t aid, Task *tsk)
{
	auto listmodel = static_cast<AriaDownloadListModel*>(model());
	if(listmodel->_taskInfos.find(aid) != listmodel->_taskInfos.end())
		return;

	int count = listmodel->rowCount();
	listmodel->beginInsertRows(QModelIndex(), count, count);
	listmodel->_tasks.push_back(aid);
	TaskInfoEx tskInfo;
	tskInfo.id = tsk->id;
	tskInfo.type = tsk->type;
	tskInfo.name = QString::fromUtf8(tsk->name.c_str());
	tskInfo.state = tsk->state;
	tskInfo.totalLength = tsk->to_size;
	tskInfo.dnloadLength = tsk->dn_size;
	tskInfo.uploadLength = tsk->up_size;
	setTaskIcon(&tskInfo);

	listmodel->_taskInfos.insert(aid, tskInfo);
	listmodel->endInsertRows();
}

void AriaListWidget::updateTaskSlt(uint64_t aid, TaskUpdateInfo tskInfo)
{
	auto listmodel = static_cast<AriaDownloadListModel*>(model());
	if(listmodel->_taskInfos.find(aid) == listmodel->_taskInfos.end())
		return;
	if(listmodel->_taskInfos[aid].state == aria2::DOWNLOAD_REMOVING)
		return;
	auto &taskInfo = listmodel->_taskInfos[aid];
	taskInfo = tskInfo;
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
		listmodel->beginRemoveRows(QModelIndex(), count, count);
		for(int i = 0; i < count; i++)
		{
			if(listmodel->_taskInfos[i].id == aid)
			{
				listmodel->_taskInfos.remove(i);
				break;
			}
		}
		listmodel->endRemoveRows();
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

	setTaskIcon(&taskInfo);
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
	if(_type == DOWNLOADING){
		auto idxs = selectedIndexes();
		auto listmodel = static_cast<AriaDownloadListModel*>(model());
		for(auto &idx : idxs){
			auto id = listmodel->_tasks[idx.row()];
			if(listmodel->_taskInfos[id].state == aria2::DOWNLOAD_REMOVING)
				continue;
			ids.push_back(id);
		}
	}
	else{
		auto idxs = selectedIndexes();
		auto listmodel = static_cast<AriaFinishListModel*>(model());
		for(auto &idx : idxs){
			auto id = listmodel->_taskInfos[idx.row()].id;
			ids.push_back(id);
		}
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
	auto listmodel = (AriaFinishListModel*)model();
	if(listmodel->_taskInfos.size() <= i)
		return;
	auto &taskInfo = listmodel->_taskInfos[i];
	QFileInfo fileInfo(taskInfo.localPath);
	QString dir = fileInfo.absoluteFilePath();
#ifdef Q_OS_WIN
	const QString explorer = "explorer";
	QStringList param;
	param << QLatin1String("/select,");
	param << QDir::toNativeSeparators(dir);
	QProcess::startDetached(explorer, param);
#else
	QDesktopServices::openUrl(QUrl(dir));
#endif
}

void AriaListWidget::restartTask(int idx)
{
	auto listmodel = (AriaFinishListModel*)model();
	auto id = listmodel->_taskInfos[idx].id;
	removeTaskSlt(id);
	AriaDlg::getMainDlg()->restartTask(id);
}

TaskInfoEx AriaListWidget::getTaskInfo(uint64_t id)
{
	TaskInfoEx tskInfo;
	auto listmodel = static_cast<AriaDownloadListModel*>(model());
	if(listmodel->_taskInfos.find(id) != listmodel->_taskInfos.end())
		tskInfo = listmodel->_taskInfos[id];
	return tskInfo;
}

void AriaListWidget::showTaskDetail(uint64_t id)
{
	if(_taskDetailWgt == nullptr){
		_taskDetailWgt = new TaskDetailWgt(this);
	}
	TaskInfoEx tskInfo = getTaskInfo(id);
	_taskDetailWgt->fillTaskDetail(tskInfo);
	_taskDetailWgt->show();
	_taskDetailWgt->setGeometry(0, height() / 2.0, width(), height() / 2.0);
}

void AriaListWidget::hideTaskDetail()
{
	if(_taskDetailWgt)
		_taskDetailWgt->hide();
}

QStyleOptionViewItem
AriaListWidget::viewOptions() const
{
	auto viewOpt = Base::viewOptions();
	viewOpt.font.setPixelSize(fontSize);

	return viewOpt;
}

void AriaListWidget::resizeEvent(QResizeEvent *ev)
{
	auto delegate = (AriaListDelegate*)itemDelegate();
	delegate->setSize(ev->size());
	Base::resizeEvent(ev);

	if(_taskDetailWgt && _taskDetailWgt->isVisible())
		_taskDetailWgt->setGeometry(0, height() / 2.0, width(), height() / 2.0);
}

void AriaListWidget::mouseMoveEvent(QMouseEvent *ev)
{
	Base::mouseMoveEvent(ev);
	auto pt = ev->pos();
	int i = pt.y() / dnheight;
	int y = pt.y() % dnheight;
	setCursor(Qt::ArrowCursor);
	if(_type == DOWNLOADING){
		auto dngate = (DownloadDelegate*)itemDelegate();
		auto mdl = (AriaDownloadListModel*)model();
		if(i >= mdl->_tasks.size())
			return;
		if(dngate->_tskRect.contains(pt.x(), y))
		{
			setCursor(Qt::PointingHandCursor);
		}
	}
	update();
}

void AriaListWidget::mousePressEvent(QMouseEvent *ev)
{
	Base::mousePressEvent(ev);
	auto pt = ev->pos();

	if(pt.y() > height() / 2.0)
		setCursor(Qt::ArrowCursor);

	int i = pt.y() / dnheight;
	int y = pt.y() % dnheight;
	if(_type == DOWNLOADING) {
		hideTaskDetail();

		auto mdl = (AriaDownloadListModel*)model();
		if(i >= mdl->_tasks.size())
			return;
		auto dngate = static_cast<DownloadDelegate*>(itemDelegate());
		if(dngate->_btnRect.contains(pt.x(), y)) {
			auto gid = mdl->_tasks[i];
			changeTaskState(gid);
		}
		else if(dngate->_tskRect.contains(pt.x(), y)) {
			auto gid = mdl->_tasks[i];
			showTaskDetail(gid);
		}
	}
	else if(_type == COMPLETED) {
		auto dngate = static_cast<AriaListDelegate*>(itemDelegate());
		if(!dngate->_btnRect.contains(pt.x(), y))
			return;
		explorerIndexAt(i);
	}
	else{
		auto dngate = static_cast<AriaListDelegate*>(itemDelegate());
		if(!dngate->_btnRect.contains(pt.x(), y))
			return;
		restartTask(i);
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
		setTaskState(id, aria2::DOWNLOAD_WAITING);
		AriaDlg::getMainDlg()->startTask(id);
	}
	else if(taskInfo.state == aria2::DOWNLOAD_ACTIVE) {
		setTaskState(id, aria2::DOWNLOAD_WAITING);
		AriaDlg::getMainDlg()->pauseTask(id);
	}else if(taskInfo.state == aria2::DOWNLOAD_ERROR) {
		setTaskState(id, aria2::DOWNLOAD_WAITING);
		AriaDlg::getMainDlg()->restartTask(id, false);
	}
}

void AriaListWidget::setTaskIcon(TaskInfo *tsk)
{
	if(tsk->type == enTaskType_Uri) {
		auto suf = QFileInfo(tsk->name).suffix().toStdString();
		tsk->iconType = TaskUpdateInfo::surfixToInt(suf);
	}
	else if(tsk->type == enTaskType_Bt)
		tsk->iconType = enTaskType_Bt;
}

void AriaListWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
	QListView::selectionChanged(selected, deselected);

	emit selectionChange(selected, deselected);
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
