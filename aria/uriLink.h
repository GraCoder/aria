#ifndef URILINK_H
#define URILINK_H

#include "frameless.h"

#include "taskInfo.h"

class QPlainTextEdit;

class URILinkWgt : public FramelessDlg{
public:
	URILinkWgt(const QString &url);

	std::vector<std::unique_ptr<UriTask>> getTasks();

	void downloadSlt();

protected:
	void createWidgets();
private:
	QPlainTextEdit *_edit;

	QPushButton *_dnBtn;
};

#endif // URILINK_H
