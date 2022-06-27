/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef MOVETOtRASHFILES_H
#define MOVETOtRASHFILES_H

#include "fileoperations/fileoperationutils/abstractjob.h"
#include "dfm-base/interfaces/abstractjobhandler.h"

#include <QObject>
#include <QThread>

DFMBASE_USE_NAMESPACE
namespace dfm_service_common {
class MoveToTrashFiles : private AbstractJob
{
    Q_OBJECT
    friend class FileOperationsService;
    explicit MoveToTrashFiles(QObject *parent = nullptr);

public:
    ~MoveToTrashFiles() override;
};
}

#endif   // MOVETOtRASHFILES_H
