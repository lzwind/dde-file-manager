#ifndef SIMPLE_FILEITEM_H
#define SIMPLE_FILEITEM_H

#include <QString>
#include <QDateTime>
#include <QIcon>
#include <QFileInfo>

/**
 * @brief 简化的文件项数据结构
 * 包含文件的基本信息和显示所需的数据
 */
struct SimpleFileItem {
    QString name;           // 文件名
    QString path;           // 完整路径
    QString type;           // 文件类型
    qint64 size;           // 文件大小
    QDateTime modified;     // 修改时间
    QDateTime created;      // 创建时间
    bool isDirectory;       // 是否为目录
    QIcon icon;            // 图标
    
    // 分组和排序相关
    QString group;          // 所属组
    int groupOrder;         // 组内排序序号
    
    SimpleFileItem() : size(0), isDirectory(false), groupOrder(0) {}
    
    SimpleFileItem(const QFileInfo& info) {
        name = info.fileName();
        path = info.absoluteFilePath();
        type = info.isDir() ? "Directory" : info.suffix().toUpper();
        size = info.isDir() ? -1 : info.size();
        modified = info.lastModified();
        created = info.birthTime();
        isDirectory = info.isDir();
        groupOrder = 0;
    }
    
    // 格式化大小显示
    QString formatSize() const {
        if (isDirectory) return "--";
        if (size < 1024) return QString("%1 B").arg(size);
        if (size < 1024 * 1024) return QString("%1 KB").arg(size / 1024);
        if (size < 1024 * 1024 * 1024) return QString("%1 MB").arg(size / 1024 / 1024);
        return QString("%1 GB").arg(size / 1024 / 1024 / 1024);
    }
    
    // 格式化时间显示
    QString formatTime(const QDateTime& dt) const {
        return dt.toString("yyyy-MM-dd hh:mm");
    }
};

#endif // SIMPLE_FILEITEM_H
