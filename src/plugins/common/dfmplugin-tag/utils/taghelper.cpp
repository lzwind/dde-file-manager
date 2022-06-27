/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
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
#include "taghelper.h"
#include "tagmanager.h"
#include "widgets/tageditor.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"

#include <QMap>
#include <QVector>
#include <QPainter>
#include <QUrl>

#include <random>
#include <mutex>

DSB_FM_USE_NAMESPACE
DSC_USE_NAMESPACE
using namespace dfmplugin_tag;

TagHelper *TagHelper::instance()
{
    static TagHelper ins;
    return &ins;
}

QList<QColor> TagHelper::defualtColors() const
{
    QList<QColor> colors;

    for (const TagColorDefine &define : colorDefines) {
        colors << define.color;
    }

    return colors;
}

TagHelper::TagHelper(QObject *parent)
    : QObject(parent)
{
    initTagColorDefines();
}

QColor TagHelper::qureyColorByColorName(const QString &name) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [name](const TagColorDefine &define) {
        return define.colorName == name;
    });

    if (ret != colorDefines.cend())
        return ret->color;

    return QColor();
}

QColor TagHelper::qureyColorByDisplayName(const QString &name) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [name](const TagColorDefine &define) {
        return define.displayName == name;
    });

    if (ret != colorDefines.cend())
        return ret->color;

    return QColor();
}

QString TagHelper::qureyColorNameByColor(const QColor &color) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [color](const TagColorDefine &define) {
        return define.color.name() == color.name();
    });

    if (ret != colorDefines.cend())
        return ret->colorName;

    return QString();
}

QString TagHelper::qureyIconNameByColorName(const QString &colorName) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [colorName](const TagColorDefine &define) {
        return define.colorName == colorName;
    });

    if (ret != colorDefines.cend())
        return ret->iconName;

    return QString();
}

QString TagHelper::qureyIconNameByColor(const QColor &color) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [color](const TagColorDefine &define) {
        return define.color.name() == color.name();
    });

    if (ret != colorDefines.cend())
        return ret->iconName;

    return QString();
}

QString TagHelper::qureyDisplayNameByColor(const QColor &color) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [color](const TagColorDefine &define) {
        return define.color.name() == color.name();
    });

    if (ret != colorDefines.cend())
        return ret->displayName;

    return QString();
}

QString TagHelper::qureyColorNameByDisplayName(const QString &name) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [name](const TagColorDefine &define) {
        return define.displayName == name;
    });

    if (ret != colorDefines.cend())
        return ret->colorName;

    return QString();
}

QString TagHelper::getTagNameFromUrl(const QUrl &url) const
{
    if (url.scheme() == TagManager::scheme())
        return url.path().mid(1, url.path().length() - 1);

    return QString();
}

QUrl TagHelper::makeTagUrlByTagName(const QString &tag) const
{
    QUrl tagUrl;
    tagUrl.setScheme(TagManager::scheme());
    tagUrl.setPath("/" + tag);

    return tagUrl;
}

QString TagHelper::getColorNameByTag(const QString &tagName) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [tagName](const TagColorDefine &define) {
        return define.displayName == tagName;
    });

    if (ret != colorDefines.cend())
        return ret->colorName;

    return randomTagDefine().colorName;
}

bool TagHelper::isDefualtTag(const QString &tagName) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [tagName](const TagColorDefine &define) {
        return define.displayName == tagName;
    });

    return ret != colorDefines.cend();
}

void TagHelper::paintTags(QPainter *painter, QRectF &rect, const QList<QColor> &colors) const
{
    bool antialiasing = painter->testRenderHint(QPainter::Antialiasing);
    const QPen pen = painter->pen();
    const QBrush brush = painter->brush();

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(Qt::white, 1));

    for (const QColor &color : colors) {
        QPainterPath circle;

        painter->setBrush(QBrush(color));
        circle.addEllipse(QRectF(QPointF(rect.right() - kTagDiameter, rect.top()), rect.bottomRight()));
        painter->drawPath(circle);
        rect.setRight(rect.right() - kTagDiameter / 2);
    }

    painter->setPen(pen);
    painter->setBrush(brush);
    painter->setRenderHint(QPainter::Antialiasing, antialiasing);
}

