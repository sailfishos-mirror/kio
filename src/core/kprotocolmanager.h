/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2000 Waldo Bastain <bastain@kde.org>
    SPDX-FileCopyrightText: 2000 Dawit Alemayehu <adawit@kde.org>
    SPDX-FileCopyrightText: 2008 Jarosław Staniek <staniek@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KPROTOCOLMANAGER_H
#define KPROTOCOLMANAGER_H

#include <QStringList>

#include "kio/global.h" // KIO::CacheControl
#include "kiocore_export.h"
#include "kprotocolinfo.h"

class KSharedConfig;
template<class T>
class QExplicitlySharedDataPointer;
typedef QExplicitlySharedDataPointer<KSharedConfig> KSharedConfigPtr;
namespace KIO
{
class WorkerConfigPrivate;
} // namespace KIO

/*!
 * \class KProtocolManager
 * \inmodule KIOCore
 *
 * \brief Provides information about I/O (Internet, etc.) settings chosen/set
 * by the end user.
 *
 * KProtocolManager has a heap of static functions that allows only read
 * access to KDE's IO related settings. These include proxy, cache, file
 * transfer resumption, timeout and user-agent related settings.
 *
 * The information provided by this class is generic enough to be applicable
 * to any application that makes use of KDE's IO sub-system.  Note that this
 * mean the proxy, timeout etc. settings are saved in a separate user-specific
 * config file and not in the config file of the application.
 */
class KIOCORE_EXPORT KProtocolManager
{
public:
    /*=========================== TIMEOUT CONFIG ================================*/

#if KIOCORE_ENABLE_DEPRECATED_SINCE(6, 11)
    /*!
     * Returns the preferred timeout value for reading from
     * remote connections in seconds.
     *
     * Returns the timeout value for remote connection in secs.
     *
     * This is not used.
     *
     * \deprecated[6.11]
     */
    KIOCORE_DEPRECATED_VERSION(6, 11, "Not used")
    static int readTimeout();
#endif

#if KIOCORE_ENABLE_DEPRECATED_SINCE(6, 11)
    /*!
     * Returns the preferred timeout value for remote connections
     * in seconds.
     *
     * Returns the timeout value for remote connection in secs.
     *
     * This is not used.
     *
     * \deprecated[6.11]
     */
    KIOCORE_DEPRECATED_VERSION(6, 11, "Not used")
    static int connectTimeout();
#endif

#if KIOCORE_ENABLE_DEPRECATED_SINCE(6, 11)
    /*!
     * Returns the preferred timeout value for proxy connections
     * in seconds.
     *
     * Returns the timeout value for proxy connection in secs.
     *
     * This is not used.
     *
     * \deprecated[6.11]
     */
    KIOCORE_DEPRECATED_VERSION(6, 11, "Not used")
    static int proxyConnectTimeout();
#endif

#if KIOCORE_ENABLE_DEPRECATED_SINCE(6, 11)
    /*!
     * Returns the preferred response timeout value for
     * remote connecting in seconds.
     *
     * Returns the timeout value for remote connection in seconds.
     *
     * This is not used.
     *
     * \deprecated[6.11]
     */
    KIOCORE_DEPRECATED_VERSION(6, 11, "Not used")
    static int responseTimeout();
#endif

    /*============================ DOWNLOAD CONFIG ==============================*/

    /*!
     * Returns \c true if partial downloads should be
     * automatically resumed.
     */
    static bool autoResume();

    /*!
     * Returns \c true if partial downloads should be marked
     * with a ".part" extension.
     */
    static bool markPartial();

    /*!
     * Returns the minimum file size for keeping aborted
     * downloads.
     *
     * Any data downloaded that does not meet this minimum
     * requirement will simply be discarded. The default size
     * is 5 KB.
     */
    static int minimumKeepSize();

    /*===================== PROTOCOL CAPABILITIES ===============================*/

    /*!
     * Returns whether the protocol can list files/objects.
     * If a protocol supports listing it can be browsed in e.g. file-dialogs
     * and konqueror.
     *
     * Whether a protocol supports listing is determined by the "listing="
     * field in the protocol description file.
     * If the protocol support listing it should list the fields it provides in
     * this field. If the protocol does not support listing this field should
     * remain empty (default.)
     *
     * \a url the url to check
     * \sa listing()
     */
    static bool supportsListing(const QUrl &url);

