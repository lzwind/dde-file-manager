#ifndef UNIFIED_FILEBROWSER_H
#define UNIFIED_FILEBROWSER_H

#include <QMainWindow>
#include <QListWidget>
#include <QListWidgetItem>
#include <QToolBar>
#include <QStatusBar>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QMenu>
#include <QFileSystemWatcher>
#include <QFileIconProvider>
#include <QDir>
#include <QTimer>
#include <QProgressBar>
#include <QSplitter>
#include <QTreeWidget>
#include <QGroupBox>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QMutex>

#include "simple_fileitem.h"

/**
 * @brief 统一的文件浏览器类 - 将所有功能合并到一个类中
 * 
 * 这个类包含了原来分散在多个类中的所有功能：
 * - 文件系统数据管理
 * - UI界面和交互
 * - 排序和分组功能
 * - 上下文菜单
 * - 选择管理
 * - 文件操作
 */
class UnifiedFileBrowser : public QMainWindow
{
    Q_OBJECT

public:
    // 排序类型枚举
    enum SortType {
        SortByName,
        SortByModified,
        SortByCreated,
        SortBySize,
        SortByType
    };

    // 分组类型枚举
    enum GroupType {
        NoGrouping,
        GroupByType,
        GroupByModifiedTime,
        GroupByCreatedTime,
        GroupByName,
        GroupBySize
    };

    // 排序顺序
    enum SortOrder { Ascending, Descending };

    explicit UnifiedFileBrowser(QWidget *parent = nullptr);
    explicit UnifiedFileBrowser(const QString& initialPath, QWidget *parent = nullptr);
    ~UnifiedFileBrowser();

public slots:
    void navigateToPath(const QString& path);
    void refreshCurrentDirectory();
    void goHome();
    void goUp();
    void setSortType(SortType type, SortOrder order = Ascending);
    void setGroupType(GroupType type, SortOrder order = Ascending);

private slots:
    // 界面响应
    void onPathEditChanged();
    void onItemDoubleClicked();
    void onItemSelectionChanged();
    void onSortComboChanged();
    void onGroupComboChanged();
    void onCustomContextMenu(const QPoint& point);
    
    // 文件系统响应
    void onDirectoryChanged(const QString& path);
    void onFileChanged(const QString& path);
    void onLoadingFinished();

private:
    // UI设置
    void setupUI();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();
    void setupContextMenu();
    
    // 文件操作
    void loadDirectory(const QString& path);
    void loadDirectoryAsync(const QString& path);
    void organizeFiles();
    void applyGrouping();
    void applySorting();
    void updateDisplay();
    void updateStatusBar();
    
    // 排序功能
    void sortFilesByName(QList<SimpleFileItem>& files, SortOrder order);
    void sortFilesByModified(QList<SimpleFileItem>& files, SortOrder order);
    void sortFilesByCreated(QList<SimpleFileItem>& files, SortOrder order);
    void sortFilesBySize(QList<SimpleFileItem>& files, SortOrder order);
    void sortFilesByType(QList<SimpleFileItem>& files, SortOrder order);
    
    // 分组功能
    void groupFilesByType(QList<SimpleFileItem>& files, SortOrder order);
    void groupFilesByModifiedTime(QList<SimpleFileItem>& files, SortOrder order);
    void groupFilesByCreatedTime(QList<SimpleFileItem>& files, SortOrder order);
    void groupFilesByName(QList<SimpleFileItem>& files, SortOrder order);
    void groupFilesBySize(QList<SimpleFileItem>& files, SortOrder order);
    
    // 显示辅助
    void addGroupHeader(const QString& groupName, int itemCount);
    void addFileItem(const SimpleFileItem& item);
    void clearDisplay();
    
    // 工具函数
    QString getTypeGroup(const QString& type) const;
    QString getTimeGroup(const QDateTime& time) const;
    QString getNameGroup(const QString& name) const;
    QString getSizeGroup(qint64 size, bool isDirectory) const;
    
    // 上下文菜单
    void showContextMenu(const QPoint& globalPos);
    void openSelectedItems();
    void showItemProperties();
    void deleteSelectedItems();
    void copySelectedItems();
    void cutSelectedItems();
    void pasteItems();

private:
    // 核心数据
    QString m_currentPath;
    QList<SimpleFileItem> m_allFiles;
    QList<SimpleFileItem> m_displayFiles;
    QStringList m_groupOrder;
    
    // 当前设置
    SortType m_currentSortType;
    SortOrder m_currentSortOrder;
    GroupType m_currentGroupType;
    SortOrder m_currentGroupOrder;
    
    // UI组件
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QListWidget* m_fileList;
    
    // 工具栏组件
    QToolBar* m_toolBar;
    QLineEdit* m_pathEdit;
    QComboBox* m_sortCombo;
    QComboBox* m_groupCombo;
    QAction* m_homeAction;
    QAction* m_upAction;
    QAction* m_refreshAction;
    
    // 状态栏组件
    QLabel* m_pathLabel;
    QLabel* m_itemCountLabel;
    QLabel* m_selectionLabel;
    QProgressBar* m_loadingProgress;
    
    // 上下文菜单
    QMenu* m_contextMenu;
    QAction* m_openAction;
    QAction* m_propertiesAction;
    QAction* m_deleteAction;
    QAction* m_copyAction;
    QAction* m_cutAction;
    QAction* m_pasteAction;
    
    // 文件系统监控
    QFileSystemWatcher* m_watcher;
    QFileIconProvider* m_iconProvider;
    QTimer* m_loadingTimer;
    
    // 选择状态
    QStringList m_selectedPaths;
    QString m_clipboardAction; // "copy" or "cut"
    QStringList m_clipboardPaths;
    
    // 线程安全
    QMutex m_dataMutex;
};

#endif // UNIFIED_FILEBROWSER_H
