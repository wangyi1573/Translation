#pragma once
// rcglobal.h - Types for ndd plugin build

#include <QTreeWidgetItem>
#include <QString>

enum RcType {
	RC_DIR = 0,
	RC_FILE = 1
};

struct fileAttriNode {
	RcType type = RC_DIR;
	QTreeWidgetItem* selfItem = nullptr;
	QTreeWidgetItem* parent = nullptr;
	QString relativePath;
};

struct WalkFileInfo {
	int direction = 0;
	QTreeWidgetItem* root = nullptr;
	QString path;

	WalkFileInfo() = default;
	WalkFileInfo(int dir, QTreeWidgetItem* r, const QString& p)
		: direction(dir), root(r), path(p) {}
};
