#ifndef URILINK_H
#define URILINK_H

#include "frameless.h"

#include "taskInfo.h"

#include <QMap>

class QPlainTextEdit;
class QListWidget;
class QListWidgetItem;

class URILinkWgt : public FramelessDlg{
public:
	URILinkWgt(const QString &url);

	std::vector<std::unique_ptr<UriTask>> getTasks();

	void downloadSlt();

	void uriChangedSlt();

	void addBtSlt();
protected:
	void createWidgets();

private:
	QPlainTextEdit 	*_edit;
	QListWidget 	*_downList;

	QPushButton *_dnBtn, *_btBtn;

	QMap<QUrl, QListWidgetItem*> _items;
};

#endif // URILINK_H
