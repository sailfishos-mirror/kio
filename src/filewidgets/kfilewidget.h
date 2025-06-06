// -*- c++ -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997, 1998 Richard Moore <rich@kde.org>
    SPDX-FileCopyrightText: 1998 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 1998 Daniel Grana <grana@ie.iwi.unibe.ch>
    SPDX-FileCopyrightText: 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 2001 Frerich Raabe <raabe@kde.org>
    SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KFILEWIDGET_H
#define KFILEWIDGET_H

#include "kfile.h"
#include "kiofilewidgets_export.h"
#include <QWidget>

#include <KFileFilter>

#include <memory>

class QUrl;
class QPushButton;
class KActionCollection;
class KFileWidgetPrivate;
class KUrlComboBox;
class KFileFilterCombo;

class KPreviewWidgetBase;
class QMimeType;
class KConfigGroup;
class KJob;
class KFileItem;
class KDirOperator;

/*!
 * \class KFileWidget
 * \inmodule KIOFileWidgets
 *
 * \brief File selector widget.
 *
 * This is the contents of the KDE file dialog, without the actual QDialog around it.
 * It can be embedded directly into applications.
 */
class KIOFILEWIDGETS_EXPORT KFileWidget : public QWidget
{
    Q_OBJECT
public:
    /*!
     * Constructs a file selector widget.
     *
     * \a startDir This can either be:
     * \list
     * \li An empty URL (QUrl()) to start in the current working directory,
     *     or the last directory where a file has been selected.
     * \li The path or URL of a starting directory.
     * \li An initial file name to select, with the starting directory being
     *     the current working directory or the last directory where a file
     *     has been selected.
     * \li The path or URL of a file, specifying both the starting directory and
     *     an initially selected file name.
     * \li A URL of the form \c kfiledialog:///<keyword>; to start in the
     *     directory last used by a filedialog in the same application that
     *     specified the same keyword.
     * \li A URL of the form \c kfiledialog:///<keyword>/<filename>;
     *     to start in the directory last used by a filedialog in the same
     *     application that specified the same keyword, and to initially
     *     select the specified filename.
     * \li Deprecated: A URL of the form \c kfiledialog:///<keyword>?global to start
     *     in the directory last used by a filedialog in any application that
     *     specified the same keyword.
     * \li Deprecated: A URL of the form \c kfiledialog:///<keyword>/<filename>?global
     *     to start in the directory last used by a filedialog in any
     *     application that specified the same keyword, and to initially
     *     select the specified filename.
     * \endlist
     *
     * \note Since 5.96, the "?global" syntax is deprecated, for lack of usage.
     *
     * \a parent The parent widget of this widget
     *
     */
    explicit KFileWidget(const QUrl &startDir, QWidget *parent = nullptr);

    ~KFileWidget() override;

    /*!
     * Defines some default behavior of the filedialog.
     *
     * E.g. in mode Opening and Saving, the selected files/urls will
     * be added to the "recent documents" list. The Saving mode also implies
     * setKeepLocation() being set.
     *
     * Other means that no default actions are performed.
     *
     * \value Other
     * \value Opening
     * \value Saving
     *
     * \sa setOperationMode
     * \sa operationMode
     */
    enum OperationMode {
        Other = 0,
        Opening,
        Saving
    };

    /*!
     * Returns The selected fully qualified filename.
     */
    QUrl selectedUrl() const;

    /*!
     * Returns The list of selected URLs.
     */
    QList<QUrl> selectedUrls() const;

    /*!
     * Returns the currently shown directory.
     */
    QUrl baseUrl() const;

    /*!
     * Returns the full path of the selected file in the local filesystem.
     * (Local files only)
     */
    QString selectedFile() const;

    /*!
     * Returns a list of all selected local files.
     */
    QStringList selectedFiles() const;

    /*!
     * Sets the directory to view.
     *
     * \a url URL to show.
     *
     * \a clearforward Indicates whether the forward queue
     * should be cleared.
     */
    void setUrl(const QUrl &url, bool clearforward = true);

    /*!
     * Sets the URL to preselect to \a url
     *
     * This method handles absolute URLs (remember to use fromLocalFile for local paths).
     * It also handles relative URLs, which you should construct like this:
     * QUrl relativeUrl; relativeUrl.setPath(fileName);
     *
     * \since 5.33
     */
    void setSelectedUrl(const QUrl &url);

    /*!
     * Sets a list of URLs as preselected
     *
     * \sa setSelectedUrl
     * \since 5.75
     */
    void setSelectedUrls(const QList<QUrl> &urls);

    /*!
     * Sets the operational mode of the filedialog to Saving, Opening
     * or Other. This will set some flags that are specific to loading
     * or saving files. E.g. setKeepLocation() makes mostly sense for
     * a save-as dialog. So setOperationMode( KFileWidget::Saving ); sets
     * setKeepLocation for example.
     *
     * The mode Saving, together with a default filter set via
     * setMimeFilter() will make the filter combobox read-only.
     *
     * The default mode is Opening.
     *
     * Call this method right after instantiating KFileWidget.
     *
     * \sa operationMode
     * \sa KFileWidget::OperationMode
     */
    void setOperationMode(OperationMode);

