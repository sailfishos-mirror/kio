/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KFILE_H
#define KFILE_H

#include <QDir>

#include "kiowidgets_export.h"

/*!
 * \class KFile
 * \inmodule KIOWidgets
 *
 * \brief KFile is a class which provides a namespace for some enumerated
 * values associated with the kfile library.  You will never need to
 * construct a KFile object itself.
 */

class KIOWIDGETS_EXPORT KFile
{
    Q_GADGET
public:
    /*!
     * Modes of operation for the dialog.
     * \value File Get a single file name from the user.
     * \value Directory Get a directory name from the user.
     * \value Files Get multiple file names from the user.
     * \value ExistingOnly Never return a filename which does not exist yet
     * \value LocalOnly Don't return remote filenames
     * \omitvalue ModeMax
     */
    enum Mode {
        File = 1,
        Directory = 2,
        Files = 4,
        ExistingOnly = 8,
        LocalOnly = 16,
        ModeMax = 65536,
    };
    Q_DECLARE_FLAGS(Modes, Mode)
    Q_FLAG(Modes)

    /*!
     * \value Default
     * \value Simple
     * \value Detail
     * \value SeparateDirs
     * \value PreviewContents
     * \value PreviewInfo
     * \value Tree
     * \value DetailTree
     * \omitvalue FileViewMax
     */
    enum FileView {
        Default = 0,
        Simple = 1,
        Detail = 2,
        SeparateDirs = 4,
        PreviewContents = 8,
        PreviewInfo = 16,
        Tree = 32,
        DetailTree = 64,
        FileViewMax = 65536,
    };

    /*!
     * \value Single
     * \value Multi
     * \value Extended
     * \value NoSelection
     */
    enum SelectionMode {
        Single = 1,
        Multi = 2,
        Extended = 4,
        NoSelection = 8,
    };

    //
    // some bittests
    //

    // sorting specific

    /*!
     *
     */
    static bool isSortByName(const QDir::SortFlags &sort);

    /*!
     *
     */
    static bool isSortBySize(const QDir::SortFlags &sort);

    /*!
     *
     */
    static bool isSortByDate(const QDir::SortFlags &sort);

    /*!
     *
     */
    static bool isSortByType(const QDir::SortFlags &sort);

    /*!
     *
     */
    static bool isSortDirsFirst(const QDir::SortFlags &sort);

    /*!
     *
     */
    static bool isSortCaseInsensitive(const QDir::SortFlags &sort);

    // view specific
    /*!
     *
     */
    static bool isDefaultView(const FileView &view);

    /*!
     *
     */
    static bool isSimpleView(const FileView &view);

    /*!
     *
     */
    static bool isDetailView(const FileView &view);

    /*!
     *
     */
    static bool isSeparateDirs(const FileView &view);

    /*!
     *
     */
    static bool isPreviewContents(const FileView &view);

    /*!
     *
     */
    static bool isPreviewInfo(const FileView &view);

    /*!
     *
     */
    static bool isTreeView(const FileView &view);

    /*!
     *
     */
    static bool isDetailTreeView(const FileView &view);

private:
    KFile() = delete;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KFile::Modes)

#endif // KFILE_H
