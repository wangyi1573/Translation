#include "encodetrans.h"
#include <QClipboard>
#include "UniConversion.h"
#include <qsciscintilla.h>
#include <Scintilla.h>

//编码转换的界面。支持https://tool.chinaz.com/上面所有的在线方法。
extern std::function<QsciScintilla* (QWidget*)> s_getCurEdit;
extern std::function<bool(QWidget*, int, void*)> s_pluginCallBack;

EncodeTrans::EncodeTrans(QWidget *parent, QWidget* pNotepad)
	: QWidget(parent), m_pNotepad(pNotepad)
{
	ui.setupUi(this);
}

EncodeTrans::~EncodeTrans()
{}

void EncodeTrans::on_asciiToUnicode()
{
    QString srcText = ui.srcTextEdit->toPlainText();
    QString qs16ShowText(tr("\\u; format:\n"));
    QString qs10ShowText(tr("&# format:\n"));

    for (int i = 0; i < srcText.length(); i++)
    {
        QChar ch = srcText.at(i);
        ushort num = ch.unicode();
        qs16ShowText = qs16ShowText + "\\u" + QString::number(num, 16);
        qs10ShowText = qs10ShowText + "&#" + QString::number(num) + ";";
    }

    qs10ShowText.append("\n");

    ui.dstTextEdit->setPlainText(qs16ShowText);
    ui.dstTextEdit->appendPlainText(qs10ShowText);

}

void EncodeTrans::mulitUnicodeLineToText()
{
    QString srcText = ui.srcTextEdit->toPlainText();
    srcText = srcText.trimmed();

    QString outText;

    QStringList tokens = srcText.split("\\u");

    //如果是以\u直接开头，则第一个元素会是0
    //去掉第一个为空的元素。如果\u直接开头，则第一个可能是空，此时有些小问题。开头会多处一个\u
    if (!tokens.isEmpty() && tokens.first().isEmpty())
    {
        tokens.removeAt(0);
    }
    else if (!srcText.startsWith("\\u"))
    {
        //如果不是以\\u开头，则第一个元素，理论上不是需要转换的unicode，直接取出保留原始字样即可
        outText = tokens.at(0);
        tokens.removeAt(0);
    }
    //以上步骤后，能够保证tokens中都是有效的\u分割字符串了。避免后续出现一些头部多一个，少一个\u的一些列问题。

    bool ok;
    QChar errorChar(3581);

    for (int i = 0; i < tokens.size(); ++i)
    {
        if (tokens.at(i).size() >= 4)
        {
            QString oneChar = tokens.at(i).mid(0, 4);
            if (!oneChar.isEmpty())
            {
                int charNum = oneChar.toInt(&ok, 16);
                if (ok)
                {
                    QChar c(charNum);
                    outText.append(c);
                }
                else
                {
                    //如果转换错误，保留原始的字符拉倒。原始是前面去掉了\u的，补充上。
                    outText.append("\\u" + oneChar);
                }
            }
            else
            {
                //如果转换错误，保留原始的字符拉倒。原始是前面去掉了\u的，补充上。
                outText.append("\\u" + oneChar);
            }
            if (tokens.at(i).size() > 4)
            {
                outText.append(tokens.at(i).mid(4));
            }

        }
        else if (tokens.at(i).size() >= 2)
        {
            //如果是2个，就是纯粹的英文数字。
            QString oneChar = tokens.at(i).mid(0, 2);
            if (!oneChar.isEmpty())
            {
                int charNum = oneChar.toInt(&ok, 16);
                if (ok)
                {
                    QChar c(charNum);
                    outText.append(c);
                }
        else
        {
                    //如果转换错误，保留原始的字符拉倒。原始是前面去掉了\u的，补充上。
                    outText.append("\\u" + oneChar);
                }
            }
            else
            {
                //如果转换错误，保留原始的字符拉倒。原始是前面去掉了\u的，补充上。
                outText.append("\\u" + oneChar);
            }
            if (tokens.at(i).size() > 2)
            {
                outText.append(tokens.at(i).mid(2));
            }
        }
        else
        {
            //不足4个和2个的，保存原样。
            outText.append("\\u" + tokens.at(i));
        }
    }

    ui.dstTextEdit->setPlainText(outText);
}

