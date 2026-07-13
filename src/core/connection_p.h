/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIO_CONNECTION_P_H
#define KIO_CONNECTION_P_H

#include "connectionbackend_p.h"
#include <QList>
#include <QObject>
#include <QString>
#include <QUrl>

#include <functional>
#include <memory>
#include <typeinfo>

namespace KIO
{
class ConnectionServer;
class ConnectionPrivate;
/*!
 * \internal
 *
 * This class provides a simple means for IPC between two applications
 * via a pipe.
 * It handles a queue of commands to be sent which makes it possible to
 * queue data before an actual connection has been established.
 */
class Connection : public QObject
{
    Q_OBJECT

public:
    enum class ReadMode {
        Polled, /// Any new tasks will be polled
        EventDriven, /// We need to emit signals when we have pending events. Requires a working QEventLoop
    };

    enum class Type {
        Application, /// This is the connection of the application side
        Worker, /// This is the connection of the worker side
    };
    /*!
     * Creates a new connection.
     * \sa connectToRemote, listenForRemote
     */
    explicit Connection(Type type, QObject *parent = nullptr);
    ~Connection() override;

    /*!
     * Connects to the remote address.
     * \a address a local:// or tcp:// URL.
     */
    void connectToRemote(const QUrl &address);

    /// Closes the connection.
    void close();

    bool isConnected() const;

    /*!
     * Checks whether the connection has been initialized.
     * Returns true if the initialized
     * \sa init()
     */
    bool inited() const;

    /*!
     * Sends/queues the given command to be sent, with its Payload (bytes and/or an in-process object).
     * Returns true if successful, false otherwise
     */
    bool send(int cmd, Payload payload);

    /*!
     * Shim: the common bytes-only send. Wraps the bytes in a Payload.
     */
    bool send(int cmd, const QByteArray &bytes = QByteArray())
    {
        return send(cmd, Payload{bytes, {}});
    }

    /*!
     * Sends the given command immediately, with its Payload.
     * Returns true if successful, false otherwise
     */
    bool sendnow(int cmd, Payload payload);

    /*!
     * Shim: the common bytes-only immediate send.
     */
    bool sendnow(int cmd, const QByteArray &data)
    {
        return sendnow(cmd, Payload{data, {}});
    }

    /*!
     * Sends a command whose payload is an object, choosing the cheapest carrier for the transport:
     * in-process the object is handed over live (no serialization); out-of-process \a serialize is
     * called to produce the bytes. The receiver gets the object (in-process) or the bytes.
     */
    template<class T>
    bool sendObject(int cmd, std::shared_ptr<T> object, std::function<QByteArray()> serialize)
    {
        return sendObjectErased(cmd, std::static_pointer_cast<void>(std::move(object)), &typeid(T), std::move(serialize));
    }

    // Type-erased worker behind the sendObject() template. \a type records T so the receiver can
    // Q_ASSERT the lane before casting.
    bool sendObjectErased(int cmd, std::shared_ptr<void> object, const std::type_info *type, const std::function<QByteArray()> &serialize);

    /*!
     * Returns true if the transport is in-process (a thread), so sendObject() can pass a live object.
     */
    bool isInProcess() const;

    /*!
     * Returns true if there are packets to be read immediately,
     * false if waitForIncomingTask must be called before more data
     * is available.
     */
    bool hasTaskAvailable() const;

    /*!
     * Waits for one more command to be handled and ready.
     *
     * \a ms   the time to wait in milliseconds
     * Returns true if one command can be read, false if we timed out
     */
    bool waitForIncomingTask(int ms = 30000);

    /*!
     * Receive data.
     *
     * \a _cmd the received command will be written here
     * \a data the received data will be written here

     * Returns >=0 indicates the received data size upon success
     *         -1  indicates error
     */
    int read(int *_cmd, Payload &payload);

    /*!
     * Don't handle incoming data until resumed.
     */
    void suspend();

    /*!
     * Resume handling of incoming data.
     */
    void resume();

    /*!
     * Returns status of connection.
     * Returns true if suspended, false otherwise
     */
    bool suspended() const;

    void setReadMode(ReadMode mode);

    /*!
     * Adopts a ready-made backend (used to pair an in-process worker without a
     * socket handshake), taking ownership of it.
     */
    void setBackend(std::unique_ptr<ConnectionBackend> backend);

Q_SIGNALS:
    void readyRead();

private:
    friend class ConnectionPrivate;
    friend class ConnectionServer;
    std::unique_ptr<class ConnectionPrivate> const d;
    Type m_type;
};

// Separated from Connection only for historical reasons - they are both private now
class ConnectionPrivate
{
public:
    inline ConnectionPrivate()
        : q(nullptr)
        , suspended(false)
        , readMode(Connection::ReadMode::EventDriven)
    {
    }

    void dequeue();
    void commandReceived(const Task &task);
    void disconnected();
    void setBackend(std::unique_ptr<ConnectionBackend> b);

    QList<Task> outgoingTasks;
    QList<Task> incomingTasks;
    std::unique_ptr<ConnectionBackend> backend;
    Connection *q;
    bool suspended;
    Connection::ReadMode readMode;
};

class ConnectionServerPrivate;
}

#endif
