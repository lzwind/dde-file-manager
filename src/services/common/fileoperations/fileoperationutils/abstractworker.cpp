/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#include "abstractworker.h"

#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/application/application.h"

#include <dfm-framework/framework.h>

#include <dfm-io/dfmio_utils.h>

#include <QUrl>
#include <QWaitCondition>
#include <QMutex>
#include <QApplication>
#include <QStorageInfo>
#include <QRegularExpression>
#include <QDebug>

using namespace dfm_service_common;

/*!
 * \brief setWorkArgs 设置当前任务的参数
 * \param args 参数
 */
void AbstractWorker::setWorkArgs(const JobHandlePointer handle, const QList<QUrl> &sources, const QUrl &target,
                                 const AbstractJobHandler::JobFlags &flags)
{
    if (!handle) {
        qWarning() << "JobHandlePointer is a nullptr, setWorkArgs failed!";
        return;
    }
    this->handle = handle;
    initHandleConnects(handle);
    this->sourceUrls = sources;
    this->targetUrl = target;
    isConvert = flags.testFlag(DFMBASE_NAMESPACE::AbstractJobHandler::JobFlag::kRevocation);
    jobFlags = flags;
}

/*!
 * \brief doOperateWork 处理用户的操作 不在拷贝线程执行的函数，协同类直接调用
 * \param actions 当前操作
 */
void AbstractWorker::doOperateWork(AbstractJobHandler::SupportActions actions)
{
    if (actions.testFlag(AbstractJobHandler::SupportAction::kStopAction)) {
        stop();
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kPauseAction)) {
        pause();
    } else if (actions.testFlag(AbstractJobHandler::SupportAction::kResumAction)) {
        resume();
    } else {
        if (actions.testFlag(AbstractJobHandler::SupportAction::kCancelAction)) {
            currentAction = AbstractJobHandler::SupportAction::kCancelAction;
        } else if (actions.testFlag(AbstractJobHandler::SupportAction::kCoexistAction)) {
            currentAction = AbstractJobHandler::SupportAction::kCoexistAction;
        } else if (actions.testFlag(AbstractJobHandler::SupportAction::kSkipAction)) {
            currentAction = AbstractJobHandler::SupportAction::kSkipAction;
        } else if (actions.testFlag(AbstractJobHandler::SupportAction::kMergeAction)) {
            currentAction = AbstractJobHandler::SupportAction::kMergeAction;
        } else if (actions.testFlag(AbstractJobHandler::SupportAction::kReplaceAction)) {
            currentAction = AbstractJobHandler::SupportAction::kReplaceAction;
        } else if (actions.testFlag(AbstractJobHandler::SupportAction::kRetryAction)) {
            currentAction = AbstractJobHandler::SupportAction::kRetryAction;
        } else if (actions.testFlag(AbstractJobHandler::SupportAction::kEnforceAction)) {
            currentAction = AbstractJobHandler::SupportAction::kEnforceAction;
        } else {
            currentAction = AbstractJobHandler::SupportAction::kNoAction;
        }

        rememberSelect.store(actions.testFlag(AbstractJobHandler::SupportAction::kRememberAction));

        handlingErrorCondition.wakeAll();
    }
}

/*!
 * \brief AbstractWorker::stop stop task
 */
void AbstractWorker::stop()
{
    setStat(AbstractJobHandler::JobState::kStopState);
    if (statisticsFilesSizeJob)
        statisticsFilesSizeJob->stop();
    // clean error info queue
    {
        QMutexLocker lk(&errorThreadIdQueueMutex);
        errorThreadIdQueue.clear();
    }

    waitCondition.wakeAll();

    handlingErrorCondition.wakeAll();

    errorCondition.wakeAll();

    if (updateProgressTimer)
        updateProgressTimer->stopTimer();

    if (updateProgressThread) {
        updateProgressThread->quit();
        updateProgressThread->wait();
    }
}
/*!
 * \brief AbstractWorker::pause paused task
 */
