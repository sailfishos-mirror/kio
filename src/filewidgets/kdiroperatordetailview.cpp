/*
    SPDX-FileCopyrightText: 2007 Peter Penz <peter.penz@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kdiroperatordetailview_p.h"
#include "kfileitemselectionemblem.h"

#include <kdirlister.h>
#include <kdirmodel.h>

#include <QApplication>
#include <QDragEnterEvent>
#include <QEvent>
#include <QHeaderView>
#include <QListView>
#include <QMimeData>
#include <QScrollBar>

KDirOperatorDetailView::KDirOperatorDetailView(KDirOperator *dirOperator, QWidget *parent)
    : QTreeView(parent)
    , m_hideDetailColumns(false)
    , m_isEmblemClicked(false)
    , m_dirOperator(dirOperator)
{
    setRootIsDecorated(false);
    setSortingEnabled(true);
    setUniformRowHeights(true);
    setDragDropMode(QListView::DragOnly);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QListView::ScrollPerPixel);
    setHorizontalScrollMode(QListView::ScrollPerPixel);

    const QFontMetrics metrics(viewport()->font());
    const int singleStep = metrics.height() * QApplication::wheelScrollLines();

    verticalScrollBar()->setSingleStep(singleStep);
    horizontalScrollBar()->setSingleStep(singleStep);
}

KDirOperatorDetailView::~KDirOperatorDetailView()
{
}

bool KDirOperatorDetailView::setViewMode(KFile::FileView viewMode)
{
    bool tree = false;

    if (KFile::isDetailView(viewMode)) {
        m_hideDetailColumns = false;
        setAlternatingRowColors(true);
    } else if (KFile::isTreeView(viewMode)) {
        m_hideDetailColumns = true;
        setAlternatingRowColors(false);
        tree = true;
    } else if (KFile::isDetailTreeView(viewMode)) {
        m_hideDetailColumns = false;
        setAlternatingRowColors(true);
        tree = true;
    } else {
        return false;
    }

    setRootIsDecorated(tree);
    setItemsExpandable(tree);
    // This allows to have a horizontal scrollbar in case this view is used as
    // a plain treeview instead of cutting off filenames, especially useful when
    // using KDirOperator in horizontally limited parts of an app.
    if (tree && m_hideDetailColumns) {
        header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    } else {
        header()->setSectionResizeMode(QHeaderView::Interactive);
    }

    return true;
}

void KDirOperatorDetailView::initViewItemOption(QStyleOptionViewItem *option) const
{
    QTreeView::initViewItemOption(option);
    option->textElideMode = Qt::ElideMiddle;
}

bool KDirOperatorDetailView::event(QEvent *event)
{
    if (event->type() == QEvent::Polish) {
        QHeaderView *headerView = header();
        headerView->setSectionResizeMode(0, QHeaderView::Stretch);
        headerView->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        headerView->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        headerView->setStretchLastSection(false);
        headerView->setSectionsMovable(false);

        setColumnHidden(KDirModel::Size, m_hideDetailColumns);
        setColumnHidden(KDirModel::ModifiedTime, m_hideDetailColumns);
        hideColumn(KDirModel::Type);
        hideColumn(KDirModel::Permissions);
        hideColumn(KDirModel::Owner);
        hideColumn(KDirModel::Group);
    } else if (event->type() == QEvent::UpdateRequest) {
        // A wheel movement will scroll 4 items
        if (model()->rowCount()) {
            verticalScrollBar()->setSingleStep((sizeHintForRow(0) / 3) * 4);
        }
    }

    return QTreeView::event(event);
}

void KDirOperatorDetailView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void KDirOperatorDetailView::mousePressEvent(QMouseEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    // When selection emblem is clicked, select it and don't do anything else
    m_isEmblemClicked = KFileItemSelectionEmblem(this, index, m_dirOperator).handleMousePressEvent(event->pos());
    if (m_isEmblemClicked) {
        return;
    }

    QTreeView::mousePressEvent(event);

    if (!index.isValid() || (index.column() != KDirModel::Name)) {
        const Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
        if (!(modifiers & Qt::ShiftModifier) && !(modifiers & Qt::ControlModifier)) {
            clearSelection();
        }
    }
}

void KDirOperatorDetailView::mouseMoveEvent(QMouseEvent *event)
{
    // Disallow selection dragging when emblem is clicked
    if (m_isEmblemClicked) {
        return;
    }
    QTreeView::mouseMoveEvent(event);
}

void KDirOperatorDetailView::mouseReleaseEvent(QMouseEvent *event)
{
    // Reset the emblem selection
    if (m_isEmblemClicked) {
        m_isEmblemClicked = false;
    }
    QTreeView::mouseReleaseEvent(event);
}

void KDirOperatorDetailView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);
}

#include "moc_kdiroperatordetailview_p.cpp"
