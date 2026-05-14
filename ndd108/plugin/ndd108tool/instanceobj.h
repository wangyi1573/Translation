#pragma once
#include <QObject>
#include <QWidget>
#include <QPointer>
#include "maintool.h"
class QMenu;

class InstanceObj :public QObject
{
	Q_OBJECT
public:
	//外面Ndd释放时，会自动释放该对象。
	InstanceObj(QWidget* pNotepad);
	~InstanceObj();

public slots:
	void doMainWork();

public:
	QWidget* m_pNotepad;

private:
	InstanceObj(const InstanceObj& other) = delete;
	InstanceObj& operator=(const InstanceObj& other) = delete;
	QPointer<MainTool> m_pMainToolWin;
};