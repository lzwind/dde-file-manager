/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "filecopymovejob.h"

#include <dfm-framework/framework.h>

#include <QUrl>

/*!
 * 特殊说明，调用当前服务的5个服务时，返回的JobHandlePointer时就已经启动了线程去处理相应的任务，那么使用JobHandlePointer做信号链接时，
 * 错误信息可能信号已发送了，只能使用JobHandlePointer去判断是否有错误，并做相应的处理
 */
DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE
FileCopyMoveJob::FileCopyMoveJob(QObject *parent)
    : QObject(parent)
{
    copyMoveTaskMutex.reset(new QMutex);
    getOperationsAndDialogServiceMutex.reset(new QMutex);
}

bool FileCopyMoveJob::getOperationsAndDialogService()
{
    QMutexLocker lk(getOperationsAndDialogServiceMutex.data());
    if (operationsService.isNull()) {
        auto &ctx = DPF_NAMESPACE::Framework::instance().serviceContext();
        operationsService = ctx.service<DSC_NAMESPACE::FileOperationsService>(DSC_NAMESPACE::FileOperationsService::name());
        if (!operationsService) {
            QString errStr;
            if (!ctx.load(DSC_NAMESPACE::FileOperationsService::name(), &errStr)) {
                qCritical() << errStr;
                abort();
            }
            operationsService = ctx.service<DSC_NAMESPACE::FileOperationsService>(DSC_NAMESPACE::FileOperationsService::name());
        }
    }
    if (dialogService.isNull()) {
        auto &ctx = DPF_NAMESPACE::Framework::instance().serviceContext();
        dialogService = ctx.service<DSC_NAMESPACE::DialogService>(DSC_NAMESPACE::DialogService::name());
        if (!dialogService) {
            QString errStr;
            if (!ctx.load(DSC_NAMESPACE::DialogService::name(), &errStr)) {
                qCritical() << errStr;
                abort();
            }
            dialogService = ctx.service<DSC_NAMESPACE::DialogService>(DSC_NAMESPACE::DialogService::name());
        }
    }
    return operationsService && dialogService;
}

void FileCopyMoveJob::onHandleAddTask()
{
    QMutexLocker lk(copyMoveTaskMutex.data());
    QObject *send = sender();
    JobHandlePointer jobHandler = send->property("jobPointer").value<JobHandlePointer>();
    send->setProperty("jobPointer", QVariant());
    if (!getOperationsAndDialogService()) {
        qCritical() << "get service fialed !!!!!!!!!!!!!!!!!!!";
        return;
    }
    dialogService->addTask(jobHandler);
    jobHandler->disconnect(jobHandler.data(), &AbstractJobHandler::finishedNotify, this, &FileCopyMoveJob::onHandleTaskFinished);
}

void FileCopyMoveJob::onHandleAddTaskWithArgs(const JobInfoPointer info)
{
    QMutexLocker lk(copyMoveTaskMutex.data());

    JobHandlePointer jobHandler = info->value(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer).value<JobHandlePointer>();
    if (!getOperationsAndDialogService()) {
        qCritical() << "get service fialed !!!!!!!!!!!!!!!!!!!";
        return;
    }

    copyMoveTask.value(jobHandler)->stop();
    dialogService->addTask(jobHandler);
}

void FileCopyMoveJob::onHandleTaskFinished(const JobInfoPointer info)
{
    JobHandlePointer jobHandler = info->value(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer).value<JobHandlePointer>();
    {
        QMutexLocker lk(copyMoveTaskMutex.data());
        copyMoveTask.remove(jobHandler);
    }
}

void FileCopyMoveJob::initArguments(const JobHandlePointer &handler)
{
    QSharedPointer<QTimer> timer(new QTimer);
    timer->setSingleShot(true);
    timer->setInterval(1000);
    timer->connect(timer.data(), &QTimer::timeout, this, &FileCopyMoveJob::onHandleAddTask);
    handler->connect(handler.data(), &AbstractJobHandler::errorNotify, this, &FileCopyMoveJob::onHandleAddTaskWithArgs);
    handler->connect(handler.data(), &AbstractJobHandler::finishedNotify, this, &FileCopyMoveJob::onHandleTaskFinished);
    timer->setProperty("jobPointer", QVariant::fromValue(handler));
    {
        QMutexLocker lk(copyMoveTaskMutex.data());
        copyMoveTask.insert(handler, timer);
    }
    timer->start();
    handler->start();
}
/*!
 * \brief FileOperationsUtils::copyFiles 拷贝文件有UI界面
 *  这个接口只能拷贝源文件在同一目录的文件，也就是源文件都是同一个设备上的文件。拷贝根据源文件的scheme来进行创建不同的file
 *  执行不同scheme的read、write和同步
 *  如果出入的scheme是自定义的（dfm-io不支持的scheme），那么拷贝任务执行拷贝时使用的read和write都使用dfm-io提供的默认操作
 * \param sources 源文件的列表
 * \param target 目标目录
 * \return QSharedPointer<AbstractJobHandler> 任务控制器
 */
