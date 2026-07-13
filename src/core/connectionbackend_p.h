/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Thiago Macieira <thiago@kde.org>
    SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIO_CONNECTIONBACKEND_P_H
#define KIO_CONNECTIONBACKEND_P_H

#include <QObject>
#include <QUrl>

#include <memory>
#include <typeinfo>

namespace KIO
{
/*!
 * \internal
 *
 * The payload carried with a command. \a data is the serialized bytes (used by the socket transport,
 * and as a small header even in-process). \a object is an optional live payload handed over by pointer
 * in-process only (ThreadConnectionBackend), sharing memory with the peer thread instead of being
 * serialized; it is always null on the socket transport, which cannot carry a pointer across a
 * process. The two are not exclusive - a command may carry a byte header in \a data and a live \a
 * object at once (e.g. batch-stat). See Connection::sendObject().
 *
 * \a type records the concrete type behind \a object (the pointee of the std::shared_ptr it was sent
 * as), so the receiver can Q_ASSERT the lane carries what the command expects before casting; it is
 * null when there is no object.
 */
struct Payload {
    QByteArray data{};
    std::shared_ptr<void> object{};
    const std::type_info *type = nullptr;
};

struct Task {
    int cmd = -1;
    QByteArray data{};
    std::shared_ptr<void> object{};
    const std::type_info *type = nullptr;
};

/*!
 * \internal
 *
 * Abstract transport used by Connection to exchange Tasks (command + payload)
 * with a peer. The peer can live in another process (SocketConnectionBackend,
 * over a QLocalSocket) or in another thread of the same process
 * (ThreadConnectionBackend, over direct queued signals/slots).
 */
class ConnectionBackend : public QObject
{
    Q_OBJECT

public:
    enum State {
        Idle,
        Listening,
        Connected,
    };
    State state = Idle;
    QUrl address;
    QString errorString;

    explicit ConnectionBackend(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    ~ConnectionBackend() override = default;

    virtual void setSuspended(bool enable) = 0;
    virtual void close() = 0;
    virtual bool waitForIncomingTask(int ms) = 0;
    virtual bool sendCommand(int command, Payload payload) = 0;

    // True for a same-process (thread) transport, where a command may hand its payload over live as
    // a shared_ptr instead of serializing it. False for the socket transport.
    virtual bool isInProcess() const
    {
        return false;
    }

Q_SIGNALS:
    void disconnected();
    void commandReceived(const KIO::Task &task);
};
}

#endif
