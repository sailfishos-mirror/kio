/*
    This file is part of the KDE desktop environment
    SPDX-FileCopyrightText: 2001, 2002 Michael Brade <brade@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KDIRLISTERTEST_GUI_H_
#define _KDIRLISTERTEST_GUI_H_

#include <QString>
#include <QUrl>
#include <QWidget>

#include <kdirlister.h>
#include <kfileitem.h>

#include <iostream>

using namespace std;

class PrintSignals : public QObject
{
    Q_OBJECT
public:
    PrintSignals()
        : QObject()
    {
    }

public Q_SLOTS:
    void started(const QUrl &url)
    {
        cout << "*** started( " << url.url().toLocal8Bit().data() << " )" << endl;
    }
    void canceled()
    {
        cout << "canceled()" << endl;
    }
    void listingDirCanceled(const QUrl &url)
    {
        cout << "*** canceled( " << url.toDisplayString().toLocal8Bit().data() << " )" << endl;
    }
    void completed()
    {
        cout << "*** completed()" << endl;
    }
    void listingDirCompleted(const QUrl &url)
    {
        cout << "*** completed( " << url.toDisplayString().toLocal8Bit().data() << " )" << endl;
    }
    void redirection(const QUrl &src, const QUrl &dest)
    {
        cout << "*** redirection( " << src.toDisplayString().toLocal8Bit().data() << ", " << dest.toDisplayString().toLocal8Bit().data() << " )" << endl;
    }
    void clear()
    {
        cout << "*** clear()" << endl;
    }
    void newItems(const KFileItemList &items)
    {
        cout << "*** newItems: " << endl;
        for (auto it = items.cbegin(), itEnd = items.constEnd(); it != itEnd; ++it) {
            cout << (*it).name().toLocal8Bit().data() << endl;
        }
    }
    void itemsDeleted(const KFileItemList &)
    {
        cout << "*** itemsDeleted: " << endl;
        // TODO
    }
    void refreshItems(const QList<QPair<KFileItem, KFileItem>> &)
    {
        cout << "*** refreshItems: " << endl;
        // TODO
    }
    void infoMessage(const QString &msg)
    {
        cout << "*** infoMessage: " << msg.toLocal8Bit().data() << endl;
    }

    void percent(int percent)
    {
        cout << "*** percent: " << percent << endl;
    }

    void totalSize(KIO::filesize_t size)
    {
        cout << "*** totalSize: " << (long)size << endl;
    }

    void processedSize(KIO::filesize_t size)
    {
        cout << "*** processedSize: " << (long)size << endl;
    }

    void speed(int bytes_per_second)
    {
        cout << "*** speed: " << bytes_per_second << endl;
    }
};

class KDirListerTest : public QWidget
{
    Q_OBJECT
public:
    KDirListerTest(QWidget *parent = nullptr, const QUrl &url = {});
    ~KDirListerTest() override;

public Q_SLOTS:
    void startRoot();
    void startHome();
    void startTar();
    void test();
    void completed();

private:
    KDirLister *lister;
    PrintSignals *debug;
};

#endif
