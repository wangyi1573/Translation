#pragma once

#include <QDialog>
#include <QCloseEvent>
#include "ui_progresswin.h"

class ProgressWin : public QDialog
{
	Q_OBJECT

public:
	ProgressWin(QWidget *parent = Q_NULLPTR);
	virtual ~ProgressWin();

	void info(QString text);
	void setTotalSteps(int step);
	void moveStep(bool isRest = false);

	int getTotalStep();
	void setStep(int step, bool isRest = false);

	bool isCancel();
	void setCancel();
	void setCloseAskProgressStatus(bool isAsk);

protected:
	void closeEvent(QCloseEvent* e) override;

public slots:
	void slot_quitBt();

signals:
	void quitClick();

private:
	Ui::ProgressWin ui;
	int m_curStep;

	bool m_isCancel;
	bool m_closeAskProgress;//关闭进度条时检查进度
};
