#pragma once

#include <QWidget>
#include <QFile>
#include "ui_filecutunion.h"

class ProgressWin;

class FileCutUnion : public QWidget
{
	Q_OBJECT

public:
	FileCutUnion(QWidget *parent = nullptr);
	~FileCutUnion();

private:
	void cutFile(QString filePath, QString saveDir);
	void mergeFile(QFile& file, QString filePath, ProgressWin* processWin);

private slots:
	void on_selectSrcFile();
	void on_fileCut();
	void on_fileMerge();

	void on_up();
	void on_down();
	void on_delete();

	void on_clear();
	void on_cutStartAddr(int status);
	void on_outputBlockNum(int status);

private:
	Ui::FileCutUnionClass ui;

	QString m_lastFileDir;

	qint64 m_outputStartAddr;
	int m_outputBlockNum;
};
