#include "taskDetailWgt.h"

#include <QStyleOption>
#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QTimer>
#include <QRadioButton>
#include <QStackedWidget>
#include <QButtonGroup>

#include "ariaUi.h"

class PieceWgt : public QWidget{
public:
	PieceWgt()
	{
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	}

	void paintEvent(QPaintEvent *ev)
	{
		const int hspace = 5, vspace = 3;
		auto sz = size();

		QRect rt(50, 30, 20, 10);

		QPainter painter(this);

		int col = (sz.width() - 60) / (rt.width() + hspace);

		for(int i = 0; i < _total; i++)
		{
			if(i % col == 0)
				rt.moveTopLeft(QPoint(50, (i / col) * (rt.height() + vspace) + 30));
			if(_comSet.find(i) != _comSet.end())
				painter.fillRect(rt, QColor(0, 240, 33));
			else
				painter.fillRect(rt, Qt::gray);
			rt.translate(hspace + rt.width(), 0);
		}
	}

	void setPieces(int total, const std::set<int> &set)
	{
		_total = total;
		_comSet = set;
		update();
	}

private:
	int _total;
	std::set<int> _comSet;
};


TaskDetailWgt::TaskDetailWgt(QWidget *par)
	: Base(par)
{
	setAttribute(Qt::WA_TranslucentBackground, false);

	setStyleSheet("TaskDetailWidget{border: 1px solid #D1DFFF;}");

	auto timer = new QTimer;
	timer->setInterval(5000);
	connect(timer, &QTimer::timeout, this, &TaskDetailWgt::updateTaskInfo);
	m_timer = timer;

	auto layout = new QHBoxLayout(this);
	auto stackwgt = new QStackedWidget;
	{
		auto grp = new QButtonGroup(this);
		grp->setExclusive(true);

		auto lay = new QVBoxLayout;
		QRadioButton *btn = nullptr;
		btn = new QRadioButton;
		lay->addWidget(btn); btn->setChecked(true);
		grp->addButton(btn, 0);
		btn = new QRadioButton;
		lay->addWidget(btn);
		grp->addButton(btn, 1);
		btn = new QRadioButton;
		lay->addWidget(btn);
		grp->addButton(btn, 2);
		layout->addLayout(lay);

		connect(grp, &QButtonGroup::idClicked, stackwgt, &QStackedWidget::setCurrentIndex);
	}

	layout->addWidget(stackwgt);

	{
		auto wgt = new QWidget;
		auto layout = new QVBoxLayout(wgt);

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

			layout->addLayout(lay);
		}
		_picWgt = new PieceWgt;
		layout->addWidget(_picWgt);
		stackwgt->addWidget(wgt);
	}
	{
		auto wgt = new QWidget;
		stackwgt->addWidget(wgt);
	}
	{
		auto wgt = new QWidget;
		stackwgt->addWidget(wgt);
	}
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
	Base::showEvent(ev);

	m_timer->start();
}

void TaskDetailWgt::hideEvent(QHideEvent *ev)
{
	Base::hideEvent(ev);

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

	_picWgt->setPieces(_tskInfo.picNums, set);
}
