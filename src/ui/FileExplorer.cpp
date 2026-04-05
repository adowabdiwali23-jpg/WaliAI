#include "ui/FileExplorer.h"

#include <QHeaderView>

FileExplorer::FileExplorer(const QString &rootPath, QWidget *parent)
    : QWidget(parent), m_rootPath(rootPath)
{
    setupUi();
}

void FileExplorer::setupUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    auto *titleLabel = new QLabel("File Explorer");
    titleLabel->setStyleSheet("QLabel { color: #e2e8f0; font-weight: bold; padding: 4px; }");
    layout->addWidget(titleLabel);

    m_model = new QFileSystemModel(this);
    m_model->setRootPath(m_rootPath);
    m_model->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

    m_treeView = new QTreeView(this);
    m_treeView->setModel(m_model);
    m_treeView->setRootIndex(m_model->index(m_rootPath));
    m_treeView->setColumnHidden(1, true); // Size
    m_treeView->setColumnHidden(2, true); // Type
    m_treeView->setColumnHidden(3, true); // Date
    m_treeView->header()->hide();
    m_treeView->setAnimated(true);
    m_treeView->setIndentation(16);
    m_treeView->setStyleSheet(
        "QTreeView { background-color: #1a202c; color: #e2e8f0; "
        "border: none; font-size: 12px; }"
        "QTreeView::item { padding: 3px; }"
        "QTreeView::item:selected { background-color: #2d3748; }"
        "QTreeView::item:hover { background-color: #2d3748; }"
    );
    layout->addWidget(m_treeView, 1);

    connect(m_treeView, &QTreeView::clicked, this, [this](const QModelIndex &index) {
        QString path = m_model->filePath(index);
        if (!m_model->isDir(index)) {
            emit fileSelected(path);
        }
    });

    setStyleSheet("QWidget { background-color: #1a202c; }");
}

void FileExplorer::setRootPath(const QString &path)
{
    m_rootPath = path;
    m_model->setRootPath(path);
    m_treeView->setRootIndex(m_model->index(path));
}

void FileExplorer::refresh()
{
    m_model->setRootPath(m_rootPath);
    m_treeView->setRootIndex(m_model->index(m_rootPath));
}
