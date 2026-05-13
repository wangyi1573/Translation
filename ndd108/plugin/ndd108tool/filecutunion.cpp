#include "filecutunion.h"
#include "progresswin.h"

#include <QStyle>
#include <QIcon>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFileDialog>

//文件切割与合并功能

FileCutUnion::FileCutUnion(QWidget *parent)
	: QWidget(parent), m_outputBlockNum(9999), m_outputStartAddr(0)
{
	ui.setupUi(this);

	QStyle* style = QApplication::style();
	QIcon icon = style->standardIcon(QStyle::SP_ArrowUp);
	ui.upFileBt->setIcon(icon);

	icon = style->standardIcon(QStyle::SP_ArrowDown);
	ui.downFileBt->setIcon(icon);

	icon = style->standardIcon(QStyle::SP_TitleBarCloseButton);
	ui.delFileBt->setIcon(icon);

}

FileCutUnion::~FileCutUnion()
{
}

void FileCutUnion::on_cutStartAddr(int status)
{
	if (Qt::Checked == status)
	{
		ui.startAddr->setEnabled(true);
	}
	else
	{
		ui.startAddr->setEnabled(false);
	}
}

void FileCutUnion::on_outputBlockNum(int status)
{
	if (Qt::Checked == status)
	{
		ui.blockNum->setEnabled(true);
	}
	else
	{
		ui.blockNum->setEnabled(false);
	}
}

//选择文件。
void FileCutUnion::on_selectSrcFile()
{
	//下面这个接口，可以选择多个文件。
	QStringList fileList = QFileDialog::getOpenFileNames(this,tr("Select File"));

	if(!fileList.isEmpty())
	{
		ui.listWidget->addItems(fileList);
		m_lastFileDir = fileList.at(0);
		ui.lineEdit->setText(m_lastFileDir);
	}
}

int readfile(QFile& file, char* data, int len)
{
	int readCount = 0;
	while (readCount < len)
	{
		int  count = file.read(data + readCount, (len - readCount));
		if (count <= 0)
		{
			break;
		}
		readCount += count;
	}
	return readCount;
}

int writefile(QFile& file, char* data, int len)
{
	int writeCount = 0;
	while (writeCount < len)
	{
		int  count = file.write(data + writeCount, (len - writeCount));
		if (count < 0)
		{
			break;
		}
		writeCount += count;
	}
	return writeCount;
}

