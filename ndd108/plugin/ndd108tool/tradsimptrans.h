#pragma once

#include <QWidget>
#include <QTreeWidgetItem>
#include <QMenu>
#include "ui_tradsimptrans.h"
#include "opencc/opencc.h"
#include "../rcglobal.h"
#include <Qsci/qsciscintilla.h>

class TradSimpTrans : public QWidget
{
	Q_OBJECT

public:
	TradSimpTrans(QWidget *parent, QWidget* pNotepad);
	~TradSimpTrans();

private slots:
	void on_simpToTrad();
	void on_tradToSimp();
	void slot_selectFile();
	void slot_itemClicked(QTreeWidgetItem* item, int /*column*/);
	void on_batchToTrad();
	void on_batchToSimp();

private:
	bool work(int mode, QsciScintilla* pEdit=nullptr, bool staticTrans = false);
	int allfile(QTreeWidgetItem* root_, QString path_);
	int loadDir(QString rootDirPath);
	void batchTrad(int mode);

private:
	Ui::TradSimpTrans ui;
	QWidget* m_pNotepad;

	QString m_fileDirPath;
	QMenu* m_menu;
	QList<fileAttriNode> m_fileAttris;
	int m_fileNum;

	const opencc::SimpleConverter* m_converter;
	const opencc::SimpleConverter* m_tsconverter;

};