void EncodeTrans::on_unicodeToAscii()
{
    QString srcText = ui.srcTextEdit->toPlainText();

    //如果是\\u的格式，统一按照多行去解析。而&#的格式，只能单行。暂时保留。其实可以统一到mulitUnicodeLineToText中去
    if (srcText.contains("\\u"))
    {
        return mulitUnicodeLineToText();
    }

    //如果是多行文本，按照多行的方式来解析。
    QString dstText;
    QStringList charList;
    int base = 10;

    if (srcText.startsWith("&#"))
    {
        srcText = srcText.replace("&#", "");
        charList = srcText.split(';');
    }
    else if (srcText.startsWith("\\u"))
    {
        charList = srcText.split("\\u");
        base = 16;
    }
    else
    {
        ui.dstTextEdit->setPlainText(tr("error src text, please check !"));
        return;
    }

    QString oneChar;
    dstText.reserve(charList.size());

    bool ok;
    QChar errorChar(3581);
    for (int i = 0, s = charList.size(); i < s; ++i)
    {
        oneChar = charList.at(i);
        if (!oneChar.isEmpty())
        {
            int charNum = oneChar.toInt(&ok,base);
            if (ok)
            {
                QChar c(charNum);
                dstText.append(c);
            }
            else
            {
                dstText.append(errorChar);
            }
        }
    }
    ui.dstTextEdit->setPlainText(dstText);
}


//这个网址的编码才是对的。http://www.mytju.com/classcode/tools/encode_utf8.asp
void EncodeTrans::on_textToUtf8()
{
    QString srcText = ui.srcTextEdit->toPlainText();
    QString qs16ShowText;

    QByteArray utf8Bytes = srcText.toUtf8();
    uchar* pBytes = (uchar*)utf8Bytes.data();

    int i = 0;
    int len = utf8Bytes.size();
    int ret = 8;
    quint32 utf8Value = 0;
    QChar errorChar(3581);

    while (i < len)
    {
        ret = Scintilla::UTF8Classify(pBytes + i, len - i);
        if (ret >= 1 && ret <= 4)
        {
            utf8Value = 0;

            for (int x = 0; x < ret; ++x)
            {
                utf8Value += ((quint32)*(pBytes + i + x)) << ((ret - x - 1)*8);
            }
            qs16ShowText = qs16ShowText + "\\u" + QString::number(utf8Value, 16);
            i += ret;
        }
        else
        {
            ++i;
            qs16ShowText.append(errorChar);
        }
    }

    ui.dstTextEdit->setPlainText(qs16ShowText);
}

void EncodeTrans::on_utf8ToText()
{
    QString srcText = ui.srcTextEdit->toPlainText();
    QStringList charList = srcText.split("\\u");

    int numValue = 0;
    QByteArray utf8Bytes;
    bool ok;

    for (int i = 0; i < charList.size(); ++i)
    {
        numValue = charList.at(i).toUInt(&ok,16);
        if (numValue == 0)
        {
            continue;
        }

        uchar* p = (uchar*)(&numValue);

        for (int x = 3; x >=0 ; --x)
        {
            if (*(p + x) == 0)
            {
                continue;
            }
            else
            {
                utf8Bytes.append(*(p+x));
            }
        }
    }

    QString utf8Text(utf8Bytes);
    ui.dstTextEdit->setPlainText(utf8Text);
}

void EncodeTrans::on_clear()
{
    ui.srcTextEdit->clear();
    ui.dstTextEdit->clear();
}

void EncodeTrans::on_swap()
{
    QString src = ui.srcTextEdit->toPlainText();
    QString dst = ui.dstTextEdit->toPlainText();

    ui.srcTextEdit->setPlainText(dst);
    ui.dstTextEdit->setPlainText(src);
}

void EncodeTrans::on_copyClip()
{
    QString dst = ui.dstTextEdit->toPlainText();

    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setText(dst);
}

//选中的unicode文本，转换为文本
void EncodeTrans::on_selectUnicodeToText()
{
    if (s_getCurEdit != nullptr)
    {
        QsciScintilla* pEdit = s_getCurEdit(m_pNotepad);
        if (pEdit == nullptr)
        {
            return;
        }
        QString text = pEdit->selectedText();
        if (text.isEmpty())
        {
            emit s_msg(tr("Select Text Is Empty, Please Select Text First !"));
            QApplication::beep();
            return;
        }


        ui.srcTextEdit->clear();
        ui.dstTextEdit->clear();

        ui.srcTextEdit->setPlainText(text);
        on_unicodeToAscii();
    }
}