void FileCutUnion::cutFile(QString filePath, QString saveDir)
{
	const qint64 blockSize = ui.blockSize->value();
	QFile file(filePath);

	qint64 cutStartAddr = m_outputStartAddr;

	ProgressWin* ploadFileProcessWin = nullptr;

	if (file.open(QIODevice::ReadOnly | QIODevice::ExistingOnly))
	{
		qint64 fileSize = file.size();

		QFileInfo fi(filePath);
		QString fileName = fi.fileName();
		bool isCancel = false;
		int partId = 0;

		int unitIndex = ui.unit->currentIndex();

		int ONE_UNIT_SIZE = 0;
		QString UNIT_STR = "M";

		if (0 == unitIndex)
		{
			ONE_UNIT_SIZE = 1024 * 1024;
		}
		else if (1 == unitIndex)
		{
			ONE_UNIT_SIZE = 1024;
			UNIT_STR = "K";
		}
		else if (2 == unitIndex)
		{
			ONE_UNIT_SIZE = 1;
			UNIT_STR = "";
		}

		if (fileSize <= blockSize * ONE_UNIT_SIZE)
		{
			ui.logEdit->appendPlainText(tr("file %1 is <= %2 %3B, skip to cut file !").arg(filePath).arg(blockSize).arg(UNIT_STR));
		}
		else if (cutStartAddr >= fileSize)
		{
			ui.logEdit->appendPlainText(tr("file %1 size is %2, < start cut address %3, skip to cut file !").arg(filePath).arg(fileSize).arg(cutStartAddr));
		}
		else
		{
			const int READ_BUF_SIZE = 1024 * 1024 * 5;//每次5M

			ploadFileProcessWin = new ProgressWin(this);

			//最多输出m_outputBlockNum个块。让用户可以控制输出多少块。
			int outputBlockNum = m_outputBlockNum;


			//从cutStartAddr地址开始分割文件，跳过文件前面cutStartAddr字节。
			if (cutStartAddr > 0)
			{
				fileSize -= cutStartAddr;
				file.seek(cutStartAddr);
			}

			if (outputBlockNum != 9999)
			{
				qint64 outputFileSize = blockSize * ONE_UNIT_SIZE * outputBlockNum;
				if (fileSize > outputFileSize)
				{
					fileSize = outputFileSize;
				}
			}

			ploadFileProcessWin->setWindowModality(Qt::WindowModal);
			ploadFileProcessWin->info(tr("Now cut file %1 to part file, please wait ...").arg(filePath));
			ploadFileProcessWin->setTotalSteps(fileSize / READ_BUF_SIZE + 1);
			ploadFileProcessWin->show();

			char* readBuf = new char[READ_BUF_SIZE];


			while ((fileSize > 0) && (outputBlockNum > 0))
			{
				--outputBlockNum;

				//如果是因为控制块输出格式，导致提前结束，则进度条退出时不检测进度是否完成。
				if (outputBlockNum == 0)
				{
					ploadFileProcessWin->setCloseAskProgressStatus(false);
				}

				//一个分块文件的大小。
				const qint64 oneBlockFileSize = blockSize * ONE_UNIT_SIZE;

				//每次读取一个块文件。
				QString partFilePath = QString("%1/%2-part%3").arg(saveDir).arg(fileName).arg(partId);

				QFile partFile(partFilePath);
				if (partFile.open(QIODevice::WriteOnly))
				{
					qint64 needReadSize = oneBlockFileSize;

					while (needReadSize > 0)
					{
						memset(readBuf, 0, READ_BUF_SIZE);

						int maxReadNum = (needReadSize >= READ_BUF_SIZE) ? READ_BUF_SIZE : needReadSize;

						int readNum = readfile(file, readBuf, maxReadNum);
						if ((readNum <=0) ||(readNum < maxReadNum))
						{
							//说明是最后一块，读取内容不足READ_BUF_SIZE
							needReadSize = 0;
						}
						else
						{
							needReadSize -= maxReadNum;
						}

						//写入新文件中
						writefile(partFile, readBuf, readNum);

						if (ploadFileProcessWin->isCancel())
						{
							isCancel = true;
							break;
						}

						ploadFileProcessWin->moveStep(true);
					}

					//一个新块文件，写入完毕。
					partFile.close();

					ploadFileProcessWin->info(tr("file %1 create part %2 file finished ...").arg(fileName).arg(partId));
				}
				else
				{
					ui.logEdit->appendPlainText(tr("create file %1 failed ! cut file failed !").arg(partFilePath));
					break;
				}

				++partId;
				//每次写入一个分块后，则减去分块大小。
				fileSize -= oneBlockFileSize;

				if (isCancel)
				{
					break;
				}
			}

			delete[]readBuf;

		}
		file.close();

		if (ploadFileProcessWin != nullptr)
		{
			ploadFileProcessWin->close();
			delete ploadFileProcessWin;
		}

		if (!isCancel)
		{
			ui.logEdit->appendPlainText(tr("file %1 cut finished, file name is %2-part0 - %2-part%3").arg(filePath).arg(fileName).arg(partId-1));
		}
		else
		{
			ui.logEdit->appendPlainText(tr("file %1 cut cancel !").arg(filePath));
		}
	}
}