    /*!
     * Returns the current operation mode, Opening, Saving or Other. Default
     * is Other.
     *
     * \sa operationMode
     * \sa KFileWidget::OperationMode
     */
    OperationMode operationMode() const;

    /*!
     * Sets whether the filename/url should be kept when changing directories.
     * This is for example useful when having a predefined filename where
     * the full path for that file is searched.
     *
     * This is implicitly set when operationMode() is KFileWidget::Saving
     *
     * getSaveFileName() and getSaveUrl() set this to true by default, so that
     * you can type in the filename and change the directory without having
     * to type the name again.
     */
    void setKeepLocation(bool keep);

    /*!
     * Returns whether the contents of the location edit are kept when
     * changing directories.
     */
    bool keepsLocation() const;

    /*!
     * Set the filters to be used.
     *
     * Each item of the list corresponds to a selectable filter.
     *
     * Only one filter is active at a time.
     *
     * \a activeFilter the initially active filter
     *
     * \since 6.0
     */
    void setFilters(const QList<KFileFilter> &filters, const KFileFilter &activeFilter = KFileFilter());

    /*!
     * Returns the current filter as entered by the user or one of the
     * predefined set via setFilters().
     *
     * \sa setFilters()
     * \sa filterChanged()
     *
     * \since 6.0
     */
    KFileFilter currentFilter() const;

    /*!
     *  Clears any MIME type or name filter. Does not reload the directory.
     */
    void clearFilter();

    /*!
     * Adds a preview widget and enters the preview mode.
     *
     * In this mode the dialog is split and the right part contains your
     * preview widget.
     *
     * Ownership is transferred to KFileWidget. You need to create the
     * preview-widget with "new", i.e. on the heap.
     *
     * \a w The widget to be used for the preview.
     */
    void setPreviewWidget(KPreviewWidgetBase *w);

    /*!
     * Sets the mode of the dialog.
     *
     * The mode is defined as (in kfile.h):
     * \code
     *    enum Mode {
     *         File         = 1,
     *         Directory    = 2,
     *         Files        = 4,
     *         ExistingOnly = 8,
     *         LocalOnly    = 16,
     *    };
     * \endcode
     * You can OR the values, e.g.
     * \code
     * KFile::Modes mode = KFile::Files |
     *                     KFile::ExistingOnly |
     *                     KFile::LocalOnly );
     * setMode( mode );
     * \endcode
     */
    void setMode(KFile::Modes m);

    /*!
     * Returns the mode of the filedialog.
     * \sa setMode()
     */
    KFile::Modes mode() const;

    /*!
     * Sets the text to be displayed in front of the selection.
     *
     * The default is "Location".
     * Most useful if you want to make clear what
     * the location is used for.
     */
    void setLocationLabel(const QString &text);

    /*!
     * Returns a pointer to the OK-Button in the filedialog.
     * Note that the button is hidden and unconnected when using KFileWidget alone;
     * KFileDialog shows it and connects to it.
     */
    QPushButton *okButton() const;

    /*!
     * Returns a pointer to the Cancel-Button in the filedialog.
     * Note that the button is hidden and unconnected when using KFileWidget alone;
     * KFileDialog shows it and connects to it.
     */
    QPushButton *cancelButton() const;

    /*!
     * Returns the combobox used to type the filename or full location of the file.
     */
    KUrlComboBox *locationEdit() const;

    /*!
     * Returns the combobox that contains the filters
     */
    KFileFilterCombo *filterWidget() const;

    /*!
     * This method implements the logic to determine the user's default directory
     * to be listed. E.g. the documents directory, home directory or a recently
     * used directory.
     *
     * \a startDir A URL specifying the initial directory, or using the
     *                 \c kfiledialog:/// syntax to specify a last used
     *                 directory.  If this URL specifies a file name, it is
     *                 ignored.  Refer to the KFileWidget::KFileWidget()
     *                 documentation for the \c kfiledialog:/// URL syntax.
     *
     * \a recentDirClass If the \c kfiledialog:/// syntax is used, this
     *        will return the string to be passed to KRecentDirs::dir() and
     *        KRecentDirs::add().
     *
     * Returns The URL that should be listed by default (e.g. by KFileDialog or
     *         KDirSelectDialog).
     * \sa KFileWidget::KFileWidget()
     */
    static QUrl getStartUrl(const QUrl &startDir, QString &recentDirClass);

    /*!
     * Similar to getStartUrl(const QUrl& startDir,QString& recentDirClass),
     * but allows both the recent start directory keyword and a suggested file name
     * to be returned.
     *
     * \a startDir A URL specifying the initial directory and/or filename,
     *                 or using the \c kfiledialog:/// syntax to specify a
     *                 last used location.
     *                 Refer to the KFileWidget::KFileWidget()
     *                 documentation for the \c kfiledialog:/// URL syntax.
     *
     * \a recentDirClass If the \c kfiledialog:/// syntax is used, this
     *        will return the string to be passed to KRecentDirs::dir() and
     *        KRecentDirs::add().
     *
     * \a fileName The suggested file name, if specified as part of the
     *        \a startDir URL.
     *
     * Returns The URL that should be listed by default (e.g. by KFileDialog or
     *         KDirSelectDialog).
     *
     * \sa KFileWidget::KFileWidget()
     */
    static QUrl getStartUrl(const QUrl &startDir, QString &recentDirClass, QString &fileName);

