#include "filelistwin.h"

FileListWin::FileListWin(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

FileListWin::~FileListWin()
{
}

//以下文件名称，满足正则条件。只有这些文件将被重命名。

void FileListWin::addItems(const QStringList& labels)
{
	ui.fileListWidget->addItems(labels);
}