    /*!
     * Returns whether the protocol can retrieve data from URLs.
     *
     * This corresponds to the "reading=" field in the protocol description file.
     * Valid values for this field are "true" or "false" (default).
     *
     * \a url the url to check
     */
    static bool supportsReading(const QUrl &url);

    /*!
     * Returns whether the protocol can store data to URLs.
     *
     * This corresponds to the "writing=" field in the protocol description file.
     * Valid values for this field are "true" or "false" (default).
     *
     * \a url the url to check
     */
    static bool supportsWriting(const QUrl &url);

    /*!
     * Returns whether the protocol can create directories/folders.
     *
     * This corresponds to the "makedir=" field in the protocol description file.
     * Valid values for this field are "true" or "false" (default).
     *
     * \a url the url to check
     */
    static bool supportsMakeDir(const QUrl &url);

    /*!
     * Returns whether the protocol can delete files/objects.
     *
     * This corresponds to the "deleting=" field in the protocol description file.
     * Valid values for this field are "true" or "false" (default).
     *
     * \a url the url to check
     */
    static bool supportsDeleting(const QUrl &url);

    /*!
     * Returns whether the protocol can create links between files/objects.
     *
     * This corresponds to the "linking=" field in the protocol description file.
     * Valid values for this field are "true" or "false" (default).
     *
     * \a url the url to check
     */
    static bool supportsLinking(const QUrl &url);

    /*!
     * Returns whether the protocol can move files/objects between different
     * locations.
     *
     * This corresponds to the "moving=" field in the protocol description file.
     * Valid values for this field are "true" or "false" (default).
     *
     * \a url the url to check
     */
    static bool supportsMoving(const QUrl &url);

    /*!
     * Returns whether the protocol can be opened using KIO::open(const QUrl&).
     *
     * This corresponds to the "opening=" field in the protocol description file.
     * Valid values for this field are "true" or "false" (default).
     *
     * \a url the url to check
     */
    static bool supportsOpening(const QUrl &url);

    /*!
     * Returns whether the protocol can be truncated with FileJob::truncate(KIO::filesize_t length).
     *
     * This corresponds to the "truncating=" field in the protocol description file.
     * Valid values for this field are "true" or "false" (default).
     *
     * \a url the url to check
     * \since 5.66
     */
    static bool supportsTruncating(const QUrl &url);

    /*!
     * Returns whether the protocol can copy files/objects directly from the
     * filesystem itself. If not, the application will read files from the
     * filesystem using the file-protocol and pass the data on to the destination
     * protocol.
     *
     * This corresponds to the "copyFromFile=" field in the protocol description file.
     * Valid values for this field are "true" or "false" (default).
     *
     * \a url the url to check
     */
    static bool canCopyFromFile(const QUrl &url);

    /*!
     * Returns whether the protocol can copy files/objects directly to the
     * filesystem itself. If not, the application will receive the data from
     * the source protocol and store it in the filesystem using the
     * file-protocol.
     *
     * This corresponds to the "copyToFile=" field in the protocol description file.
     * Valid values for this field are "true" or "false" (default).
     *
     * \a url the url to check
     */
    static bool canCopyToFile(const QUrl &url);

    /*!
     * Returns whether the protocol can rename (i.e. move fast) files/objects
     * directly from the filesystem itself. If not, the application will read
     * files from the filesystem using the file-protocol and pass the data on
     * to the destination protocol.
     *
     * This corresponds to the "renameFromFile=" field in the protocol description file.
     * Valid values for this field are "true" or "false" (default).
     *
     * \a url the url to check
     */
    static bool canRenameFromFile(const QUrl &url);

    /*!
     * Returns whether the protocol can rename (i.e. move fast) files/objects
     * directly to the filesystem itself. If not, the application will receive
     * the data from the source protocol and store it in the filesystem using the
     * file-protocol.
     *
     * This corresponds to the "renameToFile=" field in the protocol description file.
     * Valid values for this field are "true" or "false" (default).
     *
     * \a url the url to check
     */
    static bool canRenameToFile(const QUrl &url);

