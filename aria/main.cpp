#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <QApplication>

#include "ariaUi.h"


using namespace aria2;

int main(int argc, char**argv)
{
	QApplication app(argc, argv);


	AriaDlg dlg;
	dlg.exec();
	
	return 0;
}
