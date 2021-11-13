#ifndef URILINK_H
#define URILINK_H

#include "frameless.h"

#include "taskInfo.h"

#include <QMap>

class QLineEdit;
class QPlainTextEdit;
class QTableWidget;
class QTableWidgetItem;

class URILinkWgt : public FramelessDlg{
public:
	URILinkWgt();
	~URILinkWgt();

	void initUrl(const QString &, const QString &);

	std::vector<std::unique_ptr<Task>> getTasks();

	void downloadSlt();

	QString getFileName(QUrl &url);

	void uriChangedSlt();

	void addBtSlt();

	void downloadDirSlt();

	std::unique_ptr<Task> parseBtFile(const QString &file);
protected:
	void createWidgets();

	void showDownloads();

	void hideDownloads();
private:
	QPlainTextEdit 	*_edit;
	QTableWidget 	*_downList;
	QLineEdit		*_downdir;
	QPushButton *_dnBtn, *_btBtn, *_dnDirBtn;

	std::map<QString, std::unique_ptr<UriTask>> _uriTask;
	std::vector<std::unique_ptr<Task>> _btTasks;
};

#endif // URILINK_H