//开始分割文件。
void FileCutUnion::on_fileCut()
{
	QFileInfo fi(m_lastFileDir);

	QString saveDir = QFileDialog::getExistingDirectory(this, tr("Input Save Directory"),
		fi.absoluteDir().absolutePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (!saveDir.isEmpty())
	{
		QListWidgetItem* pItem = nullptr;

		//控制输出块的个数。
		if (ui.cutNumCb->isChecked())
		{
			m_outputBlockNum = ui.blockNum->value();
		}
		else
		{
			m_outputBlockNum = 9999;
		}

		//控制从文件偏移量开始进行切割。
		if (ui.startAddrCb->isChecked())
		{
			m_outputStartAddr = ui.startAddr->text().toLongLong();
		}
		else
		{
			m_outputStartAddr = 0;
		}

		for (int i = 0; i < ui.listWidget->count(); ++i)
		{
			pItem = ui.listWidget->item(i);

			cutFile(pItem->text(), saveDir);
		}

	}
}

const int READ_BUF_SIZE = 1024 * 1024 * 5;//每次5M

//合并文件。
void FileCutUnion::mergeFile(QFile& dstFile, QString filePath, ProgressWin* ploadFileProcessWin)
{
	QFile srcFile(filePath);

	if (srcFile.open(QIODevice::ReadOnly | QIODevice::ExistingOnly))
	{
		qint64 needReadSize = srcFile.size();

		char* readBuf = new char[READ_BUF_SIZE];

		while (needReadSize > 0)
		{
			memset(readBuf, 0, READ_BUF_SIZE);
			int readNum = readfile(srcFile, readBuf, READ_BUF_SIZE);
			if (readNum < READ_BUF_SIZE)
			{
				//说明是最后一块，读取内容不足READ_BUF_SIZE
				needReadSize = 0;
			}
			else
			{
				needReadSize -= READ_BUF_SIZE;
			}

			//写入新文件中
			writefile(dstFile, readBuf, readNum);

			if (ploadFileProcessWin->isCancel())
			{
				break;
			}

			ploadFileProcessWin->moveStep(true);
		}
		//一个新块文件，写入完毕。
		srcFile.close();

		delete []readBuf;

		ploadFileProcessWin->info(tr("file %1 merge finished ...").arg(filePath));
	}
}

void FileCutUnion::on_clear()
{
	ui.listWidget->clear();
	ui.logEdit->clear();
	ui.startAddr->setText("");
	ui.startAddrCb->setChecked(false);
	ui.cutNumCb->setChecked(false);
}


//文本合并
void FileCutUnion::on_fileMerge()
{
	QString saveFilePath = QFileDialog::getSaveFileName(this, tr("Merge File As ..."), QString());

	if (!saveFilePath.isEmpty())
	{
			QListWidgetItem* pItem = nullptr;

			QFile file(saveFilePath);

			if (!file.open(QIODevice::ReadWrite))
			{
				return;
			}

			qint64 totalMergeFileSize = 0;

			for (int i = 0; i < ui.listWidget->count(); ++i)
			{
				pItem = ui.listWidget->item(i);

				QFileInfo fi(pItem->text());

				totalMergeFileSize += fi.size();
			}

			ProgressWin* ploadFileProcessWin = new ProgressWin(this);

			ploadFileProcessWin->setWindowModality(Qt::WindowModal);
			ploadFileProcessWin->info(tr("Now merge file to %1 , please wait ...").arg(saveFilePath));
			ploadFileProcessWin->setTotalSteps(totalMergeFileSize / READ_BUF_SIZE + 1);
			ploadFileProcessWin->show();

			for (int i = 0; i < ui.listWidget->count(); ++i)
			{
				pItem = ui.listWidget->item(i);

				mergeFile(file, pItem->text(), ploadFileProcessWin);

				if (ploadFileProcessWin->isCancel())
				{
					break;
				}
			}

			file.close();

			if (!ploadFileProcessWin->isCancel())
			{
				ui.logEdit->appendPlainText(tr("file %1 merge finished !").arg(saveFilePath));
			}
			else
			{
				ui.logEdit->appendPlainText(tr("file %1 merge cancel !").arg(saveFilePath));
			}

			ploadFileProcessWin->close();
			delete ploadFileProcessWin;

	}

}

void FileCutUnion::on_up()
{
	QListWidgetItem* item = ui.listWidget->currentItem();
	int rows = ui.listWidget->count();

	if (item != nullptr)
	{
		int row = ui.listWidget->currentRow();
		if (row > 0 && row < rows)
		{
			item = ui.listWidget->takeItem(row);
			int newRow = row - 1;
			ui.listWidget->insertItem(newRow, item);
			ui.listWidget->setCurrentRow(newRow);
		}
	}
}

void FileCutUnion::on_down()
{
	QListWidgetItem* item = ui.listWidget->currentItem();
	int rows = ui.listWidget->count();

	if (item != nullptr)
	{
		int row = ui.listWidget->currentRow();
		if (row < (rows - 1) && row >= 0)
		{
			item = ui.listWidget->takeItem(row);
			int newRow = row + 1;
			ui.listWidget->insertItem(newRow, item);
			ui.listWidget->setCurrentRow(newRow);
		}
	}
}

void FileCutUnion::on_delete()
{
	int rows = ui.listWidget->count();
	int row = ui.listWidget->currentRow();
	if (row <= (rows - 1) && row >= 0)
	{
		QListWidgetItem* item = ui.listWidget->takeItem(row);
		delete item;
	}
}
