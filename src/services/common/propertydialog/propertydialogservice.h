/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef PROPERTYDIALOGSERVICE_H
#define PROPERTYDIALOGSERVICE_H

#include "utils/registercreateprocess.h"

#include <dfm-framework/framework.h>

namespace dfm_service_common {

class PropertyDialogService final : public dpf::PluginService, dpf::AutoServiceRegister<PropertyDialogService>
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertyDialogService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

private:
    explicit PropertyDialogService(QObject *parent = nullptr);

public:
    static QString name()
    {
        return "org.deepin.service.PropertyDialogService";
    }

    static PropertyDialogService *service();

    bool registerControlExpand(CPY_NAMESPACE::createControlViewFunc view, int index = -1);

    bool registerCustomizePropertyView(CPY_NAMESPACE::createControlViewFunc view, QString scheme);

    bool registerBasicViewFiledExpand(CPY_NAMESPACE::basicViewFieldFunc func, const QString &scheme);

    bool registerFilterControlField(const QString &scheme, CPY_NAMESPACE::FilePropertyControlFilter filter);

    QWidget *createWidget(const QUrl &url);

    QMap<int, QWidget *> createControlView(const QUrl &url);

    QMap<CPY_NAMESPACE::BasicExpandType, CPY_NAMESPACE::BasicExpand> basicExpandField(const QUrl &url);

    CPY_NAMESPACE::FilePropertyControlFilter contorlFieldFilter(const QUrl &url);

    bool isContains(const QUrl &url);

    void addComputerPropertyToPropertyService();
};
}
#define propertyServIns ::DSC_NAMESPACE::PropertyDialogService::service()
#endif   // PROPERTYDIALOGSERVICE_H
