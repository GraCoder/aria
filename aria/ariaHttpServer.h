#ifndef ARIAHTTPSERVER_H
#define ARIAHTTPSERVER_H

namespace http {
namespace server {
class server;
struct request;
}
}

#include <memory>
#include <optional>


class AriaHttpServer
{
public:
	AriaHttpServer();

	~AriaHttpServer();

	void run();

private:
	std::optional<std::string> handler(const http::server::request &req);
private:
	std::unique_ptr<http::server::server> _server;
};

#endif // ARIAHTTPSERVER_H
