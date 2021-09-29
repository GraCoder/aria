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
#include <QObject>
#include <QString>

class AriaDlg;

class AriaHttpServer : public QObject
{
	Q_OBJECT
public:
	AriaHttpServer(AriaDlg *dlg);

	~AriaHttpServer();

	void run();

signals:
	void addUriTaskSig(QString, QString, QString);
private:
	std::optional<std::string> handler(const http::server::request &req);
private:
	std::unique_ptr<http::server::server> _server;

	AriaDlg		*_dlg;
};

#endif // ARIAHTTPSERVER_H
