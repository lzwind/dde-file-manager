/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "config.h"   //cmake

#include <dfm-framework/framework.h>

#include <DApplication>
#include <DMainWindow>

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QtGlobal>

#include <iostream>
#include <algorithm>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

#ifdef DFM_ORGANIZATION_NAME
#    define ORGANIZATION_NAME DFM_ORGANIZATION_NAME
#else
#    define ORGANIZATION_NAME "deepin"
#endif

/// @brief PLUGIN_INTERFACE 默认插件iid
static const char *const kFmPluginInterface = "org.deepin.plugin.filemanager";
static const char *const kCommonPluginInterface = "org.deepin.plugin.common";
static const char *const kPluginCore = "dfmplugin-core";
static const char *const kLibCore = "libdfmplugin-core.so";

static bool pluginsLoad()
{
    dpfCheckTimeBegin();

    auto &&lifeCycle = dpfInstance.lifeCycle();

    // set plugin iid from qt style
    lifeCycle.addPluginIID(kFmPluginInterface);
    lifeCycle.addPluginIID(kCommonPluginInterface);

    QString pluginsDir(qApp->applicationDirPath() + "/../../plugins");
    if (!QDir(pluginsDir).exists()) {
        pluginsDir = DFM_PLUGIN_PATH;
    }
    qDebug() << "using plugins dir:" << pluginsDir;

    lifeCycle.setPluginPaths({ pluginsDir });

    qInfo() << "Depend library paths:" << DApplication::libraryPaths();
    qInfo() << "Load plugin paths: " << dpf::LifeCycle::pluginPaths();

    // read all plugins in setting paths
    if (!lifeCycle.readPlugins())
        return false;

    // We should make sure that the core plugin is loaded first
    auto corePlugin = lifeCycle.pluginMetaObj(kPluginCore);
    if (corePlugin.isNull())
        return false;
    if (!corePlugin->fileName().contains(kLibCore))
        return false;
    if (!lifeCycle.loadPlugin(corePlugin))
        return false;

    // load plugins without core
    if (!lifeCycle.loadPlugins())
        return false;

    dpfCheckTimeEnd();

    return true;
}

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    a.setOrganizationName(ORGANIZATION_NAME);

    dpfInstance.initialize();

    if (!pluginsLoad()) {
        qCritical() << "Load pugin failed!";
        abort();
    }

    return a.exec();
}
