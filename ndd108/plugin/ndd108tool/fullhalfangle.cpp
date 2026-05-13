#include "fullhalfangle.h"
#include <qsciscintilla.h>
#include <Scintilla.h>

extern std::function<QsciScintilla* (QWidget*)> s_getCurEdit;
extern std::function<bool(QWidget* ,int, void*)> s_pluginCallBack;

FullHalfAngle::FullHalfAngle(QWidget *parent, QWidget* pNotepad)
	: QWidget(parent), m_pNotepad(pNotepad)
{
	ui.setupUi(this);
}

FullHalfAngle::~FullHalfAngle()
{}


//高亮全角字符。
void FullHalfAngle::on_highFullChar()
{
	if (s_pluginCallBack != nullptr)
	{
		QString re("[\\x{ff01}-\\x{ff5e}]|\\x{3000}"); 
		s_pluginCallBack(m_pNotepad, 3,&re);
	}
}

//高亮半角字符。
void FullHalfAngle::on_highHalfChar()
{
	if (s_pluginCallBack != nullptr)
	{
		QString re("[\\x{0020}-\\x{007e}]");
		s_pluginCallBack(m_pNotepad, 3, &re);
	}
}

void FullHalfAngle::dealChar(int mode)
{
	if (s_getCurEdit != nullptr)
	{
		QsciScintilla* pEdit = s_getCurEdit(m_pNotepad);
		if (pEdit == nullptr)
		{
			return;
		}
		QString text = pEdit->text();
		if (text.isEmpty())
		{
			return;
		}

		for (int i = text.length() - 1; i >= 0; --i)
		{
			QChar ch = text.at(i);
			ushort num = ch.unicode();

			if (mode == 0)
			{
				if ((num >= 0xff01 && num <= 0xff5e) || (num == 0x3000))
				{
					text.remove(i, 1);
				}
			}
			else if(mode == 1)
			{
				if (num >= 0x0020 && num <= 0x007e)
				{
					text.remove(i, 1);
				}
			}
			else if (mode == 2)
			{
				//全转半 0xfee0,0x2fe0
				if ((num >= 0xff01 && num <= 0xff5e))
				{
					num -= 0xfee0;
					QChar newchar(num);
					text[i] = newchar;
				}
				else if (num == 0x3000)
				{
					num -= 0x2fe0;
					QChar newchar(num);
					text[i] = newchar;
				}
			}
			else if (mode == 3)
			{
				//半转全 0xfee0,0x2fe0
				if (num >= 0x0021 && num <= 0x007e)
				{
					num += 0xfee0;
					QChar newchar(num);
					text[i] = newchar;
				}
				else if (num == 0x0020)
				{
					num += 0x2fe0;
					QChar newchar(num);
					text[i] = newchar;
				}
			}
		}
		QByteArray newText = text.toUtf8();
		newText.append('\0');
		const char* pText = newText.data();

		pEdit->SendScintilla(SCI_SETTEXT, (uintptr_t)0, (const char*)pText);

	}
}

//删除所有全角字符。
void FullHalfAngle::on_deleteAllFullChar()
{
	dealChar(0);
}

void FullHalfAngle::on_deleteAllHalfChar()
{
	dealChar(1);
}

void FullHalfAngle::on_convertFullToHalf()
{
	dealChar(2);
}

void FullHalfAngle::on_convertHalfToFull()
{
	dealChar(3);
}
