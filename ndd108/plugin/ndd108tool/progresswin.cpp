#include "progresswin.h"
#include <QCoreApplication>
#include <QMessageBox>

ProgressWin::ProgressWin(QWidget *parent)
	: QDialog(parent), m_curStep(0),m_isCancel(false), m_closeAskProgress(true)
{
	ui.setupUi(this);
    Qt::WindowFlags flags = Qt::Dialog;
    flags |= Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
}

ProgressWin::~ProgressWin()
{
}


void ProgressWin::info(QString text)
{
	ui.output->append(text);
}

void ProgressWin::setTotalSteps(int step)
{
	ui.progressBar->setValue(0);
	ui.progressBar->setMaximum(step);
	m_curStep = 0;
}

void ProgressWin::moveStep(bool isRest)
{
	++m_curStep;
	ui.progressBar->setValue(m_curStep);
	ui.progressBar->update();

	if (isRest)
	{
		QCoreApplication::processEvents();
	}
}

int ProgressWin::getTotalStep()
{
	return ui.progressBar->maximum();
}

void ProgressWin::setStep(int step, bool isRest)
{
	ui.progressBar->setValue(step);
	ui.progressBar->update();
	m_curStep = step;
	if (isRest)
	{
		QCoreApplication::processEvents();
	}
}

bool ProgressWin::isCancel()
{
	return m_isCancel;
}

void ProgressWin::setCancel()
{
	m_isCancel = true;
	emit quitClick();
}

void ProgressWin::closeEvent(QCloseEvent* e)
{
	if (m_closeAskProgress && (m_curStep < ui.progressBar->maximum()))
	{
		slot_quitBt();

		if (!m_isCancel)
		{
			e->ignore();
		}
	}
}

void ProgressWin::slot_quitBt()
{
	if (QMessageBox::Yes != QMessageBox::question(this, tr("Notice"), tr("Are you sure to cancel?")))
	{
		return;
	}

	m_isCancel = true;
	emit quitClick();
}

//直接关闭进度条时，会检测进度是否完成，是否是正常退出。
//如果设置该标记为false，则不会检查进度
void ProgressWin::setCloseAskProgressStatus(bool isAsk)
{
	m_closeAskProgress = isAsk;
}