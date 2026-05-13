#include "timestamptrans.h"
#include <QDateTime>
#include <QStyle>
#include <QIcon>
#include <qsciscintilla.h>
#include <Scintilla.h>

extern std::function<QsciScintilla* (QWidget*)> s_getCurEdit;
extern std::function<bool(QWidget*, int, void*)> s_pluginCallBack;

//时间戳转换函数
TimeStampTrans::TimeStampTrans(QWidget *parent, QWidget* pNotepad)
	: QWidget(parent),m_pNotepad(pNotepad)
{
	ui.setupUi(this);

	QStyle* style = QApplication::style();
	QIcon icon = style->standardIcon(QStyle::SP_ArrowLeft);
	ui.toTimeStampBt->setIcon(icon);

	icon = style->standardIcon(QStyle::SP_ArrowRight);
	ui.toTimeBt->setIcon(icon);

}

TimeStampTrans::~TimeStampTrans()
{
}

//单个转换，时间戳到时间。
void TimeStampTrans::on_singleToTime()
{
	QString timestamp = ui.timestampLe->text();

	bool ok;

	qint64 timeNum = timestamp.toLongLong(&ok);

	if (ok)
	{
		if (1 == ui.comboBoxTimeStamp->currentIndex())
		{
			timeNum *= 1000;
		}

		QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(timeNum);
		ui.timeLe->setText(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
	}
	else
	{
		ui.timeLe->setText("unknown");
	}
}


//单个转换，时间到时间戳
void TimeStampTrans::on_singleToTimeStamp()
{
	QString stringFormat = ui.srcTimeLe->text().trimmed();

	QRegExp re("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}");

	if (re.exactMatch(stringFormat))
	{
		QDateTime dataTime = QDateTime::fromString(stringFormat,"yyyy-MM-dd hh:mm:ss");

		qint64 value = dataTime.toMSecsSinceEpoch();

		if (1 == ui.comboBoxTime->currentIndex())
		{
			value /= 1000;
		}


		ui.dstTimestampLe->setText(QString::number(value));
	}
}

//批量转时间
void TimeStampTrans::on_batchToTime()
{
	QString timestampText = ui.timeStampTextEdit->toPlainText();
	QStringList timestampList = timestampText.split("\n");
	ui.timeTextEdit->clear();

	bool ok;

	for (int i = 0; i < timestampList.size(); ++i)
	{
		QString timestamp = timestampList.at(i);
		timestamp = timestamp.trimmed();

		qint64 v = timestamp.toLongLong(&ok);
		if (ok)
		{
			QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(v);
			ui.timeTextEdit->appendPlainText(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
		}
		else
		{
			ui.timeTextEdit->appendPlainText("error timestamp");
		}
	}
}

void TimeStampTrans::on_batchToTimeStamp()
{
	QString timeText = ui.timeTextEdit->toPlainText();
	QStringList timeList = timeText.split("\n");
	ui.timeStampTextEdit->clear();

	QRegExp re("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}");

	for (int i = 0; i < timeList.size(); ++i)
	{
		QString time = timeList.at(i);
		time = time.trimmed();

		if (re.exactMatch(time))
		{
			QDateTime dataTime = QDateTime::fromString(time, "yyyy-MM-dd hh:mm:ss");
			qint64 value = dataTime.toMSecsSinceEpoch();
			ui.timeStampTextEdit->appendPlainText(QString::number(value));
		}
		else
		{
			ui.timeStampTextEdit->appendPlainText("error time");
		}
	}
}

//高亮当前时间戳。
void TimeStampTrans::on_highTimeStamp()
{
	//按秒格式。

	if (s_pluginCallBack != nullptr)
	{
		QString re("\\D\\d{11,13}\\D");

		if (1 == ui.timeFormat->currentIndex())
		{
			re = ("\\D\\d{8,10}\\D");
		}

		s_pluginCallBack(m_pNotepad, 3, &re);
	}
}

bool isNum(const QChar& c)
{
	return c >= '0' && c <= "9";
}

void TimeStampTrans::on_replaceTimeStampToTime()
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

		auto determineResult = [](int & offset, QString& result){

			//第一位是不是数字,前移一位，过滤掉非数字。去掉第一位
			if (!isNum(result.at(0)))
			{
				++offset;
				result = result.mid(1);
			}

			//最后一位不是数字，去掉最后一位
			if (!isNum(result.at(result.size() - 1)))
			{
				result = result.mid(0, result.size() - 1);
			}
		};

		QRegularExpression re("\\D\\d{11,13}\\D");

		bool isSec = false;

		if (1 == ui.timeFormat->currentIndex())
		{
			re.setPattern("\\D\\d{8,10}\\D");
			isSec = true;
		}

		QRegularExpressionMatch rmatch;

		int from = -1;
		bool ok;

		int times = 0;

		for (int i = 0; i < text.size(); ++i)
		{
			int offset = text.lastIndexOf(re,from, &rmatch);
			if (offset == -1)
			{
				break;
			}
			QString result = rmatch.captured();

			determineResult(offset,result);

			qint64 v = result.toLongLong(&ok);
			if (ok)
			{
				if (isSec)
				{
					v *= 1000;
				}

				QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(v);
				text.replace(offset, result.size(), dateTime.toString("yyyy-MM-dd hh:mm:ss"));
				++times;

			}

			from = offset;
		}

		QByteArray newText = text.toUtf8();
		newText.append('\0');
		const char* pText = newText.data();

		pEdit->SendScintilla(SCI_SETTEXT, (uintptr_t)0, (const char*)pText);

		emit s_msg(tr("A total of %1 timestamps were converted !").arg(times));
	}
}

//把选中的时间戳文本，转换为时间
void TimeStampTrans::on_dealSelectTextToTime()
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
			return;
		}

		QRegExp re("\\d{11,13}");

		bool isSec = false;

		if (1 == ui.timeFormat->currentIndex())
		{
			re.setPattern("\\d{8,10}");
			isSec = true;
		}

		bool ok;

		if (re.exactMatch(text))
		{
			qint64 v = text.toLongLong(&ok);
			if (ok)
			{
				if (isSec)
				{
					v *= 1000;
				}

				QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(v);
				pEdit->replaceSelectedText(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
			}
		}
		else
		{
			emit s_msg(tr(" '%1' is not a valid timestamp !").arg(text));
		}
	}
}