    /*!
     * Returns whether the protocol can recursively delete directories by itself.
     * If not (the usual case) then KIO will list the directory and delete files
     * and empty directories one by one.
     *
     * This corresponds to the "deleteRecursive=" field in the protocol description file.
     * Valid values for this field are "true" or "false" (default).
     *
     * \a url the url to check
     */
    static bool canDeleteRecursive(const QUrl &url);

    /*!
     * This setting defines the strategy to use for generating a filename, when
     * copying a file or directory to another directory. By default the destination
     * filename is made out of the filename in the source URL. However if the
     * KIO worker displays names that are different from the filename of the URL
     * (e.g. kio_fonts shows Arial for arial.ttf, or kio_trash shows foo.txt and
     * uses some internal URL), using Name means that the display name (UDS_NAME)
     * will be used to as the filename in the destination directory.
     *
     * This corresponds to the "fileNameUsedForCopying=" field in the protocol description file.
     * Valid values for this field are "Name" or "FromURL" (default).
     *
     * \a url the url to check
     */
    static KProtocolInfo::FileNameUsedForCopying fileNameUsedForCopying(const QUrl &url);

    /*!
     * Returns default MIME type for this URL based on the protocol.
     *
     * This corresponds to the "defaultMimetype=" field in the protocol description file.
     *
     * \a url the url to check
     */
    static QString defaultMimetype(const QUrl &url);

    /*!
     * Returns whether the protocol should be treated as a filesystem
     * or as a stream when reading from it.
     *
     * This corresponds to the "input=" field in the protocol description file.
     * Valid values for this field are "filesystem", "stream" or "none" (default).
     *
     * \a url the url to check
     */
    static KProtocolInfo::Type inputType(const QUrl &url);

    /*!
     * Returns whether the protocol should be treated as a filesystem
     * or as a stream when writing to it.
     *
     * This corresponds to the "output=" field in the protocol description file.
     * Valid values for this field are "filesystem", "stream" or "none" (default).
     *
     * \a url the url to check
     */
    static KProtocolInfo::Type outputType(const QUrl &url);

    /*!
     * Returns the list of fields this protocol returns when listing
     * The current possibilities are
     * Name, Type, Size, Date, AccessDate, Access, Owner, Group, Link, URL, MimeType
     * as well as Extra1, Extra2 etc. for extra fields (see extraFields).
     *
     * This corresponds to the "listing=" field in the protocol description file.
     * The supported fields should be separated with ',' in the protocol description file.
     *
     * \a url the url to check
     */
    static QStringList listing(const QUrl &url);

    /*!
     * Returns whether the protocol can act as a source protocol.
     *
     * A source protocol retrieves data from or stores data to the
     * location specified by a URL.
     * A source protocol is the opposite of a filter protocol.
     *
     * The "source=" field in the protocol description file determines
     * whether a protocol is a source protocol or a filter protocol.
     *
     * \a url the url to check
     *
     * Returns true if the protocol is a source of data (e.g. http), false if the
     *         protocol is a filter (e.g. gzip)
     */
    static bool isSourceProtocol(const QUrl &url);

    /*!
     * Returns which protocol handles this MIME type, if it's an archive MIME type.
     * For instance zip:/ handles application/x-zip.
     *
     * This is defined in the protocol description file using an entry like
     * "archiveMimetype=application/x-zip"
     *
     * \a mimeType the MIME type to check
     */
    static QString protocolForArchiveMimetype(const QString &mimeType);

    /*=============================== OTHERS ====================================*/

    /*!
     * Force a reload of the general config file of
     * KIO workers ( kioslaverc).
     */
    static void reparseConfiguration();

    /*!
     * Returns the charset to use for the specified \a url.
     *
     */
    static QString charsetFor(const QUrl &url);

    /*!
     * Returns whether the protocol suppports KIO/POSIX permissions handling.
     *
     * When this is false the Permissions properties tab may be hidden, for example. The protocol may still support
     * permission control through other means, specific to the individual KIO worker.
     *
     * \a url the url to check
     * \since 5.98
     */
    static bool supportsPermissions(const QUrl &url);

private:
    friend class KIO::WorkerConfigPrivate;

    KIOCORE_NO_EXPORT static QMap<QString, QString> entryMap(const QString &group);
};
#endif
