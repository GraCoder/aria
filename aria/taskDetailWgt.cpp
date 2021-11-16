#include "taskDetailWgt.h"

#include <QStyleOption>
#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>

#include "ariaUi.h"

TaskDetailWgt::TaskDetailWgt(QWidget *par)
	: QWidget(par)
{
	setAttribute(Qt::WA_TranslucentBackground, false);

	setStyleSheet("TaskDetailWgt{border:1px solid #088ACB;}");

	auto layout = new QVBoxLayout(this);

	{
		auto lay = new QHBoxLayout;
		lay->addStretch();
		lay->addWidget(new QLabel(tr("name: ")));
		_name = new QLabel;
		lay->addWidget(_name);
		lay->addStretch();
		_hashlabel = new QLabel(tr("hash: "));
		lay->addWidget(_hashlabel);
		_hash = new QLabel;
		lay->addWidget(_hash);
		lay->addStretch();
		layout->addLayout(lay);
	}
	{
		auto lay = new QHBoxLayout;
		lay->addWidget(new QLabel(tr("piece size: ")));
		_picSize = new QLabel;
		lay->addWidget(_picSize);
		lay->addWidget(new QLabel(tr("piece num: ")));
		_picNum = new QLabel;
		lay->addWidget(_picNum);
		lay->addWidget(new QLabel(tr("piece completed: ")));
		_picCom = new QLabel;
		lay->addWidget(_picCom);

		lay->addWidget(new QLabel(tr("connections: ")));
		_connections = new QLabel;
		lay->addWidget(_connections);
		layout->addLayout(lay);
	}

	layout->addStretch();
}

void TaskDetailWgt::fillTaskDetail(TaskInfoEx &tskInfo)
{
	auto dlg = AriaDlg::getMainDlg();
	dlg->fillTaskDetail(tskInfo);
	_name->setText(tskInfo.name);
	if(tskInfo.type == enTaskType_Uri){
		_hashlabel->hide();
		_hash->hide();
	}
	else{
		_hash->setText(tskInfo.btHash.c_str());
		_hash->show();
		_hashlabel->show();
	}

	auto lo = locale();
	_picSize->setText(lo.formattedDataSize(tskInfo.picLength));
	_picNum->setText(QString::number(tskInfo.picNums));

	auto set = getPieces(tskInfo.picString);

	_picCom->setText(QString::number(set.size()));

	_connections->setText(QString::number(tskInfo.connections));
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
}
