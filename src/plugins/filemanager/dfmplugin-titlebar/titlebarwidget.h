/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef TITLEBARWIDGET_H
#define TITLEBARWIDGET_H

#include "navwidget.h"
#include "addressbar.h"
#include "crumbbar.h"
#include "optionbuttonbox.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QToolButton>

class TitleBarWidget : public QFrame
{
    Q_OBJECT
public:
    explicit TitleBarWidget(QFrame *parent = nullptr);

private:
    void initializeUi();
    void initConnect();
    void showAddrsssBar();   // switch addrasssBar and crumbBar show
    void showCrumbBar();
    void showSearchButton();
    void showSearchFilterButton();
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onSearchButtonClicked();

private:
    QHBoxLayout *titleBarLayout { nullptr };   // 标题栏布局
    NavWidget *navWidget { nullptr };   // 导航小部件
    AddressBar *addressBar { nullptr };   // 地址编辑栏
    QToolButton *searchButton { nullptr };   // 搜索栏按钮
    QToolButton *searchFilterButton { nullptr };   // 搜索过滤按钮
    OptionButtonBox *optionButtonBox { nullptr };   // 功能按鈕栏
    CrumbBar *crumbBar { nullptr };   // 面包屑
};

#endif   // TITLEBARWIDGET_H
