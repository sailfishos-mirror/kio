/*
    SPDX-FileCopyrightText: 2015 Montel Laurent <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KURIFILTERSEARCHPROVIDERACTIONS_H
#define KURIFILTERSEARCHPROVIDERACTIONS_H

#include "kiowidgets_export.h"
#include <QObject>

#include <memory>

class QMenu;
class QAction;
namespace KIO
{
class WebShortcutsMenuManagerPrivate;
/*!
 * \class KIO::KUriFilterSearchProviderActions
 * \inheaderfile KIO/KUriFilterSearchProviderActions
 * \inmodule KIOWidgets
 *
 * \brief This class is a manager for web shortcuts.
 *
 * It will provide a list of web shortcuts against a selected text
 *
 * You can set the selected text with setSelectedText() function
 *
 * \since 5.16
 */
class KIOWIDGETS_EXPORT KUriFilterSearchProviderActions : public QObject
{
    Q_OBJECT
public:
    /*!
     * Constructs a webshorts menu manager.
     *
     * \a parent The QObject parent.
     */
    explicit KUriFilterSearchProviderActions(QObject *parent = nullptr);
    ~KUriFilterSearchProviderActions() override;

    /*!
     * Returns the selected text
     */
    QString selectedText() const;
    /*!
     * Set selected text
     * \a selectedText the text to search for
     */
    void setSelectedText(const QString &selectedText);

    /*!
     * Add web shortcut actions to existing menu.
     * \a menu menu to add shortcuts to
     */
    void addWebShortcutsToMenu(QMenu *menu);

private Q_SLOTS:
    KIOWIDGETS_NO_EXPORT void slotConfigureWebShortcuts();
    KIOWIDGETS_NO_EXPORT void slotHandleWebShortcutAction(QAction *action);

private:
    std::unique_ptr<WebShortcutsMenuManagerPrivate> const d;
};
}

#endif // WEBSHORTCUTSMENUMANAGER_H
