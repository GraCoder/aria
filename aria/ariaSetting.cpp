#include "ariaSetting.h"

#include <QApplication>
#include <QDir>

#include "ariaUi.h"
#include "taskDatabase.h"

ariaSetting::ariaSetting()
{
	_appPath = QCoreApplication::applicationDirPath().toStdString();
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

std::map<std::string, std::string> &
ariaSetting::setting()
{
	if(_settings.empty()) {
		_settings["user-agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
				"AppleWebKit/537.36 (KHTML, like Gecko) Chrome/93.0.4577.82 Safari/537.36 Edg/93.0.961.52";
		_settings["enable-uictl"] = "true";
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
			//_settings["input-file"]= path;
			_settings["save-session"]= path;
		}
		_settings["log"] = "log.txt";

		_settings["save-session-interval"]= "5"; //session

		_settings["auto-save-interval"]= "10"; //control

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
#ifdef NDEBUG
		_settings["max-overall-download-limit"] = "4096K";
#else
		_settings["max-overall-download-limit"] = "12K";
#endif
		//_settings["max-download-limit"] = "10k";
		_settings["optimize-concurrent-downloads"] = "false";

		//-bt--------------------------------------------------------
		_settings["listen-port"]= "51413";
		_settings["dht-listen-port"]= "6881-6999";
		_settings["enable-dht"]= "true";
		_settings["enable_dht6"]= "false";
		_settings["dht-file-path"]= _appPath + "/dht.dat";
		_settings["dht-entry-point"]= "dht.transmissionbt.com:6881";
		_settings["enable-peer-exchange"]= "true";
		_settings["bt-max-peers"]= "60";
		_settings["bt-request-peer-speed-limit"]= "5M";
		_settings["max-overall-upload-limit"]= "2M";
		_settings["max-upload-limit"]= "0";
		_settings["seed-ratio"]= "2.0";

		auto sets = AriaDlg::getMainDlg()->getDatabase()->getSettings();
		for(auto &set : sets) {
			_settings[set.first] = set.second;
		}
	}

	return _settings;
}
