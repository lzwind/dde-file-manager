/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "crumbmodel.h"
#include "dtkwidget_global.h"

DWIDGET_USE_NAMESPACE

CrumbModel::CrumbModel(QObject *parent)
    : QStandardItemModel(parent)
{
}

CrumbModel::~CrumbModel()
{
}

void CrumbModel::removeAll()
{
    removeRows(0, rowCount());
}

QModelIndex CrumbModel::lastIndex()
{
    return index(rowCount() - 1, 0);
}
