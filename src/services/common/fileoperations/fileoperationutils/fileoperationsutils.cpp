/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "fileoperationsutils.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/utils/fileutils.h"

#include <QDirIterator>
#include <QUrl>
#include <QDebug>

#undef signals
extern "C" {
#include <gio/gio.h>
}
#define signals public

#include <sys/vfs.h>
#include <sys/stat.h>
#include <fts.h>
#include <unistd.h>
#include <sys/utsname.h>

DSC_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

/*!
 * \brief FileOperationsUtils::statisticsFilesSize 使用c库统计文件大小
 * 统计了所有文件的大小信息（如果文件大小 <= 0就统计这个文件大小为一个内存页，有些文件系统dir有大小，就统计dir大小，
 * 如果dir大小 <= 0就统计这个dir大小为一个内存页），统计文件的数量, 统计所有的文件及子目录路径
 * \param files 统计文件的urllist
 * \param isRecordUrl 是否统计所有的文件及子目录路径
 * \return QSharedPointer<FileOperationsUtils::FilesSizeInfo> 文件大小信息
 */
SizeInfoPointer FileOperationsUtils::statisticsFilesSize(const QList<QUrl> &files, const bool &isRecordUrl)
{
    SizeInfoPointer filesSizeInfo(new dfmbase::FileUtils::FilesSizeInfo);

    for (auto url : files) {
        statisticFilesSize(url, filesSizeInfo, isRecordUrl);
    }

    filesSizeInfo->dirSize = filesSizeInfo->dirSize <= 0 ? FileUtils::getMemoryPageSize() : filesSizeInfo->dirSize;
    return filesSizeInfo;
}

bool FileOperationsUtils::isFilesSizeOutLimit(const QUrl &url, const qint64 limitSize)
{
    qint64 totalSize = 0;
    char *paths[2] = { nullptr, nullptr };
    paths[0] = strdup(url.path().toUtf8().toStdString().data());
    FTS *fts = fts_open(paths, 0, nullptr);
    if (paths[0])
        free(paths[0]);

    if (nullptr == fts) {
        perror("fts_open");
        qWarning() << "fts_open open error : " << QString::fromLocal8Bit(strerror(errno));
        return false;
    }
    while (1) {
        FTSENT *ent = fts_read(fts);
        if (ent == nullptr) {
            break;
        }
        unsigned short flag = ent->fts_info;

        if (flag != FTS_DP)
            totalSize += ent->fts_statp->st_size <= 0 ? FileUtils::getMemoryPageSize() : ent->fts_statp->st_size;

        if (totalSize > limitSize)
            break;
    }

    fts_close(fts);

    return totalSize > limitSize;
}

void FileOperationsUtils::statisticFilesSize(const QUrl &url,
                                             SizeInfoPointer &sizeInfo,
                                             const bool &isRecordUrl)
{
    char *paths[2] = { nullptr, nullptr };
    paths[0] = strdup(url.path().toUtf8().toStdString().data());
    FTS *fts = fts_open(paths, 0, nullptr);
    if (paths[0])
        free(paths[0]);

    if (nullptr == fts) {
        perror("fts_open");
        qWarning() << "fts_open open error : " << QString::fromLocal8Bit(strerror(errno));
        return;
    }
    while (1) {
        FTSENT *ent = fts_read(fts);
        if (ent == nullptr) {
            break;
        }
        unsigned short flag = ent->fts_info;
        if (isRecordUrl && flag != FTS_DP)
            sizeInfo->allFiles.append(QUrl::fromLocalFile(ent->fts_path));
        if (flag != FTS_DP)
            sizeInfo->totalSize += ent->fts_statp->st_size <= 0 ? FileUtils::getMemoryPageSize() : ent->fts_statp->st_size;
        if (sizeInfo->dirSize == 0 && flag == FTS_D)
            sizeInfo->dirSize = ent->fts_statp->st_size <= 0 ? FileUtils::getMemoryPageSize() : static_cast<quint16>(ent->fts_statp->st_size);
        if (flag == FTS_F)
            sizeInfo->fileCount++;
    }
    fts_close(fts);
}

bool FileOperationsUtils::isAncestorUrl(const QUrl &from, const QUrl &to)
{
    QUrl parentUrl = UrlRoute::urlParent(to);
    return from.path() == parentUrl.path();
}

bool FileOperationsUtils::isFileOnDisk(const QUrl &url)
{
    if (!url.isValid())
        return false;

    g_autoptr(GFile) destDirFile = g_file_new_for_uri(url.toString().toLocal8Bit().data());
    g_autoptr(GMount) destDirMount = g_file_find_enclosing_mount(destDirFile, nullptr, nullptr);
    if (destDirMount) {
        return !g_mount_can_unmount(destDirMount);
    }
    return true;
}

void FileOperationsUtils::getDirFiles(const QUrl &url, QList<QUrl> &files)
{
    DIR *dir = nullptr;
    struct dirent *ptr = nullptr;
    if ((dir = opendir(url.path().toStdString().data())) == nullptr) {
        qCritical() << "Open dir error by system c opendir function";
        return;
    }

    QString urlPath = url.path();
    if (!urlPath.endsWith("/"))
        urlPath.append("/");
    files.clear();
    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        } else if (ptr->d_type == 4 || ptr->d_type == 8 || ptr->d_type == 10) {
            files.append(QUrl::fromLocalFile(urlPath + ptr->d_name));
        }
    }
}