void AbstractWorker::pause()
{
    if (currentState == AbstractJobHandler::JobState::kPauseState)
        return;
    setStat(AbstractJobHandler::JobState::kPauseState);
}
/*!
 * \brief AbstractWorker::resume resume task
 */
void AbstractWorker::resume()
{
    setStat(AbstractJobHandler::JobState::kRunningState);

    waitCondition.wakeAll();

    errorCondition.wakeAll();
}
/*!
 * \brief AbstractWorker::startCountProccess start update proccess timer
 */
void AbstractWorker::startCountProccess()
{
    if (!updateProgressTimer)
        updateProgressTimer.reset(new UpdateProgressTimer());
    if (!updateProgressThread)
        updateProgressThread.reset(new QThread);
    updateProgressTimer->moveToThread(updateProgressThread.data());
    updateProgressThread->start();
    connect(this, &AbstractWorker::startUpdateProgressTimer, updateProgressTimer.data(), &UpdateProgressTimer::doStartTime);
    connect(updateProgressTimer.data(), &UpdateProgressTimer::updateProgressNotify, this, &AbstractWorker::onUpdateProgress, Qt::DirectConnection);
    emit startUpdateProgressTimer();
}
/*!
 * \brief AbstractWorker::statisticsFilesSize statistics source files size
 * \return
 */
bool AbstractWorker::statisticsFilesSize()
{
    if (sourceUrls.isEmpty()) {
        qWarning() << "sources files list is empty!";
        return false;
    }

    const QUrl &firstUrl = sourceUrls.first();

    // 判读源文件所在设备位置，执行异步或者同统计源文件大小
    isSourceFileLocal = FileOperationsUtils::isFileOnDisk(firstUrl);

    if (isSourceFileLocal) {
        const QString &fsType = DFMIO::DFMUtils::fsTypeFromUrl(firstUrl);
        isSourceFileLocal = fsType.startsWith("ext");
    }

    if (isSourceFileLocal) {
        const SizeInfoPointer &fileSizeInfo = FileOperationsUtils::statisticsFilesSize(sourceUrls, true);

        allFilesList = fileSizeInfo->allFiles;
        sourceFilesTotalSize = fileSizeInfo->totalSize;
        dirSize = fileSizeInfo->dirSize;
        sourceFilesCount = fileSizeInfo->fileCount;
    } else {
        statisticsFilesSizeJob.reset(new DFMBASE_NAMESPACE::FileStatisticsJob());
        connect(statisticsFilesSizeJob.data(), &DFMBASE_NAMESPACE::FileStatisticsJob::finished, this, &AbstractWorker::onStatisticsFilesSizeFinish, Qt::DirectConnection);
        connect(statisticsFilesSizeJob.data(), &DFMBASE_NAMESPACE::FileStatisticsJob::sizeChanged, this, &AbstractWorker::onStatisticsFilesSizeUpdate, Qt::DirectConnection);
        statisticsFilesSizeJob->start(sourceUrls);
    }
    return true;
}
/*!
 * \brief AbstractWorker::copyWait Blocking waiting for task
 * \return Is it running
 */
bool AbstractWorker::workerWait()
{
    QMutex lock;
    waitCondition.wait(&lock);
    lock.unlock();

    return currentState == AbstractJobHandler::JobState::kRunningState;
}
/*!
 * \brief AbstractWorker::setStat Set current task status
 * \param stat task status
 */
void AbstractWorker::setStat(const AbstractJobHandler::JobState &stat)
{
    if (stat == AbstractJobHandler::JobState::kRunningState)
        waitCondition.wakeAll();

    if (stat == currentState)
        return;

    currentState = stat;

    emitStateChangedNotify();
}
/*!
 * \brief AbstractWorker::initArgs init job agruments
 * \return
 */
