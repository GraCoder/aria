#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <iostream>
#include <QApplication>

#include <QSettings>

#include "ariaUi.h"
#include "ariaHttpServer.h"

using namespace aria2;

void browserIntersect()
{
	QCoreApplication::applicationDirPath() + "\\aria.json";

	QSettings setting("HKEY_CURRENT_USER\\SOFTWARE\\Google\\Chrome\\NativeMessagingHosts\\com.persepolis.pdmchromewrapper", QSettings::NativeFormat);
	setting.setValue("", "billy");
}


int main(int argc, char**argv)
{
	AriaHttpServer server;
	server.run();
	QApplication app(argc, argv);
	//QFontDatabase db;
	//auto ret = db.families();
	AriaDlg dlg;
	dlg.exec();
	return 0;
}
