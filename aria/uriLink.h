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
	URILinkWgt(const QString &url);

	std::vector<std::unique_ptr<UriTask>> getTasks();

	void downloadSlt();

	void uriChangedSlt();

	void addBtSlt();

	void downloadDirSlt();
protected:
	void createWidgets();

	void showDownloads();

	void hideDownloads();
private:
	QPlainTextEdit 	*_edit;
	QTableWidget 	*_downList;
	QLineEdit		*_downdir;
	QPushButton *_dnBtn, *_btBtn, *_dnDirBtn;

	QMap<QUrl, QTableWidgetItem*> _items;
};

#endif // URILINK_H
