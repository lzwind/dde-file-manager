// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GROUPEDMODELDATA_H
#define GROUPEDMODELDATA_H

#include "dfmplugin_workspace_global.h"

#include "groups/filegroupdata.h"
#include "groups/modelitemwrapper.h"

#include <QList>
#include <QHash>
#include <QString>
#include <QMutex>

#include <optional>

DPWORKSPACE_BEGIN_NAMESPACE

/**
 * @brief Container for grouped model data
 *
 * This class manages the flattened representation of grouped files
 * for use in the model-view architecture.
 */
class GroupedModelData
{
public:
    /**
     * @brief Default constructor
     */
    GroupedModelData();

    /**
     * @brief Copy constructor
     */
    GroupedModelData(const GroupedModelData &other);

    /**
     * @brief Assignment operator
     */
    GroupedModelData &operator=(const GroupedModelData &other);

    /**
     * @brief Destructor
     */
    ~GroupedModelData();

    // Core data members
    QList<FileGroupData> groups;   ///< All group data
    QHash<QString, bool> groupExpansionStates;   ///< Group expansion state mapping

    /**
     * @brief Get all files from all groups
     * @return List of all file data pointers
     */
    QList<FileItemDataPointer> getAllFiles() const;

    /**
     * @brief Set the expansion state of a group
     * @param groupKey The group identifier
     * @param expanded True to expand, false to collapse
     */
    void setGroupExpanded(const QString &groupKey, bool expanded);

    /**
     * @brief Check if a group is expanded
     * @param groupKey The group identifier
     * @return True if the group is expanded, false otherwise
     */
    bool isGroupExpanded(const QString &groupKey) const;

    /**
     * @brief Rebuild the flattened items list
     *
     * This method reconstructs the flattened list based on current
     * groups and their expansion states.
     */
    void rebuildFlattenedItems();

    /**
     * @brief Update a specific group header in flattened items
     * 
     * This method updates the group header information in flattened items
     * without rebuilding the entire list.
     * @param groupKey The group identifier
     */
    void updateGroupHeader(const QString &groupKey);

    /**
     * @brief Clear all data
     */
    void clear();

    /**
     * @brief Check if the data is empty
     * @return True if there are no groups, false otherwise
     */
    bool isEmpty() const;

    /**
     * @brief Add a new group to the model data
     * @param group The group data to add
     * @return True if the group was added successfully, false if a group with the same key already exists
     */
    bool addGroup(const FileGroupData &group);

    /**
     * @brief Remove a group from the model data
     * @param groupKey The group identifier to remove
     * @return True if the group was removed successfully, false if the group was not found
     */
    bool removeGroup(const QString &groupKey);

    /**
     * @brief Insert an item to the flattened items list
     * @param index The position where to insert the item
     * @param item The item to insert
     */
    void insertItem(int index, const ModelItemWrapper &item);

    /**
     * @brief Remove items from the flattened items list
     * @param index The position from where to start removing items
     * @param count The number of items to remove
     * @return The number of items actually removed
     */
    int removeItems(int index, int count);

    /**
     * @brief Replace an item in the flattened items list
     * @param index The position of the item to replace
     * @param item The new item
     */
    void replaceItem(int index, const ModelItemWrapper &item);

    /**
     * @brief Get a group by its key
     * @param groupKey The group identifier
     * @return Pointer to the group data, or nullptr if not found
     */
    FileGroupData *getGroup(const QString &groupKey);

    /**
     * @brief Get a group by its key (const version)
     * @param groupKey The group identifier
     * @return Const pointer to the group data, or nullptr if not found
     */
    const FileGroupData *getGroup(const QString &groupKey) const;

    /**
     * @brief Thread-safe access to item count
     * @return Total item count in the flattened list
     */
    int getItemCount() const;

    /**
     * @brief Thread-safe access to file item count
     * @return Total file item count in the flattened list
     */
    int getFileItemCount() const;

    /**
     * @brief Thread-safe access to item at index
     * @param index The index in the flattened list
     * @return The model item wrapper at the specified index
     */
    ModelItemWrapper getItemAt(int index) const;

    /**
     * @brief Find the start position of a group header
     * @param key The group identifier
     * @return The start position of the group header, or std::nullopt if not found
     */
    std::optional<int> findGroupHeaderStartPos(const QString &key) const;

    /**
     * @brief Find the start position of a file
     * @param url The file URL
     * @return The start position of the file, or std::nullopt if not found
     */
    std::optional<int> findFileStartPos(const QUrl &url) const;

private:
    mutable QMutex m_mutex;   ///< Mutex for thread safety
    QList<ModelItemWrapper> flattenedItems;   ///< Flattened model items list
};

DPWORKSPACE_END_NAMESPACE

#endif   // GROUPEDMODELDATA_H