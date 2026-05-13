#pragma once

#include <QWidget>
#include "ui_filelistwin.h"

class FileListWin : public QWidget
{
	Q_OBJECT

public:
	FileListWin(QWidget *parent = Q_NULLPTR);
	~FileListWin();

	void addItems(const QStringList& labels);

private:
	Ui::FileListWin ui;
};