    /*!
     * \internal
     * Used by KDirSelectDialog to share the dialog's start directory.
     */
    static void setStartDir(const QUrl &directory);

    /*!
     * Set a custom widget that should be added to the file dialog.
     *
     * \a widget A widget, or a widget of widgets, for displaying custom
     *               data in the file widget. This can be used, for example, to
     *               display a check box with the title "Open as read-only".
     *               When creating this widget, you don't need to specify a parent,
     *               since the widget's parent will be set automatically by KFileWidget.
     */
    void setCustomWidget(QWidget *widget);

    /*!
     * Sets a custom widget that should be added below the location and the filter
     * editors.
     *
     * \a text     Label of the custom widget, which is displayed below the labels
     *                 "Location:" and "Filter:".
     *
     * \a widget   Any kind of widget, but preferable a combo box or a line editor
     *                 to be compliant with the location and filter layout.
     *                 When creating this widget, you don't need to specify a parent,
     *                 since the widget's parent will be set automatically by KFileWidget.
     */
    void setCustomWidget(const QString &text, QWidget *widget);

    /*!
     * Sets whether the user should be asked for confirmation
     * when an overwrite might occur.
     *
     * \a enable Set this to true to enable checking.
     */
    void setConfirmOverwrite(bool enable);

    /*!
     * Forces the inline previews to be shown or hidden, depending on \a show.
     *
     * \a show Whether to show inline previews or not.
     */
    void setInlinePreviewShown(bool show);

    /*!
     * Provides a size hint, useful for dialogs that embed the widget.
     *
     * Returns a QSize, calculated to be optimal for a dialog.
     * \since 5.0
     */
    QSize dialogSizeHint() const;

    /*!
     * Sets how the view should be displayed.
     *
     * \sa KFile::FileView
     * \since 5.0
     */
    void setViewMode(KFile::FileView mode);

    QSize sizeHint() const override;

    /*!
     * Set the URL schemes that the file widget should allow navigating to.
     *
     * If the returned list is empty, all schemes are supported.
     *
     * \sa QFileDialog::setSupportedSchemes
     * \since 5.43
     */
    void setSupportedSchemes(const QStringList &schemes);

    /*!
     * Returns the URL schemes that the file widget should allow navigating to.
     *
     * If the returned list is empty, all schemes are supported. Examples for
     * schemes are \c "file" or \c "ftp".
     *
     * \sa QFileDialog::supportedSchemes
     * \since 5.43
     */
    QStringList supportedSchemes() const;

public Q_SLOTS:
    /*!
     * Called when clicking ok (when this widget is used in KFileDialog)
     * Might or might not call accept().
     */
    void slotOk();

    /*!
     *
     */
    void accept();

    /*!
     *
     */
    void slotCancel();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

Q_SIGNALS:
    /*!
     * Emitted when the user selects a file. It is only emitted in single-
     * selection mode. The best way to get notified about selected file(s)
     * is to connect to the okClicked() signal inherited from KDialog
     * and call selectedFile(), selectedFiles(),
     * selectedUrl() or selectedUrls().
     *
     * \since 4.4
     */
    void fileSelected(const QUrl &);

    /*!
     * Emitted when the user highlights a file.
     * \since 4.4
     */
    void fileHighlighted(const QUrl &);

    /*!
     * Emitted when the user highlights one or more files in multiselection mode.
     *
     * Note: fileHighlighted() or fileSelected() are @em not
     * emitted in multiselection mode. You may use selectedItems() to
     * ask for the current highlighted items.
     * \sa fileSelected
     */
    void selectionChanged();

    /*!
     * Emitted when the filter changed, i.e.\ the user entered an own filter
     * or chose one of the predefined set via setFilters().
     *
     * \a filter contains the new filter (only the extension part,
     * not the explanation), i.e. "*.cpp" or "*.cpp *.cc".
     *
     * \sa setFilters()
     * \sa currentFilter()
     *
     * \since 6.0
     */
    void filterChanged(const KFileFilter &filter);

    /*!
     * Emitted by slotOk() (directly or asynchronously) once everything has
     * been done. Should be used by the caller to call accept().
     */
    void accepted();

public:
    /*!
     * Returns the KDirOperator used to navigate the filesystem
     */
    KDirOperator *dirOperator();

#if KIOFILEWIDGETS_ENABLE_DEPRECATED_SINCE(6, 3)
    /*!
     * reads the configuration for this widget from the given config group
     * \a group the KConfigGroup to read from
     *
     * Deprecated: No known use case
     *
     * \deprecated[6.3]
     */
    KIOFILEWIDGETS_DEPRECATED_VERSION(6, 3, "No known use case")
    void readConfig(KConfigGroup &group);
#endif

private:
    friend class KFileWidgetPrivate;
    std::unique_ptr<KFileWidgetPrivate> const d;
};

#endif
