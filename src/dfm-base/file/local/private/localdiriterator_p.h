// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCALFILEDIRITERATOR_P_H
#define LOCALFILEDIRITERATOR_P_H

#include "file/local/localdiriterator.h"
#include "base/urlroute.h"

#include <QDirIterator>
#include <QPointer>
#include <QDebug>

#include <dfm-io/core/denumerator.h>
#include <dfm-io/dfmio_global.h>
#include <dfm-io/dfmio_register.h>
#include <dfm-io/core/diofactory.h>

USING_IO_NAMESPACE
namespace dfmbase {
class LocalDirIterator;
class LocalDirIteratorPrivate : public QObject
{
    friend class LocalDirIterator;
    class LocalDirIterator *const q;

public:
    struct InitQuerierAsyncOp
    {
        QPointer<LocalDirIteratorPrivate> me;
        QUrl url;
    };

    explicit LocalDirIteratorPrivate(const QUrl &url,
                                     const QStringList &nameFilters,
                                     QDir::Filters filters,
                                     QDirIterator::IteratorFlags flags,
                                     LocalDirIterator *q);
    virtual ~LocalDirIteratorPrivate() override;

    void initQuerierAsyncCallback(bool succ, void *data);
    void cacheAttribute(const QUrl &url);
    AbstractFileInfoPointer fileInfo();

private:
    QSharedPointer<dfmio::DEnumerator> dfmioDirIterator = nullptr;   // dfmio的文件迭代器
    QUrl currentUrl;   // 当前迭代器所在位置文件的url
    QSet<QString> hideFileList;
    bool isLocalDevice = false;
    bool isCdRomDevice = false;
};
}
#endif   // ABSTRACTDIRITERATOR_P_H