JobHandlePointer FileCopyMoveJob::paste(const QList<QUrl> &sources, const QUrl &target,
                                        const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags)
{
    if (!getOperationsAndDialogService()) {
        qCritical() << "get service fialed !!!!!!!!!!!!!!!!!!!";
        return nullptr;
    }

    JobHandlePointer jobHandle = operationsService->paste(sources, target, flags);
    initArguments(jobHandle);

    return jobHandle;
}
/*!
 * \brief FileOperationsUtils::moveFilesToTrash 移动文件到回收站
 *  一个移动到回收站的任务的源文件都是在同一目录下，所以源文件都是在同一个设备上。移动到回收站只能是不可移除设备
 * （系统盘，或者安装系统时挂载的机械硬盘），如果文件的大小操过1g（目前设计是这样）不能放入回收站，只能删除
 * \param sources 移动到回收站的源文件
 * \return JobHandlePointer 任务控制器
 */
JobHandlePointer FileCopyMoveJob::moveToTrash(const QList<QUrl> &sources,
                                              const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags)
{
    if (!getOperationsAndDialogService()) {
        qCritical() << "get service fialed !!!!!!!!!!!!!!!!!!!";
        return nullptr;
    }

    JobHandlePointer jobHandle = operationsService->moveToTrash(sources, flags);
    initArguments(jobHandle);

    return jobHandle;
}
/*!
 * \brief FileOperationsUtils::restoreFilesFromTrash
 * 从回收站还原文件，原目录下有相同文件，提示替换或者共存
 * \param sources 需要还原的文件
 * \return JobHandlePointer 任务控制器
 */
JobHandlePointer FileCopyMoveJob::restoreFromTrash(const QList<QUrl> &sources,
                                                   const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags)
{
    if (!getOperationsAndDialogService()) {
        qCritical() << "get service fialed !!!!!!!!!!!!!!!!!!!";
        return nullptr;
    }

    JobHandlePointer jobHandle = operationsService->restoreFromTrash(sources, flags);
    initArguments(jobHandle);

    return jobHandle;
}
/*!
 * \brief FileOperationsUtils::deleteFiles 删除文件
 * 一个删除的任务的源文件都是在同一目录下，所以源文件都是在同一个设备上。
 * \param sources 需要删除的源文件
 * \return JobHandlePointer 任务控制器
 */
JobHandlePointer FileCopyMoveJob::deletes(const QList<QUrl> &sources,
                                          const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags)
{
    if (!getOperationsAndDialogService()) {
        qCritical() << "get service fialed !!!!!!!!!!!!!!!!!!!";
        return nullptr;
    }

    JobHandlePointer jobHandle = operationsService->deletes(sources, flags);
    initArguments(jobHandle);

    return jobHandle;
}
/*!
 * \brief FileOperationsUtils::cutFiles 剪切文件
 * 一个剪切的任务的源文件都是在同一目录下，所以源文件都是在同一个设备上。判断源文件和目标文件是否在同一个设备上
 * 在同一个目录上执行dorename，如果dorename失败或者不在同一个设备上就执行先拷贝，在剪切
 * \param sources 源文件的列表
 * \param target 目标目录
 * \return JobHandlePointer 任务控制器
 */
JobHandlePointer FileCopyMoveJob::cut(const QList<QUrl> &sources, const QUrl &target,
                                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags)
{
    if (!getOperationsAndDialogService()) {
        qCritical() << "get service fialed !!!!!!!!!!!!!!!!!!!";
        return nullptr;
    }

    JobHandlePointer jobHandle = operationsService->cut(sources, target, flags);
    initArguments(jobHandle);

    return jobHandle;
}
