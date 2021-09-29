#include "ariaHttpServer.h"
#include "httpserver/server.h"

#include "json.hpp"
#include "taskInfo.h"
#include "ariaUi.h"


AriaHttpServer::AriaHttpServer(AriaDlg *dlg)
	: _dlg(dlg)
{
	_server	= std::make_unique<http::server::server>("127.0.0.1", "8623", "httpserver");
	_server->reqHandler().set_handle(std::bind(&AriaHttpServer::handler, this, std::placeholders::_1));

	connect(this, &AriaHttpServer::addUriTaskSig, dlg, &AriaDlg::addUri);
}

AriaHttpServer::~AriaHttpServer()
{

}

void AriaHttpServer::run()
{
	_server->run();
}

std::optional<std::string> AriaHttpServer::handler(const http::server::request &req)
{
	if(req.method != "POST")
		return std::nullopt;
	if(req.uri == "/addTask")
	{
		for(auto &header : req.headers)
		{
			if(header.name == "uris")
			{
				auto jss = nlohmann::json::parse(header.value);
				if(jss.count("url_links")){
					auto uriArray = jss["url_links"];
					for(auto iter = uriArray.begin(); iter!= uriArray.end(); iter++)
					{
						QString url, name, cookie;
						if(iter->contains("url"))
							url = QString::fromStdString((*iter)["url"]);
						else
							continue;
						if(iter->contains("filename"))
							name = QString::fromStdString((*iter)["filename"]);
						if(iter->contains("cookies"))
							cookie = QString::fromStdString((*iter)["cookies"]);
						addUriTaskSig(url, name, cookie);
					}
				}
				break;
			}
		}
		return "succeed";
	}
	return std::nullopt;
}
