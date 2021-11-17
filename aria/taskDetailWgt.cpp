#include "taskDetailWgt.h"

#include <QStyleOption>
#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QTimer>

#include "ariaUi.h"

TaskDetailWgt::TaskDetailWgt(QWidget *par)
	: QWidget(par)
{
	setAttribute(Qt::WA_TranslucentBackground, false);

	setStyleSheet("QWidget{background-color: #D1DFFF;}");

	auto timer = new QTimer;
	timer->setInterval(5000);
	connect(timer, &QTimer::timeout, this, &TaskDetailWgt::updateTaskInfo);
	m_timer = timer;

	auto layout = new QVBoxLayout(this);

	{
		_name = new QLabel;
		layout->addWidget(_name, 0, Qt::AlignCenter);
		layout->addSpacing(10);
	}
	{
		auto lay = new QHBoxLayout;
		_picSize = new QLabel;
		lay->addStretch();
		lay->addWidget(_picSize);
		lay->addStretch();
		_picNum = new QLabel;
		lay->addWidget(_picNum);
		lay->addStretch();
		_picCom = new QLabel;
		lay->addWidget(_picCom);
		lay->addStretch();

		_connections = new QLabel;
		lay->addWidget(_connections);
		lay->addStretch();
		layout->addLayout(lay);
	}

	layout->addStretch();
}

void TaskDetailWgt::fillTaskDetail(TaskInfoEx &tskInfo)
{
	_tskInfo = tskInfo;
	updateTaskInfo();
}

std::set<int> TaskDetailWgt::getPieces(const std::string &pic)
{
	std::set<int> ret;
	int count = 0;
	unsigned char ch = 0x80;
	unsigned char chtmp;
	for(int i = 0; i < pic.size(); i++)
	{
		for(int j = 0; j < 8; j++)
		{
			chtmp = pic[i] << j;
			if(chtmp & ch)
				ret.insert(count + j);
		}
		count += 8;
	}
	return ret;
}

void TaskDetailWgt::paintEvent(QPaintEvent *ev)
{
	QStyleOption opt;
	opt.init(this);
	QPainter painter(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
	//QWidget::paintEvent(ev);
}

void TaskDetailWgt::showEvent(QShowEvent *ev)
{
	QWidget::showEvent(ev);

	m_timer->start();
}

void TaskDetailWgt::hideEvent(QHideEvent *ev)
{
	QWidget::hideEvent(ev);

	m_timer->stop();
}

void TaskDetailWgt::updateTaskInfo()
{
	auto dlg = AriaDlg::getMainDlg();
	dlg->fillTaskDetail(_tskInfo);
	_name->setText("name: <b>" + _tskInfo.name + "</b>");

	auto lo = locale();
	_picSize->setText(tr("piece size: <b>") + lo.formattedDataSize(_tskInfo.picLength) + "</b>");
	_picNum->setText("piece num: <b>" + QString::number(_tskInfo.picNums) + "</b>");

	auto set = getPieces(_tskInfo.picString);

	_picCom->setText("piece completed: <b>" + QString::number(set.size()) + "</b>");

	_connections->setText("connections: <b>" + QString::number(_tskInfo.connections) + "</b>");

}
