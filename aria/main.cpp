#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <QApplication>

#include "ariaUi.h"
#include <aria2/aria2.h>

using namespace aria2;

int main(int argc, char**argv)
{
	QApplication app(argc, argv);

	libraryInit();

	AriaDlg dlg;

	dlg.exec();

	libraryDeinit();
	
	return 0;
}
