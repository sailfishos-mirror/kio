/*
    This file is part of the KDE libraries
    Copyright (c) 2020 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KIO_COMMANDLAUNCHERJOB_H
#define KIO_COMMANDLAUNCHERJOB_H

#include "kiogui_export.h"
#include <KJob>

namespace KIO {

class CommandLauncherJobPrivate;

/**
 * @brief CommandLauncherJob runs a command and watches it while running.
 *
 * It creates a startup notification and finishes it on success or on error (for the taskbar).
 * It also emits an error message if necessary (e.g. "program not found").
 *
 * The job finishes when the application is successfully started. At that point you can
 * query the PID.
 *
 * @since 5.69
 */
class KIOGUI_EXPORT CommandLauncherJob : public KJob
{
public:
    /**
     * @brief Creates a CommandLauncherJob
     * @param command the shell command to run
     * @param parent the parent QObject
     *
     * Please consider also calling setExecutable and setIcon for better startup notification.
     */
    explicit CommandLauncherJob(const QString &command, QObject *parent = nullptr);

    /**
     * Destructor
     * Note that jobs auto-delete themselves after emitting result
     */
    ~CommandLauncherJob() override;

    /**
     * Allows to set the name of the executable, used in the startup notification.
     * (see KStartupInfoData::setBin)
     * @param executable executable name, with or without a path
     * Alternatively, use setDesktopName.
     */
    void setExecutable(const QString &executable);

    /**
     * @brief Sets the icon for the startup notification
     * @param iconName name of the icon, to be loaded from the current icon theme
     * Alternatively, use setDesktopName.
     */
    void setIcon(const QString &iconName);

    /**
     * Allows to set the name of the desktop file (e.g. "org.kde.dolphin", the extension is optional)
     * This is an alternative solution for setIcon and setExecutable, i.e. the icon
     * will be taken from the desktop file, and the executable inferred from the Exec line.
     */
    void setDesktopName(const QString &desktopName);

    /**
     * @brief setStartupId sets the startupId of the new application
     * @param startupId Application startup notification id, if any (otherwise "").
     */
    void setStartupId(const QByteArray &startupId);

    /**
     * @brief Sets the working directory from which to run the command
     * @param workingDirectory path of a local directory
     */
    void setWorkingDirectory(const QString &workingDirectory);

    /**
     * @brief start starts the job. You must call this, after all the setters.
     */
    void start() override;

    /**
     * Blocks until the process has started.
     */
    bool waitForStarted();

    /**
     * @return the PID of the application that was started.
     * Available after the job emits result().
     */
    qint64 pid() const;

private:
    friend class CommandLauncherJobPrivate;
    QScopedPointer<CommandLauncherJobPrivate> d;
};

} // namespace KIO

#endif
