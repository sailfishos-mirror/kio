/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "dbusactivationrunner_p.h"

#include "kiogui_debug.h"
#include <KConfigGroup>
#include <KDesktopFile>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QTimer>

bool DBusActivationRunner::activationPossible(const QString &serviceEntryPath, KIO::ApplicationLauncherJob::RunFlags flags, const QString &suggestedFileName)
{
#if defined Q_OS_UNIX && !defined Q_OS_ANDROID
    const KDesktopFile desktopFile(serviceEntryPath);
    if (!desktopFile.hasApplicationType()) {
        return false;
    }
    if (desktopFile.desktopGroup().readEntry("DBusActivatable", false)) {
        if (!suggestedFileName.isEmpty()) {
            qCDebug(KIO_GUI) << "Cannot activate" << serviceEntryPath << "because suggestedFileName is set";
            return false;
        }
        if (flags & KIO::ApplicationLauncherJob::DeleteTemporaryFiles) {
            qCDebug(KIO_GUI) << "Cannot activate" << serviceEntryPath << "because DeleteTemporaryFiles is set";
            return false;
        }
        return true;
    }
#endif
    return false;
}

DBusActivationRunner::DBusActivationRunner(const QString &action)
    : KProcessRunner()
    , m_actionName(action)
{
}

void DBusActivationRunner::startProcess()
{
    // DBusActivatatable as per https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html#dbus
    const QString objectPath = QStringLiteral("/%1").arg(m_desktopName).replace(QLatin1Char('.'), QLatin1Char('/'));
    const QString interface = QStringLiteral("org.freedesktop.Application");
    QDBusMessage message;
    if (m_urls.isEmpty()) {
        if (m_actionName.isEmpty()) {
            message = QDBusMessage::createMethodCall(m_desktopName, objectPath, interface, QStringLiteral("Activate"));
        } else {
            message = QDBusMessage::createMethodCall(m_desktopName, objectPath, interface, QStringLiteral("ActivateAction"));
            message << m_actionName << QVariantList();
        }
    } else {
        message = QDBusMessage::createMethodCall(m_desktopName, objectPath, interface, QStringLiteral("Open"));
        message << QUrl::toStringList(m_urls);
    }
    message << QVariantMap{{QStringLiteral("desktop-startup-id"), m_startupId.id()}};
    auto call = QDBusConnection::sessionBus().asyncCall(message);
    auto watcher = new QDBusPendingCallWatcher(call);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();
        if (watcher->isError()) {
            Q_EMIT error(watcher->error().name());
            terminateStartupNotification();
        } else {
            setPid(QDBusConnection::sessionBus().interface()->servicePid(m_desktopName));
            Q_EMIT processStarted(m_pid);
        }
        deleteLater();
    });
}

bool DBusActivationRunner::waitForStarted(int timeout)
{
    if (m_pid || m_watcher->isFinished()) {
        return true;
    }
    QEventLoop loop;
    bool success = false;
    connect(this, &KProcessRunner::processStarted, [&loop, &success]() {
        loop.quit();
        success = true;
    });
    connect(this, &KProcessRunner::error, &loop, &QEventLoop::quit);
    QTimer::singleShot(timeout, &loop, &QEventLoop::quit);
    loop.exec();
    return success;
}
