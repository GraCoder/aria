#include "ariaHttpServer.h"
#include "httpserver/server.h"

#include "json.hpp"


AriaHttpServer::AriaHttpServer()
{
	_server	= std::make_unique<http::server::server>("127.0.0.1", "8623", "httpserver");
	_server->reqHandler().set_handle(std::bind(&AriaHttpServer::handler, this, std::placeholders::_1));
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
						std::string name = (*iter)["name"];
						std::string url = (*iter)["url"];
						std::string cookeie = (*iter)["cookies"];
						printf("");
					}
				}
				break;
			}
		}
		return "succeed";
	}
	return std::nullopt;
}
