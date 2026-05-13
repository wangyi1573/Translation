#include "tradsimptrans.h"
#include "opencc/opencc.h"
#include "opencc/SimpleConverter.hpp"
#include "../rcglobal.h"
#include <QCoreApplication>
#include <qsciscintilla.h>
#include <Scintilla.h>
#include <qbytearray.h>
#include <QFile>
#include <QMessageBox>
#include <stdexcept>
#include <QFileDialog>
#include <QProcess>
#include "progresswin.h"

//繁体简体互换插件

//编码转换的界面。支持https://tool.chinaz.com/上面所有的在线方法。
extern std::function<QsciScintilla* (QWidget*)> s_getCurEdit;
extern std::function<bool(QWidget*, int, void*)> s_pluginCallBack;

//为了避免路径中\\不一样导致的查找不到问题，进行统一替换
static QString getRegularFilePath(QString& path)
{
#ifdef _WIN32
	path = path.replace("/", "\\");
#else
	path = path.replace("\\", "/");
#endif

	return path;
}

void showFileInExplorer(QString path)
{
	QString cmd;

#ifdef _WIN32
	path = path.replace("/", "\\");
	cmd = QString("explorer.exe /select,%1").arg(path);
#endif

#ifdef ubu
	path = path.replace("\\", "/");
	cmd = QString("nautilus %1").arg(path);
#endif

#ifdef uos
	path = path.replace("\\", "/");
	cmd = QString("dde-file-manager %1").arg(path);
#endif 

#if defined(Q_OS_MAC)
	path = path.replace("\\", "/");
	cmd = QString("open -R %1").arg(path);
#endif

	QProcess process;
	process.startDetached(cmd);
}

TradSimpTrans::TradSimpTrans(QWidget *parent, QWidget* pNotepad)
	: QWidget(parent), m_pNotepad(pNotepad), m_converter(nullptr), m_tsconverter(nullptr), m_menu(nullptr), m_fileNum(0)
{
	ui.setupUi(this);

    connect(ui.treeWidget, &QTreeWidget::itemPressed, this, &TradSimpTrans::slot_itemClicked);
	connect(ui.newfileCb, &QCheckBox::stateChanged, this, [this](int state) {
			ui.lineEdit->setEnabled(Qt::Checked == state);
		});

	ui.treeWidget->setAlternatingRowColors(true);
}

TradSimpTrans::~TradSimpTrans()
{
    if (m_converter != nullptr)
    {
        delete m_converter;
        m_converter = nullptr;
    }

    if (m_tsconverter != nullptr)
    {
        delete m_tsconverter;
        m_tsconverter = nullptr;
    }
}

//右键菜单
void TradSimpTrans::slot_itemClicked(QTreeWidgetItem* item, int /*column*/)
{
	if ((item != nullptr) && (Qt::RightButton == QGuiApplication::mouseButtons()))
	{

		if (m_menu == nullptr)
		{
			m_menu = new QMenu(this);
			m_menu->addAction(tr("&Show File in Explorer..."), this, [&]() {
				QString path, cmd;

				QTreeWidgetItem* it = ui.treeWidget->currentItem();
				if (it == nullptr)
				{
					return;
				}

				path = QString("%1").arg(it->data(0, Qt::ToolTipRole).toString());
				showFileInExplorer(path);
				});
		}
		m_menu->move(QCursor::pos());
		m_menu->show();
	}
}

