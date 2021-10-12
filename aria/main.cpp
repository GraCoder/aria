#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort
#include <iostream>
#include <fstream>

#include <QSettings>

#include <QApplication>
#include "singleapplication.h"
#include "ariaUi.h"
#include "ariaHttpServer.h"
#include "json.hpp"

using namespace aria2;

void browserIntersect()
{
	using namespace nlohmann;
	json chrome = R"({
		"name": "com.t.ariachromewrapper",
		"type": "stdio",
		"browser_action":{},
		"permissions":["http://127.0.0.1/*"],
		"description": "Integrate aria with vivaldi using WebExtensions",
		"allowed_origins": ["chrome-extension://dgblchojffkdlbgmgajnhkjpbdegghkp/"]
	})"_json;
	chrome["path"] = QCoreApplication::applicationDirPath().toStdString() + "\\xx.exe";
	auto contents = chrome.dump();
	auto configpath = QCoreApplication::applicationDirPath().toStdString() + "\\com.t.ariachromewrapper.json";
	std::ofstream(configpath) << contents;

	QSettings setting("HKEY_CURRENT_USER\\SOFTWARE\\Google\\Chrome\\NativeMessagingHosts\\com.t.ariachromewrapper", QSettings::NativeFormat);
	setting.setValue(".", QString::fromStdString(configpath));
	setting.sync();
}


int main(int argc, char**argv)
{
	SingleApplication app(argc, argv);

	auto ft = qApp->font();
	AriaDlg dlg;

	AriaHttpServer server(&dlg);
	std::thread httpThread(
		std::bind(&AriaHttpServer::run, &server));
	browserIntersect();

	dlg.show();

	return app.exec();
}
