#include "ariaSetting.h"

#include <QApplication>
#include <QDir>

ariaSetting::ariaSetting()
{
	_appPath = QCoreApplication::applicationDirPath().toStdString();

	_settings["user-agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) \
			AppleWebKit/537.36 (KHTML, like Gecko) Chrome/93.0.4577.82 Safari/537.36 Edg/93.0.961.52";
			_settings["dir"] = "d:/thunder";
	_settings["disable-ipv6"] = "true";
	_settings["check-certificate"] = "false";
	_settings["disk-cache"] = "64M";
	_settings["no-file-allocation-limit"] = "64M";
	_settings["continue"] = "true";
	_settings["remote-time"] = "true";

	{
		auto path = _appPath;
		path += "/aria.session";
		QFile file(path.c_str());
		if(!file.exists())
		{
			file.open(QIODevice::WriteOnly);
			file.close();
		}
		_settings["input-file"]= path;
		_settings["save-session"]= path;
	}

	_settings["save-session-interval"]= "1";
	_settings["auto-save-interval"]= "20";

	_settings["max-concurrent-downloads"]= "5";
	_settings["max-file-not-found"]= "10";
	_settings["max-tries"]= "5";
	_settings["retry-wait"]= "10";
	_settings["connect-timeout"]= "10";
	_settings["timeout"]= "10";
	_settings["split"]= "64";
	_settings["min-split-size"]= "4M";
	_settings["piece-length"]= "1M";
	_settings["allow-piece-length-change"]= "true";
	_settings["max-overall-download-limit"] = "1248K";
	//		_settings["max-download-limit"] = "10k";
	_settings["optimize-concurrent-downloads"] = "false";
	//		_settings[""]= "";
	//		_settings[""]= "";
	//		_settings[""]= "";
	//		_settings[""]= "";
	//		_settings[""]= "";
}

ariaSetting &ariaSetting::instance()
{
	static ariaSetting set;
	return set;
}

std::string ariaSetting::appPath()
{
	return _appPath;
}

std::string ariaSetting::downloadPath()
{
	return _settings["dir"];
}
