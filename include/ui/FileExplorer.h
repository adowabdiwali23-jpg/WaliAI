#pragma once

#include <QWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QVBoxLayout>
#include <QLabel>

class FileExplorer : public QWidget
{
    Q_OBJECT

public:
    explicit FileExplorer(const QString &rootPath, QWidget *parent = nullptr);

    void setRootPath(const QString &path);
    void refresh();

signals:
    void fileSelected(const QString &filePath);

private:
    void setupUi();

    QTreeView *m_treeView = nullptr;
    QFileSystemModel *m_model = nullptr;
    QString m_rootPath;
};
