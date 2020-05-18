/*
    This file is part of the KDE libraries
    Copyright (c) 2014, 2020 David Faure <faure@kde.org>

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

#include "applicationlauncherjobtest.h"
#include "applicationlauncherjob.h"
#include <kprocessrunner_p.h>
#include <untrustedprogramhandlerinterface.h>
#include <openwithhandlerinterface.h>

#include "kiotesthelper.h" // createTestFile etc.

#include <kservice.h>
#include <kconfiggroup.h>
#include <KDesktopFile>

#ifdef Q_OS_UNIX
#include <signal.h> // kill
#endif

#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

QTEST_GUILESS_MAIN(ApplicationLauncherJobTest)

namespace KIO {
KIOGUI_EXPORT void setDefaultUntrustedProgramHandler(KIO::UntrustedProgramHandlerInterface *iface);
KIOGUI_EXPORT void setDefaultOpenWithHandler(KIO::OpenWithHandlerInterface *iface);
}
class TestUntrustedProgramHandler : public KIO::UntrustedProgramHandlerInterface
{
public:
    void showUntrustedProgramWarning(KJob *job, const QString &programName) override {
        Q_UNUSED(job)
        m_calls << programName;
        Q_EMIT result(m_retVal);
    }

    void setRetVal(bool b) { m_retVal = b; }

    QStringList m_calls;
    bool m_retVal = false;
};

static TestUntrustedProgramHandler s_handler;

class TestOpenWithHandler : public KIO::OpenWithHandlerInterface
{
public:
    void promptUserForApplication(KJob *job, const QList<QUrl> &url, const QString &mimeType) override
    {
        Q_UNUSED(job);
        m_urls << url;
        m_mimeTypes << mimeType;
        if (m_chosenService) {
            Q_EMIT serviceSelected(m_chosenService);
        } else {
            Q_EMIT canceled();
        }
    }
    QList<QUrl> m_urls;
    QStringList m_mimeTypes;
    KService::Ptr m_chosenService;
};
static TestOpenWithHandler s_openWithHandler;

void ApplicationLauncherJobTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void ApplicationLauncherJobTest::cleanupTestCase()
{
    std::for_each(m_filesToRemove.begin(), m_filesToRemove.end(), [](const QString & f) {
        QFile::remove(f);
    });
}

static const char s_tempServiceName[] = "applicationlauncherjobtest_service.desktop";

static void createSrcFile(const QString path)
{
    QFile srcFile(path);
    QVERIFY2(srcFile.open(QFile::WriteOnly), qPrintable(srcFile.errorString()));
    srcFile.write("Hello world\n");
}

void ApplicationLauncherJobTest::startProcess_data()
{
    QTest::addColumn<bool>("tempFile");
    QTest::addColumn<bool>("useExec");
    QTest::addColumn<int>("numFiles");

    QTest::newRow("1_file_exec") << false << true << 1;
    QTest::newRow("1_file_waitForStarted") << false << false << 1;
    QTest::newRow("1_tempfile_exec") << true << true << 1;
    QTest::newRow("1_tempfile_waitForStarted") << true << false << 1;

    QTest::newRow("2_files_exec") << false << true << 2;
    QTest::newRow("2_files_waitForStarted") << false << false << 2;
    QTest::newRow("2_tempfiles_exec") << true << true << 2;
    QTest::newRow("2_tempfiles_waitForStarted") << true << false << 2;
}

void ApplicationLauncherJobTest::startProcess()
{
    QFETCH(bool, tempFile);
    QFETCH(bool, useExec);
    QFETCH(int, numFiles);

    // Given a service desktop file and a number of source files
    const QString path = createTempService();
    QTemporaryDir tempDir;
    const QString srcDir = tempDir.path();
    QList<QUrl> urls;
    for (int i = 0; i < numFiles; ++i) {
        const QString srcFile = srcDir + "/srcfile" + QString::number(i + 1);
        createSrcFile(srcFile);
        QVERIFY(QFile::exists(srcFile));
        urls.append(QUrl::fromLocalFile(srcFile));
    }

    // When running a ApplicationLauncherJob
    KService::Ptr servicePtr(new KService(path));
    KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(servicePtr, this);
    job->setUrls(urls);
    if (tempFile) {
        job->setRunFlags(KIO::ApplicationLauncherJob::DeleteTemporaryFiles);
    }
    if (useExec) {
        QVERIFY2(job->exec(), qPrintable(job->errorString()));
    } else {
        job->start();
        QVERIFY(job->waitForStarted());
    }
    const QVector<qint64> pids = job->pids();

    // Then the service should be executed (which copies the source file to "dest")
    QCOMPARE(pids.count(), numFiles);
    QVERIFY(!pids.contains(0));
    for (int i = 0; i < numFiles; ++i) {
        const QString dest = srcDir + "/dest_srcfile" + QString::number(i + 1);
        QTRY_VERIFY2(QFile::exists(dest), qPrintable(dest));
        QVERIFY(QFile::exists(srcDir + "/srcfile" + QString::number(i + 1))); // if tempfile is true, kioexec will delete it... in 3 minutes.
        QVERIFY(QFile::remove(dest)); // cleanup
    }

#ifdef Q_OS_UNIX
    // Kill the running kioexec processes
    for (qint64 pid : pids) {
        ::kill(pid, SIGTERM);
    }
#endif

    // The kioexec processes that are waiting for 3 minutes and got killed above,
    // will now trigger KProcessRunner::slotProcessError, KProcessRunner::slotProcessExited and delete the KProcessRunner.
    // We wait for that to happen otherwise it gets confusing to see that output from later tests.
    QTRY_COMPARE(KProcessRunner::instanceCount(), 0);
}

void ApplicationLauncherJobTest::shouldFailOnNonExecutableDesktopFile_data()
{
    QTest::addColumn<bool>("withHandler");
    QTest::addColumn<bool>("handlerRetVal");
    QTest::addColumn<bool>("useExec");

    QTest::newRow("no_handler_exec") << false << false << true;
    QTest::newRow("handler_false_exec") << true << false << true;
    QTest::newRow("handler_true_exec") << true << true << true;
    QTest::newRow("no_handler_waitForStarted") << false << false << false;
    QTest::newRow("handler_false_waitForStarted") << true << false << false;
    QTest::newRow("handler_true_waitForStarted") << true << true << false;
}

void ApplicationLauncherJobTest::shouldFailOnNonExecutableDesktopFile()
{
    QFETCH(bool, useExec);
    QFETCH(bool, withHandler);
    QFETCH(bool, handlerRetVal);

    // Given a .desktop file in a temporary directory (outside the trusted paths)
    QTemporaryDir tempDir;
    const QString srcDir = tempDir.path();
    const QString desktopFilePath = srcDir + "/shouldfail.desktop";
    writeTempServiceDesktopFile(desktopFilePath);
    m_filesToRemove.append(desktopFilePath);

    const QString srcFile = srcDir + "/srcfile";
    createSrcFile(srcFile);
    const QList<QUrl> urls{QUrl::fromLocalFile(srcFile)};
    KService::Ptr servicePtr(new KService(desktopFilePath));

    s_handler.m_calls.clear();
    s_handler.setRetVal(handlerRetVal);
    KIO::setDefaultUntrustedProgramHandler(withHandler ? &s_handler : nullptr);

    KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(servicePtr, this);
    job->setUrls(urls);
    bool success;
    if (useExec) {
        success = job->exec();
    } else {
        job->start();
        success = job->waitForStarted();
    }
    if (!withHandler) {
        QVERIFY(!success);
        QCOMPARE(job->error(), KJob::UserDefinedError);
        QCOMPARE(job->errorString(), QStringLiteral("You are not authorized to execute this file."));
    } else {
        if (handlerRetVal) {
            QVERIFY(success);
            const QString dest = srcDir + "/dest_srcfile";
            QTRY_VERIFY2(QFile::exists(dest), qPrintable(dest));

            // The actual shell process will race against the deletion of the QTemporaryDir,
            // so don't be surprised by stderr like getcwd: cannot access parent directories: No such file or directory
            QTest::qWait(50); // this helps a bit
        } else {
            QVERIFY(!success);
            QCOMPARE(job->error(), KIO::ERR_USER_CANCELED);
        }
        }

    if (withHandler) {
        // check that the handler was called
        QCOMPARE(s_handler.m_calls.count(), 1);
        QCOMPARE(s_handler.m_calls.at(0), QStringLiteral("KRunUnittestService"));
    }

}

void ApplicationLauncherJobTest::shouldFailOnNonExistingExecutable_data()
{
    QTest::addColumn<bool>("tempFile");
    QTest::addColumn<bool>("fullPath");

    QTest::newRow("file") << false << false;
    QTest::newRow("tempFile") << true << false;
    QTest::newRow("file_fullPath") << false << true;
    QTest::newRow("tempFile_fullPath") << true << true;
}

void ApplicationLauncherJobTest::shouldFailOnNonExistingExecutable()
{
    QFETCH(bool, tempFile);
    QFETCH(bool, fullPath);

    const QString desktopFilePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kservices5/non_existing_executable.desktop");
    KDesktopFile file(desktopFilePath);
    KConfigGroup group = file.desktopGroup();
    group.writeEntry("Name", "KRunUnittestService");
    group.writeEntry("Type", "Service");
    if (fullPath) {
        group.writeEntry("Exec", "/usr/bin/does_not_exist %f %d/dest_%n");
    } else {
        group.writeEntry("Exec", "does_not_exist %f %d/dest_%n");
    }
    file.sync();

    KService::Ptr servicePtr(new KService(desktopFilePath));
    KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(servicePtr, this);
    job->setUrls({QUrl::fromLocalFile(desktopFilePath)}); // just to have one URL as argument, as the desktop file expects
    if (tempFile) {
        job->setRunFlags(KIO::ApplicationLauncherJob::DeleteTemporaryFiles);
    }
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), KJob::UserDefinedError);
    if (fullPath) {
        QCOMPARE(job->errorString(), QStringLiteral("Could not find the program '/usr/bin/does_not_exist'"));
    } else {
        QCOMPARE(job->errorString(), QStringLiteral("Could not find the program 'does_not_exist'"));
    }
    QFile::remove(desktopFilePath);
}

void ApplicationLauncherJobTest::shouldFailOnInvalidService()
{
    const QString desktopFilePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kservices5/invalid_service.desktop");
    KDesktopFile file(desktopFilePath);
    KConfigGroup group = file.desktopGroup();
    group.writeEntry("Name", "KRunUnittestService");
    group.writeEntry("Type", "NoSuchType");
    group.writeEntry("Exec", "does_not_exist");
    file.sync();

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("The desktop entry file \".*\" has Type.*\"NoSuchType\" instead of \"Application\" or \"Service\""));
    KService::Ptr servicePtr(new KService(desktopFilePath));
    KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(servicePtr, this);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), KJob::UserDefinedError);
    QCOMPARE(job->errorString(), QStringLiteral("The desktop entry file\n%1\nis not valid.").arg(desktopFilePath));

    QFile::remove(desktopFilePath);
}

void ApplicationLauncherJobTest::shouldFailOnServiceWithNoExec()
{
    const QString desktopFilePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kservices5/invalid_service.desktop");
    KDesktopFile file(desktopFilePath);
    KConfigGroup group = file.desktopGroup();
    group.writeEntry("Name", "KRunUnittestServiceNoExec");
    group.writeEntry("Type", "Service");
    file.sync();

    QTest::ignoreMessage(QtWarningMsg, qPrintable(QString("No Exec field in \"%1\"").arg(desktopFilePath)));
    KService::Ptr servicePtr(new KService(desktopFilePath));
    KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(servicePtr, this);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), KJob::UserDefinedError);
    QCOMPARE(job->errorString(), QStringLiteral("No Exec field in %1").arg(desktopFilePath));

    QFile::remove(desktopFilePath);
}

void ApplicationLauncherJobTest::shouldFailOnExecutableWithoutPermissions()
{
#ifdef Q_OS_UNIX
    // Given an executable shell script that copies "src" to "dest" (we'll cheat with the mimetype to treat it like a native binary)
    QTemporaryDir tempDir;
    const QString dir = tempDir.path();
    const QString scriptFilePath = dir + QStringLiteral("/script.sh");
    QFile scriptFile(scriptFilePath);
    QVERIFY(scriptFile.open(QIODevice::WriteOnly));
    scriptFile.write("#!/bin/sh\ncp src dest");
    scriptFile.close();
    // Note that it's missing executable permissions

    const QString desktopFilePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kservices5/invalid_service.desktop");
    KDesktopFile file(desktopFilePath);
    KConfigGroup group = file.desktopGroup();
    group.writeEntry("Name", "KRunUnittestServiceNoPermission");
    group.writeEntry("Type", "Service");
    group.writeEntry("Exec", scriptFilePath);
    file.sync();

    KService::Ptr servicePtr(new KService(desktopFilePath));
    KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(servicePtr, this);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), KJob::UserDefinedError);
    QCOMPARE(job->errorString(), QStringLiteral("The program '%1' is missing executable permissions.").arg(scriptFilePath));

    QFile::remove(desktopFilePath);
#else
    QSKIP("This test is not run on Windows");
#endif
}

void ApplicationLauncherJobTest::showOpenWithDialog_data()
{
     QTest::addColumn<bool>("handlerRetVal");

     QTest::newRow("false_canceled") << false;
     QTest::newRow("true_service_selected") << true;
}

void ApplicationLauncherJobTest::showOpenWithDialog()
{
#ifdef Q_OS_UNIX
    QFETCH(bool, handlerRetVal);

    // Given a local text file (we could test multiple files, too...)
    QTemporaryDir tempDir;
    const QString srcDir = tempDir.path();
    const QString srcFile = srcDir + QLatin1String("/file.txt");
    createSrcFile(srcFile);

    KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(this);
    job->setUrls({QUrl::fromLocalFile(srcFile)});

    KService::Ptr service = KService::serviceByDesktopName(QString(s_tempServiceName).remove(".desktop"));
    QVERIFY(service);
    s_openWithHandler.m_urls.clear();
    s_openWithHandler.m_mimeTypes.clear();
    s_openWithHandler.m_chosenService = handlerRetVal ? service : KService::Ptr{};
    KIO::setDefaultOpenWithHandler(&s_openWithHandler);

    const bool success = job->exec();

    // Then --- it depends on what the user says via the handler

    QCOMPARE(s_openWithHandler.m_urls.count(), 1);
    QCOMPARE(s_openWithHandler.m_mimeTypes.count(), 1);
    QCOMPARE(s_openWithHandler.m_mimeTypes.at(0), QString()); // the job doesn't have the information
    if (handlerRetVal) {
        QVERIFY2(success, qPrintable(job->errorString()));
        // If the user chose a service, it should be executed (it writes to "dest")
        const QString dest = srcDir + "/dest_file.txt";
        QTRY_VERIFY2(QFile::exists(dest), qPrintable(dest));
    } else {
        QVERIFY(!success);
        QCOMPARE(job->error(), KIO::ERR_USER_CANCELED);
    }
#else
    QSKIP("Test skipped on Windows because the code ends up in QDesktopServices::openUrl");
#endif
}

void ApplicationLauncherJobTest::writeTempServiceDesktopFile(const QString &filePath)
{
    if (!QFile::exists(filePath)) {
        KDesktopFile file(filePath);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "KRunUnittestService");
        group.writeEntry("Type", "Service");
#ifdef Q_OS_WIN
        group.writeEntry("Exec", "copy.exe %f %d/dest_%n");
#else
        group.writeEntry("Exec", "cd %d ; cp %f %d/dest_%n"); // cd is just to show that we can't do QFile::exists(binary)
#endif
        file.sync();
    }
}

QString ApplicationLauncherJobTest::createTempService()
{
    const QString fileName = s_tempServiceName;
    const QString fakeService = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kservices5/") + fileName;
    writeTempServiceDesktopFile(fakeService);
    m_filesToRemove.append(fakeService);
    return fakeService;
}