bool TradSimpTrans::work(int mode, QsciScintilla* pEdit, bool staticTrans)
{
    QString fileName = QCoreApplication::applicationDirPath();

    QString configFilepath;

    if (mode == 0)
    {
         configFilepath = QString("%1/plugin/%2").arg(fileName).arg("s2t.json");
    }
    else
    {
         configFilepath = QString("%1/plugin/%2").arg(fileName).arg("t2s.json");
    }


    //配置文件不存在
    if (!QFile::exists(configFilepath))
    {
		if (!staticTrans)
		{
			QMessageBox::warning(this, tr("config error"), tr("config %1 not found !").arg(configFilepath));
		}
		return false;
    }

    const opencc::SimpleConverter* convert;

    if (mode == 0)
    {
        if (m_converter == nullptr)
        {
            m_converter = new opencc::SimpleConverter(configFilepath.toStdString());
        }
        convert = m_converter;

    }
    else
    {
        if (m_tsconverter == nullptr)
        {
            m_tsconverter = new opencc::SimpleConverter(configFilepath.toStdString());
        }

        convert = m_tsconverter;
    }

	if (pEdit == nullptr)
	{
		if (s_getCurEdit != nullptr)
		{
			pEdit = s_getCurEdit(m_pNotepad);
			}
		}
    if (pEdit != nullptr)
    {
        QString text = pEdit->text();
        if (text.isEmpty())
        {
            QApplication::beep();
			return false;
        }

        QByteArray u8bytes = text.toUtf8();

        try {
            std::string ret = convert->Convert(u8bytes.data());
            pEdit->SendScintilla(SCI_SETTEXT, (uintptr_t)0, (const char*)(ret.c_str()));
        }
        catch (...) 
        {
			if (!staticTrans)
			{
				QMessageBox::warning(this, tr("config error"), tr("Conversion Configuration file missing !"));
			}
			return false;
        }

    }

	return true;
}
//繁体转简体
void TradSimpTrans::on_tradToSimp()
{
    work(1);
}

//简体转繁体。
void TradSimpTrans::on_simpToTrad()
{
    work(0);
}


//打开文件目录
void TradSimpTrans::slot_selectFile()
{
    //加载左边的文件树
    QString rootpath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), QString());

    if (!rootpath.isEmpty())
    {
        ui.batchDirPath->setText(rootpath);

        ui.treeWidget->clear();

        m_fileAttris.clear();

        m_fileNum = loadDir(rootpath);
    }


}

int TradSimpTrans::allfile(QTreeWidgetItem* root_, QString path_)
{
	QList<WalkFileInfo> dirsList;
	WalkFileInfo oneDir(0, root_, path_);
	dirsList.append(oneDir);

	int fileNums = 0;

	m_fileDirPath = path_;

	while (!dirsList.isEmpty())
	{
		WalkFileInfo curDir = dirsList.first();
		dirsList.pop_front();

		QTreeWidgetItem* root = curDir.root;
		QString path = curDir.path;
		int direction = curDir.direction;

		/*添加path路径文件*/
		QDir dir(path);          //遍历各级子目录

		//先获取文件到列表
		//再获取文件夹到列表
		QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);   //获取当前所有目录

		for (int i = 0; i != folder_list.size(); ++i)         //自动递归添加各目录到上一级目录
		{
			QString namepath = folder_list.at(i).absoluteFilePath();    //获取路径
			QFileInfo folderinfo = folder_list.at(i);
			QString name = folderinfo.fileName();      //获取目录名

			QTreeWidgetItem* childroot = new QTreeWidgetItem(QStringList() << name);
			childroot->setIcon(0, QIcon(":/Resources/img/dir.png"));
			root->addChild(childroot);              //将当前目录添加成path的子项

			fileAttriNode node;
			node.type = RC_DIR;//是目录
			node.selfItem = childroot;

			node.parent = root;
			node.relativePath = folderinfo.absoluteFilePath();
			//把路径名称保存到tips中，后续需要这个来排序，下同
			childroot->setData(0, Qt::ToolTipRole, node.relativePath);

			m_fileAttris.append(node);

			WalkFileInfo oneDir(direction, childroot, namepath);

			dirsList.push_front(oneDir);
		}

		QDir dir_file(path);
		dir_file.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);//获取当前所有文件
		QFileInfoList list_file = dir_file.entryInfoList();
		for (int i = 0; i < list_file.size(); ++i)
		{  //将当前目录中所有文件添加到treewidget中
			QFileInfo fileInfo = list_file.at(i);

			QString name2 = fileInfo.fileName();
			QTreeWidgetItem* child = new QTreeWidgetItem(QStringList() << name2);
			child->setIcon(0, QIcon(":/Resources/img/point.png"));

			child->setText(1, QString::number(fileInfo.size()));

			root->addChild(child);

			fileAttriNode node;
			node.type = RC_FILE;//是文件
			node.selfItem = child;

			node.parent = root;
			node.relativePath = fileInfo.absoluteFilePath();
			//把路径名称保存到tips中，后续需要这个来排序，下同

			child->setData(0, Qt::ToolTipRole, node.relativePath);

			m_fileAttris.append(node);

		}

		fileNums += list_file.size();
	}

	return fileNums;
}


