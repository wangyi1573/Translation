#pragma once

#include <QWidget>
#include "ui_timestamptrans.h"

class TimeStampTrans : public QWidget
{
	Q_OBJECT

public:
	TimeStampTrans(QWidget *parent, QWidget* pNotepad);
	~TimeStampTrans();

signals:
	void s_msg(QString msg);

private slots:
	void on_singleToTime();
	void on_singleToTimeStamp();
	void on_batchToTime();
	void on_batchToTimeStamp();
	void on_highTimeStamp();
	void on_replaceTimeStampToTime();
	void on_dealSelectTextToTime();

private:
	Ui::TimeStampTrans ui;
	QWidget* m_pNotepad;
};