bool AbstractWorker::initArgs()
{
    sourceFilesTotalSize = 0;
    setStat(AbstractJobHandler::JobState::kRunningState);
    if (!handler)
        handler.reset(new LocalFileHandler);
    completeSourceFiles.clear();
    completeTargetFiles.clear();
    completeCustomInfos.clear();

    return true;
}
/*!
 * \brief AbstractWorker::endWork end task and emit task finished
 */
void AbstractWorker::endWork()
{
    setStat(AbstractJobHandler::JobState::kStopState);

    // send finish signal
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteFilesKey, QVariant::fromValue(completeSourceFiles));
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteTargetFilesKey, QVariant::fromValue(completeTargetFiles));
    info->insert(AbstractJobHandler::NotifyInfoKey::kCompleteCustomInfosKey, QVariant::fromValue(completeCustomInfos));
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(handle));

    saveOperations();

    emit finishedNotify(info);

    qDebug() << "\n=========================nwork end, job: " << jobType << " sources: " << sourceUrls << " target: " << targetUrl << " time elapsed: " << timeElapsed.elapsed() << "\n";
}
/*!
 * \brief AbstractWorker::emitStateChangedNotify send state changed signal
 */
void AbstractWorker::emitStateChangedNotify()
{
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobStateKey, QVariant::fromValue(currentState));

    emit stateChangedNotify(info);
}
/*!
 * \brief AbstractWorker::emitCurrentTaskNotify send current task information
 * \param from source url
 * \param to target url
 */
void AbstractWorker::emitCurrentTaskNotify(const QUrl &from, const QUrl &to)
{
    JobInfoPointer info = createCopyJobInfo(from, to);

    emit currentTaskNotify(info);
}
/*!
 * \brief AbstractWorker::emitProgressChangedNotify send process changed signal
 * \param writSize task complete data size
 */
void AbstractWorker::emitProgressChangedNotify(const qint64 &writSize)
{
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
    if (AbstractJobHandler::JobType::kCopyType == jobType
        || AbstractJobHandler::JobType::kCutType == jobType) {
        info->insert(AbstractJobHandler::NotifyInfoKey::kTotalSizeKey, QVariant::fromValue(qint64(sourceFilesTotalSize)));
    } else {
        info->insert(AbstractJobHandler::NotifyInfoKey::kTotalSizeKey, QVariant::fromValue(qint64(sourceFilesCount)));
    }
    AbstractJobHandler::StatisticState state = AbstractJobHandler::StatisticState::kNoState;
    if (statisticsFilesSizeJob) {
        if (statisticsFilesSizeJob->isFinished())
            state = AbstractJobHandler::StatisticState::kStopState;
        else
            state = AbstractJobHandler::StatisticState::kRunningState;
    }
    info->insert(AbstractJobHandler::NotifyInfoKey::kStatisticStateKey, QVariant::fromValue(state));

    info->insert(AbstractJobHandler::NotifyInfoKey::kCurrentProgressKey, QVariant::fromValue(writSize));

    emit progressChangedNotify(info);
}
/*!
 * \brief AbstractWorker::emitErrorNotify send job error signal
 * \param from source url
 * \param to target url
 * \param error task error type
 * \param errorMsg task error message
 */
void AbstractWorker::emitErrorNotify(const QUrl &from, const QUrl &to, const AbstractJobHandler::JobErrorType &error, const QString &errorMsg)
{
    JobInfoPointer info = createCopyJobInfo(from, to);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobHandlePointer, QVariant::fromValue(handle));
    info->insert(AbstractJobHandler::NotifyInfoKey::kErrorTypeKey, QVariant::fromValue(error));
    info->insert(AbstractJobHandler::NotifyInfoKey::kErrorMsgKey, QVariant::fromValue(errorMsg));
    info->insert(AbstractJobHandler::NotifyInfoKey::kActionsKey, QVariant::fromValue(supportActions(error)));
    info->insert(AbstractJobHandler::NotifyInfoKey::kSourceUrlKey, QVariant::fromValue(from));
    emit errorNotify(info);

    qDebug() << "work error, job: " << jobType << " job error: " << error << " url from: " << from << " url to: " << to
             << " error msg: " << errorMsg;
}

