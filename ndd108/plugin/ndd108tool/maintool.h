#pragma once

#include <QMainWindow>
#include <QMap>
#include "ui_maintool.h"

class MainTool : public QMainWindow
{
    Q_OBJECT

public:
    MainTool(QWidget *parent);
    ~MainTool();

private:
    void init();
    bool removeOldStackWidget(int funId);
    QWidget* findWid(int row);

private slots:
    void on_funItemClick(int row);
    void on_showMsg(QString info);

private:
    Ui::MainToolClass ui;
    QMap<int, QWidget*> m_widMap;
    QWidget* m_pNotepad;
};
