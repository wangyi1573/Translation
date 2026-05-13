#include "instanceobj.h"
#include "maintool.h"


InstanceObj::InstanceObj(QWidget* pNotepad) :QObject(pNotepad)
{
	m_pNotepad = pNotepad;
}

InstanceObj::~InstanceObj()
{

}

void InstanceObj::doMainWork()
{

	//108个功能的主界面，不需要维护插件了，只需要扩展MainTool中的功能就行。
	if (m_pMainToolWin.isNull())
	{
		m_pMainToolWin = new MainTool(m_pNotepad);
		m_pMainToolWin->setAttribute(Qt::WA_DeleteOnClose);
	}
	m_pMainToolWin->show();
}