SideBar::ItemInfo TagHelper::createSidebarItemInfo(const QString &tag)
{
    SideBar::ItemInfo item;
    item.group = SideBar::DefaultGroup::kTag;

    QUrl tagUrl;
    tagUrl.setScheme(TagManager::scheme());
    tagUrl.setPath("/" + tag);

    item.url = tagUrl;
    item.iconName = TagManager::instance()->getTagIconName(tag);
    item.text = tag;
    item.flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
    item.renameCb = TagManager::renameHandle;
    item.contextMenuCb = TagManager::contenxtMenuHandle;

    return item;
}

void TagHelper::showTagEdit(const QRectF &parentRect, const QRectF &iconRect, const QList<QUrl> &fileList)
{
    TagEditor *editor = new TagEditor();

    editor->setBaseSize(160, 98);
    editor->setFilesForTagging(fileList);
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->setFocusOutSelfClosing(true);

    QList<QString> sameTagsInDiffFiles = TagManager::instance()->getTagsByUrls(fileList);
    editor->setDefaultCrumbs(sameTagsInDiffFiles);

    int showPosX = static_cast<int>(iconRect.center().x());
    int showPosY = static_cast<int>(iconRect.bottom());

    auto subValue = parentRect.bottom() - showPosY;
    if (subValue < 98) {
        editor->setArrowDirection(DArrowRectangle::ArrowDirection::ArrowBottom);
        showPosY = qMin(static_cast<int>(parentRect.bottom()), showPosY);
    }

    editor->show(showPosX, showPosY);
}

QUrl TagHelper::redirectTagUrl(const QUrl &url)
{
    if (url.fragment().isEmpty())
        return url;

    return QUrl::fromLocalFile(url.fragment(QUrl::FullyDecoded));
}

TitleBarService *TagHelper::titleServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::TitleBarService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::TitleBarService>(DSB_FM_NAMESPACE::TitleBarService::name());
}

SideBarService *TagHelper::sideBarServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::SideBarService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::SideBarService>(DSB_FM_NAMESPACE::SideBarService::name());
}

WorkspaceService *TagHelper::workspaceServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::WorkspaceService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::WorkspaceService>(DSB_FM_NAMESPACE::WorkspaceService::name());
}

FileOperationsService *TagHelper::fileOperationsServIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSC_NAMESPACE::FileOperationsService::name()))
            abort();
    });

    return ctx.service<DSC_NAMESPACE::FileOperationsService>(DSC_NAMESPACE::FileOperationsService::name());
}

void TagHelper::initTagColorDefines()
{
    colorDefines << TagColorDefine("Orange", "dfm_tag_orange", QObject::tr("Orange"), "#ffa503")
                 << TagColorDefine("Red", "dfm_tag_red", QObject::tr("Red"), "#ff1c49")
                 << TagColorDefine("Purple", "dfm_tag_purple", QObject::tr("Purple"), "#9023fc")
                 << TagColorDefine("Navy-blue", "dfm_tag_deepblue", QObject::tr("Navy-blue"), "#3468ff")
                 << TagColorDefine("Azure", "dfm_tag_lightblue", QObject::tr("Azure"), "#00b5ff")
                 << TagColorDefine("Grass-green", "dfm_tag_green", QObject::tr("Green"), "#58df0a")
                 << TagColorDefine("Yellow", "dfm_tag_yellow", QObject::tr("Yellow"), "#fef144")
                 << TagColorDefine("Gray", "dfm_tag_gray", QObject::tr("Gray"), "#cccccc");
}

TagColorDefine TagHelper::randomTagDefine() const
{
    std::random_device device {};

    // Choose a random index in colorDefines
    std::default_random_engine engine(device());
    std::uniform_int_distribution<int> uniform_dist(0, colorDefines.length() - 1);

    return colorDefines.at(uniform_dist(engine));
}

TagColorDefine::TagColorDefine(const QString &colorName, const QString &iconName, const QString &dispaly, const QColor &color)
    : colorName(colorName),
      iconName(iconName),
      displayName(dispaly),
      color(color)
{
}