int TradSimpTrans::loadDir(QString rootDirPath)
{

	QString rootpath = rootDirPath;

	QTreeWidgetItem* root = nullptr;

	int fileNums = 0;

	ui.treeWidget->setColumnWidth(0, 400);
	ui.treeWidget->clear();


	root = new QTreeWidgetItem(ui.treeWidget);
	root->setText(0, rootpath);
	root->setExpanded(true);

	//第一个节点是目录根节点
	fileAttriNode node;
	node.type = RC_DIR;//是目录
	node.selfItem = root;
	node.parent = nullptr;
	node.relativePath = ".";

	m_fileAttris.append(node);

	fileNums = allfile(root, rootpath);


	return fileNums;
}

void TradSimpTrans::batchTrad(int mode)
{
	QsciScintilla* pEdit = nullptr;

	if (m_fileAttris.isEmpty())
	{
		return;
	}

	if (s_pluginCallBack != nullptr)
	{
		QVariant v;
		bool ret = s_pluginCallBack(m_pNotepad, 4, &v);
		if (!ret)
		{
			return;
		}

		pEdit = v.value<QsciScintilla*>();

		if (pEdit == nullptr)
		{
			return;
		}
	}

	bool createNewFile = ui.newfileCb->isChecked();
	QString fileSuffix = ui.lineEdit->text();
	if (createNewFile && fileSuffix.isEmpty())
	{
		fileSuffix = "_new";
	}



	ProgressWin* ploadFileProcessWin = new ProgressWin(this);

	ploadFileProcessWin->setWindowModality(Qt::WindowModal);
	ploadFileProcessWin->info(tr("Now batch trans file, please wait ..."));
	ploadFileProcessWin->setTotalSteps(m_fileNum);
	ploadFileProcessWin->show();

	for (QList<fileAttriNode>::iterator it = m_fileAttris.begin(); it != m_fileAttris.end(); ++it)
	{
		if (it->type == RC_FILE)
		{
			pEdit->setProperty("filepath", it->relativePath);
			bool ret = s_pluginCallBack(m_pNotepad, 5, pEdit);

			if (ret && work(mode, pEdit, true))
			{
				//保存文件
				if (createNewFile)
				{
					QString oldPath = it->relativePath;
					getRegularFilePath(oldPath);

					QFileInfo fi(oldPath);
					QString newPath = QString("%1/%2%3.%4").arg(fi.absolutePath()).arg(fi.baseName()).arg(fileSuffix).arg(fi.completeSuffix());
					getRegularFilePath(newPath);
					pEdit->setProperty("filepath", newPath);
				}

				ret = s_pluginCallBack(m_pNotepad, 6, pEdit);
				if (ret)
				{
					it->selfItem->setText(2, "success ");
				}
			}

			if (!ret)
			{
				it->selfItem->setText(2, "failed ");
			}

			ploadFileProcessWin->moveStep();
		}
	}

	ploadFileProcessWin->close();
	delete ploadFileProcessWin;
	ploadFileProcessWin = nullptr;

	if (pEdit != nullptr)
	{
		delete pEdit;
	}
}

//批量转繁体
void TradSimpTrans::on_batchToTrad()
{
	batchTrad(0);
}

//批量转简体
void TradSimpTrans::on_batchToSimp()
{
	batchTrad(1);
}
