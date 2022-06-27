/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef CANVASVIEWMENUPROXY_H
#define CANVASVIEWMENUPROXY_H

#include "ddplugin_canvas_global.h"

#include <QObject>

namespace ddplugin_canvas {
class CanvasView;
class CanvasViewMenuProxy : public QObject
{
    Q_OBJECT
public:
    explicit CanvasViewMenuProxy(CanvasView *parent = nullptr);
    ~CanvasViewMenuProxy();
    static bool disableMenu();
    void showEmptyAreaMenu(const Qt::ItemFlags &indexFlags, const QPoint gridPos);
    void showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags, const QPoint gridPos);
public slots:
    void changeIconLevel(bool increase);

private:
    CanvasView *view;
};

}

#endif   // CANVASVIEWMENUPROXY_H
