#ifndef URILINK_H
#define URILINK_H

#include "frameless.h"

class QPlainTextEdit;

class URILinkWgt : public FramelessDlg{
public:
	URILinkWgt(const QString &url, const QString &name);

	std::vector<std::string> getUris();

	void downloadSlt();
private:
	QPlainTextEdit *_edit;

	QPushButton *_dnBtn;
};

#endif // URILINK_H
