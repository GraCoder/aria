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
		QString url, agent;
		for(auto &header : req.headers)
		{
			if(header.name == "uris")
			{
				url = QString::fromUtf8(header.value.c_str());
			}
			else if(header.name == "agent")
			{
				agent = QString::fromUtf8(header.value.c_str());
			}
		}
		if(!url.isEmpty())
			addUriTaskSig(url, agent);
		return "succeed";
	}
	return std::nullopt;
}