AbstractJobHandler::SupportActions AbstractWorker::supportActions(const AbstractJobHandler::JobErrorType &error)
{
    AbstractJobHandler::SupportActions support = AbstractJobHandler::SupportAction::kCancelAction;
    switch (error) {
    case AbstractJobHandler::JobErrorType::kPermissionError:
    case AbstractJobHandler::JobErrorType::kOpenError:
    case AbstractJobHandler::JobErrorType::kReadError:
    case AbstractJobHandler::JobErrorType::kWriteError:
    case AbstractJobHandler::JobErrorType::kSymlinkError:
    case AbstractJobHandler::JobErrorType::kMkdirError:
    case AbstractJobHandler::JobErrorType::kResizeError:
    case AbstractJobHandler::JobErrorType::kRemoveError:
    case AbstractJobHandler::JobErrorType::kRenameError:
    case AbstractJobHandler::JobErrorType::kIntegrityCheckingError:
    case AbstractJobHandler::JobErrorType::kNonexistenceError:
    case AbstractJobHandler::JobErrorType::kDeleteFileError:
    case AbstractJobHandler::JobErrorType::kDeleteTrashFileError:
    case AbstractJobHandler::JobErrorType::kNoSourceError:
    case AbstractJobHandler::JobErrorType::kCancelError:
    case AbstractJobHandler::JobErrorType::kUnknowUrlError:
    case AbstractJobHandler::JobErrorType::kNotSupportedError:
    case AbstractJobHandler::JobErrorType::kPermissionDeniedError:
    case AbstractJobHandler::JobErrorType::kSeekError:
    case AbstractJobHandler::JobErrorType::kProrogramError:
    case AbstractJobHandler::JobErrorType::kDfmIoError:
    case AbstractJobHandler::JobErrorType::kMakeStandardTrashError:
    case AbstractJobHandler::JobErrorType::kGetRestorePathError:
    case AbstractJobHandler::JobErrorType::kIsNotTrashFileError:
    case AbstractJobHandler::JobErrorType::kCreateParentDirError:
    case AbstractJobHandler::JobErrorType::kUnknowError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kRetryAction;
    case AbstractJobHandler::JobErrorType::kSpecialFileError:
        return AbstractJobHandler::SupportAction::kSkipAction;
    case AbstractJobHandler::JobErrorType::kFileSizeTooBigError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kNotEnoughSpaceError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kRetryAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kFileExistsError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kReplaceAction | AbstractJobHandler::SupportAction::kCoexistAction;
    case AbstractJobHandler::JobErrorType::kDirectoryExistsError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kMergeAction | AbstractJobHandler::SupportAction::kCoexistAction;
    case AbstractJobHandler::JobErrorType::kTargetReadOnlyError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kTargetIsSelfError:
        return support | AbstractJobHandler::SupportAction::kSkipAction | AbstractJobHandler::SupportAction::kEnforceAction;
    case AbstractJobHandler::JobErrorType::kSymlinkToGvfsError:
        return support | AbstractJobHandler::SupportAction::kSkipAction;
    default:
        break;
    }

    return support;
}
/*!
 * \brief AbstractWorker::isStopped current task is stopped
 * \return current task is stopped
 */
bool AbstractWorker::isStopped()
{
    return AbstractJobHandler::JobState::kStopState == currentState;
}
/*!
 * \brief AbstractWorker::createCopyJobInfo create signal agruments information
 * \param from source url
 * \param to from url
 * \return signal agruments information
 */
