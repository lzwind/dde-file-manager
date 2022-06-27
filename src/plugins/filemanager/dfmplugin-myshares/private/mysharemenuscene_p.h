/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef MYSHAREMENUSCENE_P_H
#define MYSHAREMENUSCENE_P_H

#include "dfmplugin_myshares_global.h"
#include <interfaces/private/abstractmenuscene_p.h>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_myshares {

class MyShareMenuScene;
class MyShareMenuScenePrivate : public AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class MyShareMenuScene;

public:
    explicit MyShareMenuScenePrivate(AbstractMenuScene *qq);

private:
    void createFileMenu(QMenu *parent);
    bool triggered(const QString &id);
};

}

#endif   // MYSHAREMENUSCENE_P_H
