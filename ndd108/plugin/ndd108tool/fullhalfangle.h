#pragma once

#include <QWidget>
#include "ui_fullhalfangle.h"

class FullHalfAngle : public QWidget
{
	Q_OBJECT

public:
	FullHalfAngle(QWidget *parent, QWidget* pNotepad);
	~FullHalfAngle();

private slots:
	void on_highFullChar();
	void on_highHalfChar();
	void on_deleteAllFullChar();
	void on_deleteAllHalfChar();
	void on_convertFullToHalf();
	void on_convertHalfToFull();

private:
	void dealChar(int mode);

private:
	Ui::FullHalfAngleClass ui;
	QWidget* m_pNotepad;
};