JobInfoPointer AbstractWorker::createCopyJobInfo(const QUrl &from, const QUrl &to)
{
    JobInfoPointer info(new QMap<quint8, QVariant>);
    info->insert(AbstractJobHandler::NotifyInfoKey::kJobtypeKey, QVariant::fromValue(jobType));
    info->insert(AbstractJobHandler::NotifyInfoKey::kSourceUrlKey, QVariant::fromValue(from));
    info->insert(AbstractJobHandler::NotifyInfoKey::kTargetUrlKey, QVariant::fromValue(to));
    QString fromMsg, toMsg;
    if (AbstractJobHandler::JobType::kCopyType == jobType) {
        fromMsg = QString(QObject::tr("copy file %1")).arg(from.path());
        toMsg = QString(QObject::tr("to %1")).arg(to.path());
    } else if (AbstractJobHandler::JobType::kDeleteTpye == jobType) {
        fromMsg = QString(QObject::tr("delete file %1")).arg(from.path());
    } else if (AbstractJobHandler::JobType::kCutType == jobType) {
        fromMsg = QString(QObject::tr("cut file %1")).arg(from.path());
        toMsg = QString(QObject::tr("to %1")).arg(to.path());
    } else if (AbstractJobHandler::JobType::kMoveToTrashType == jobType) {
        fromMsg = QString(QObject::tr("move file %1")).arg(from.path());
        toMsg = QString(QObject::tr("to trash %1")).arg(to.path());
    } else if (AbstractJobHandler::JobType::kRestoreType == jobType) {
        fromMsg = QString(QObject::tr("restore file %1 from trash ")).arg(from.path());
        toMsg = QString(QObject::tr("to %1")).arg(to.path());
    } else if (AbstractJobHandler::JobType::kCleanTrashType == jobType) {
        fromMsg = QString(QObject::tr("clean trash file %1  ")).arg(from.path());
    }
    info->insert(AbstractJobHandler::NotifyInfoKey::kSourceMsgKey, QVariant::fromValue(fromMsg));
    info->insert(AbstractJobHandler::NotifyInfoKey::kTargetMsgKey, QVariant::fromValue(toMsg));
    return info;
}
/*!
 * \brief AbstractWorker::doWork task Thread execution
 * \return
 */
bool AbstractWorker::doWork()
{
    timeElapsed.start();
    qDebug() << "\n=========================\nwork begin, job: " << jobType << " sources: " << sourceUrls << " target: " << targetUrl << "\n";

    // 执行拷贝的业务逻辑
    if (!initArgs()) {
        endWork();
        return false;
    }
    // 统计文件总大小
    if (!statisticsFilesSize()) {
        endWork();
        return false;
    }
    // 启动统计写入数据大小计时器
    startCountProccess();

    return true;
}
/*!
 * \brief AbstractWorker::stateCheck Blocking waiting for task and check status
 * \return is Correct state
 */
bool AbstractWorker::stateCheck()
{
    if (currentState == AbstractJobHandler::JobState::kRunningState) {
        return true;
    }
    if (currentState == AbstractJobHandler::JobState::kPauseState) {
        qInfo() << "Will be suspended";
        if (!workerWait()) {
            return currentState != AbstractJobHandler::JobState::kStopState;
        }
    } else if (currentState == AbstractJobHandler::JobState::kStopState) {
        return false;
    }

    return true;
}
/*!
 * \brief AbstractWorker::onStatisticsFilesSizeFinish  Count the size of all files
 * and the slot at the end of the thread
 * \param sizeInfo All file size information
 */
void AbstractWorker::onStatisticsFilesSizeFinish()
{
    statisticsFilesSizeJob->stop();
    const SizeInfoPointer &sizeInfo = statisticsFilesSizeJob->getFileSizeInfo();
    sourceFilesTotalSize = statisticsFilesSizeJob->totalProgressSize();
    dirSize = sizeInfo->dirSize;
    sourceFilesCount = sizeInfo->fileCount;
}

void AbstractWorker::onStatisticsFilesSizeUpdate(qint64 size)
{
    sourceFilesTotalSize = size;
}

