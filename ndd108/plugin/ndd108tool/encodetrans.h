#pragma once

#include <QWidget>
#include "ui_encodetrans.h"

class EncodeTrans : public QWidget
{
	Q_OBJECT

public:
	EncodeTrans(QWidget *parent, QWidget* pNotepad);
	~EncodeTrans();

signals:
	void s_msg(QString msg);

private slots:
	void on_asciiToUnicode();
	void on_unicodeToAscii();

	void on_textToUtf8();
	void on_utf8ToText();
	void on_clear();
	void on_swap();
	void on_copyClip();

	void on_selectUnicodeToText();

private:
	void mulitUnicodeLineToText();

private:
	Ui::EncodeTransClass ui;
	QWidget* m_pNotepad;
};
