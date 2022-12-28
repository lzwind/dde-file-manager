/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
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
#include "filedialogmenuscene.h"

#include "dfm-base/dfm_menu_defines.h"

#include <QMenu>

using namespace filedialog_core;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *FileDialogMenuCreator::create()
{
    return new FileDialogMenuScene();
}

FileDialogMenuScene::FileDialogMenuScene(QObject *parent)
    : AbstractMenuScene(parent)
{
}

QString FileDialogMenuScene::name() const
{
    return FileDialogMenuCreator::name();
}

bool FileDialogMenuScene::initialize(const QVariantHash &params)
{
    Q_UNUSED(params)

    workspaceScene = dynamic_cast<AbstractMenuScene *>(this->parent());
    return AbstractMenuScene::initialize(params);
}

void FileDialogMenuScene::updateState(QMenu *parent)
{
    Q_ASSERT(parent);

    filterAction(parent, false);
    AbstractMenuScene::updateState(parent);
}

QString FileDialogMenuScene::findSceneName(QAction *act) const
{
    QString name;
    if (workspaceScene) {
        auto childScene = workspaceScene->scene(act);
        if (childScene)
            name = childScene->name();
    }
    return name;
}

void FileDialogMenuScene::filterAction(QMenu *parent, bool isSubMenu)
{
    // TODO(zhangs): add whitelist by global config
    static const QStringList whiteActIdList { "new-folder", "new-document", "display-as", "sort-by",
                                              "open", "rename", "delete", "copy", "cut", "paste" };
    static const QStringList whiteSceneList { "NewCreateMenu", "ClipBoardMenu", "OpenDirMenu", "FileOperatorMenu",
                                              "OpenWithMenu", "ShareMenu", "SortAndDisplayMenu" };

    auto actions = parent->actions();
    for (auto act : actions) {
        if (act->isSeparator()) {
            act->setVisible(true);
            continue;
        }

        QString id { act->property(ActionPropertyKey::kActionID).toString() };
        QString sceneName { findSceneName(act) };
        if (!isSubMenu) {
            if (!whiteActIdList.contains(id) || !whiteSceneList.contains(sceneName)) {
                act->setVisible(false);
            } else {
                auto subMenu = act->menu();
                if (subMenu)
                    filterAction(subMenu, true);
            }
        } else {
            if (!whiteSceneList.contains(sceneName))
                act->setVisible(false);
        }
    }
}