AbstractWorker::AbstractWorker(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<DFMBASE_NAMESPACE::AbstractJobHandler::ShowDialogType>();
}
/*!
 * \brief AbstractWorker::formatFileName Processing and formatting file names
 * \param fileName file name
 * \return format file name
 */
QString AbstractWorker::formatFileName(const QString &fileName)
{
    // 获取目标文件的文件系统，是vfat格式是否要特殊处理，以前的文管处理的
    if (jobFlags.testFlag(AbstractJobHandler::JobFlag::kDontFormatFileName)) {
        return fileName;
    }

    const QString &fs_type = QStorageInfo(targetUrl.path()).fileSystemType();

    if (fs_type == "vfat") {
        QString new_name = fileName;

        return new_name.replace(QRegExp("[\"*:<>?\\|]"), "_");
    }

    return fileName;
}

void AbstractWorker::saveOperations()
{
    if (!isConvert && !completeTargetFiles.isEmpty()) {
        // send saveoperator event
        if (jobType == AbstractJobHandler::JobType::kCopyType
            || jobType == AbstractJobHandler::JobType::kCutType
            || jobType == AbstractJobHandler::JobType::kMoveToTrashType
            || jobType == AbstractJobHandler::JobType::kRestoreType) {
            GlobalEventType operatorType = GlobalEventType::kDeleteFiles;
            QUrl targetUrl;
            switch (jobType) {
            case AbstractJobHandler::JobType::kCopyType:
                operatorType = GlobalEventType::kDeleteFiles;
                targetUrl = UrlRoute::urlParent(completeSourceFiles.first());
                break;
            case AbstractJobHandler::JobType::kCutType:
                operatorType = GlobalEventType::kCutFile;
                targetUrl = UrlRoute::urlParent(completeSourceFiles.first());
                break;
            case AbstractJobHandler::JobType::kMoveToTrashType:
                operatorType = GlobalEventType::kRestoreFromTrash;
                break;
            case AbstractJobHandler::JobType::kRestoreType:
                operatorType = GlobalEventType::kMoveToTrash;
                break;
            default:
                operatorType = GlobalEventType::kDeleteFiles;
                break;
            }
            QVariantMap values;
            values.insert("event", QVariant::fromValue(static_cast<uint16_t>(operatorType)));
            values.insert("sources", QUrl::toStringList(completeTargetFiles));
            values.insert("targets", QUrl::toStringList({ targetUrl }));
            if (jobType != AbstractJobHandler::JobType::kDeleteTpye)
                dpfSignalDispatcher->publish(GlobalEventType::kSaveOperator, values);
        }
    }
}

AbstractWorker::~AbstractWorker()
{
}

/*!
 * \brief AbstractWorker::initHandleConnects 初始化当前信号的连接
 * \param handle 任务控制处理器
 */
void AbstractWorker::initHandleConnects(const JobHandlePointer handle)
{
    if (!handle) {
        qWarning() << "JobHandlePointer is a nullptr,so connects failed!";
        return;
    }

    connect(handle.get(), &AbstractJobHandler::userAction, this, &AbstractWorker::doOperateWork, Qt::QueuedConnection);

    connect(this, &AbstractWorker::progressChangedNotify, handle.get(), &AbstractJobHandler::onProccessChanged, Qt::QueuedConnection);
    connect(this, &AbstractWorker::stateChangedNotify, handle.get(), &AbstractJobHandler::onStateChanged, Qt::QueuedConnection);
    connect(this, &AbstractWorker::currentTaskNotify, handle.get(), &AbstractJobHandler::onCurrentTask, Qt::QueuedConnection);
    connect(this, &AbstractWorker::finishedNotify, handle.get(), &AbstractJobHandler::onFinished, Qt::QueuedConnection);
    connect(this, &AbstractWorker::errorNotify, handle.get(), &AbstractJobHandler::onError, Qt::QueuedConnection);
    connect(this, &AbstractWorker::speedUpdatedNotify, handle.get(), &AbstractJobHandler::onSpeedUpdated, Qt::QueuedConnection);
}
