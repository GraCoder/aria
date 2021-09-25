#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <QApplication>

#include "ariaUi.h"

#include <QFontDatabase>

using namespace aria2;

int main(int argc, char**argv)
{
	QApplication app(argc, argv);
	QFontDatabase db;
	auto ret = db.families();
	AriaDlg dlg;
	dlg.exec();
	return 0;
}
