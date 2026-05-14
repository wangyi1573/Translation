#include "maintool.h"
#include "encodetrans.h"
#include "fullhalfangle.h"
#include "filecutunion.h"
#include "timestamptrans.h"
#include "tradsimptrans.h"
#include "volcenginetranslate.h"
#include "../rcglobal.h"
#include "renamewin.h"


#include <QWidget>
#include <QShortcut>


const char* item_id = "fun_id";


enum FUN_ID {

	ENCODE_TRANS = 0, //编码转换
	VOLCENGINE_TRANS,//火山引擎翻译
	TRAD_SIMPLIFIED,//繁简转换
	FULL_HALF_ANGLE,//全角半角转换
	TIME_STAMP,//时间戳转换
	FILE_CUT_MERGE,//文件分割合并
	BATCH_FILE_RENAME,//批量文件重命名
	FUN_ID_ENDS
};

extern std::function<bool(QWidget*, int, void*)> s_pluginCallBack;

MainTool::MainTool(QWidget *parent): QMainWindow(parent)
{
	ui.setupUi(this);

#if defined(Q_OS_MAC)
    Qt::WindowFlags m_flags = windowFlags();
    setWindowFlags(m_flags | Qt::Tool);
#endif

#if defined(uos)
    Qt::WindowFlags m_flags = windowFlags();
    setWindowFlags(m_flags | Qt::Tool);
#endif

    QShortcut* escSc = new QShortcut(this);
    escSc->setKey(QKeySequence(Qt::Key_Escape));
    escSc->setContext(Qt::WindowShortcut);
    connect(escSc, &QShortcut::activated, this, [this]() {
        close();
        });

	m_pNotepad = parent;


	init();
}

MainTool::~MainTool()
{
}

void MainTool::init()
{
	connect(ui.listWidget, &QListWidget::currentRowChanged, this, &MainTool::on_funItemClick);



#if 0
	EncodeTrans* p = new EncodeTrans(ui.stackedWidget,m_pNotepad);
	ui.stackedWidget->addWidget(p);
	ui.stackedWidget->setCurrentIndex(0);
	p->setProperty(item_id, ENCODE_TRANS);
	m_widMap[ENCODE_TRANS] = p;
	connect((EncodeTrans*)p, &EncodeTrans::s_msg, this, &MainTool::on_showMsg);
#endif
}


bool MainTool::removeOldStackWidget(int funId)
{
	QWidget* curFunWid = ui.stackedWidget->currentWidget();
	if (curFunWid == nullptr)
	{
		return true;
	}

	int curFunId = curFunWid->property(item_id).toInt();
	if (curFunId != funId)
	{
		QWidget* p = ui.stackedWidget->widget(0);
		ui.stackedWidget->removeWidget(p);
		p->close();
		return true;
	}

	return false;
}

QWidget* MainTool::findWid(int row)
{
	QMap<int,QWidget*>::iterator it =  m_widMap.find(row);
	if (it == m_widMap.end())
	{
		return nullptr;
	}
	return it.value();
}

//列表项切换处理
void MainTool::on_funItemClick(int row)
{
	int ret = removeOldStackWidget(row);
	if (!ret)
	{
		return;
	}

	QWidget* pWidget = findWid(row);

	bool exist = true;

	if (pWidget == nullptr)
	{
		exist = false;

		switch (row)
		{
			case ENCODE_TRANS:
			{
				pWidget = new EncodeTrans(ui.stackedWidget, m_pNotepad);
				pWidget->setProperty(item_id, ENCODE_TRANS);
				connect((EncodeTrans*)pWidget, &EncodeTrans::s_msg, this, &MainTool::on_showMsg);
			}
			break;

			case VOLCENGINE_TRANS:
			{
				pWidget = new VolcengineTranslate(ui.stackedWidget, m_pNotepad);
				pWidget->setProperty(item_id, VOLCENGINE_TRANS);
			}
			break;

			case FULL_HALF_ANGLE:
			{
				pWidget = new FullHalfAngle(ui.stackedWidget, m_pNotepad);
				pWidget->setProperty(item_id, FULL_HALF_ANGLE);
			}
			break;

			case FILE_CUT_MERGE:
			{
				pWidget = new FileCutUnion(ui.stackedWidget);
				pWidget->setProperty(item_id, FILE_CUT_MERGE);
			}
			break;

			case TIME_STAMP:
			{
				pWidget = new TimeStampTrans(ui.stackedWidget, m_pNotepad);
				pWidget->setProperty(item_id, TIME_STAMP);
				connect((TimeStampTrans*)pWidget, &TimeStampTrans::s_msg, this, &MainTool::on_showMsg);
			}
				break;

			case TRAD_SIMPLIFIED:
			{
				pWidget = new TradSimpTrans(ui.stackedWidget, m_pNotepad);
				pWidget->setProperty(item_id, TRAD_SIMPLIFIED);
				//connect((TimeStampTrans*)pWidget, &TimeStampTrans::s_msg, this, &MainTool::on_showMsg);
			}
				break;

			case BATCH_FILE_RENAME:
			{
				pWidget = new ReNameWin(ui.stackedWidget);
				pWidget->setProperty(item_id, BATCH_FILE_RENAME);
			}
				break;
			default:
				break;
		}
	}

	if (pWidget != nullptr)
	{
		pWidget->show();
		ui.stackedWidget->addWidget(pWidget);
		ui.stackedWidget->setCurrentIndex(0);

		if (!exist)
		{
			m_widMap[row] = pWidget;
		}
	}
}

void MainTool::on_showMsg(QString info)
{
	ui.statusBar->showMessage(info, 10000);
}