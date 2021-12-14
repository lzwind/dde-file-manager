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
#ifndef DFMCOMPLETERVIEW_H
#define DFMCOMPLETERVIEW_H

#include <QCompleter>
#include <QListView>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QFileSystemModel>

class CompleterViewDelegate;
class CompleterViewPrivate;
class CompleterView : public QListView
{
    Q_OBJECT
    friend class CompleterViewPrivate;
    CompleterViewPrivate *const d;

public:
    explicit CompleterView();
    QCompleter *completer();
    QStringListModel *model();
    CompleterViewDelegate *itemDelegate();

Q_SIGNALS:
    void completerActivated(const QString &text);
    void completerActivated(const QModelIndex &index);
    void completerHighlighted(const QString &text);
    void completerHighlighted(const QModelIndex &index);
};

#endif   //DFMCOMPLETERVIEW_H
