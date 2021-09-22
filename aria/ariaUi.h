#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <memory>
#include <thread>

#include <QDialog>


class QListWidget;

namespace aria2{
	struct Session;
}

class AriaDlg : public QDialog{
public:
	AriaDlg();
	~AriaDlg();

	QWidget* createToolBar();

	QWidget* createStatusBar();

	void initAria();

private:
	void download();
private:
	QListWidget *_mainWidget;

	std::thread 	_thread;
	aria2::Session 	*_session;
};
