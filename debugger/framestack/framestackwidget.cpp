/*
 * This file is part of KDevelop
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
 * Copyright 2009 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "framestackwidget.h"

#include <QHeaderView>
#include <QScrollBar>
#include <QListView>
#include <QVBoxLayout>
#include <QLabel>
#include <QTreeView>

#include <KDebug>
#include <KLocalizedString>
#include <KIcon>
#include <KTextEditor/Cursor>

#include "../../interfaces/icore.h"
#include "../../interfaces/idebugcontroller.h"
#include "../../interfaces/idocumentcontroller.h"
#include "../interfaces/istackcontroller.h"
#include "framestackmodel.h"

namespace KDevelop {

FramestackWidget::FramestackWidget(IDebugController* controller, QWidget* parent)
    : QSplitter(Qt::Horizontal, parent), m_controller(controller)
{
    connect(controller, SIGNAL(sessionAdded(KDevelop::IDebugSession*)), SLOT(sessionAdded(KDevelop::IDebugSession*)));
    connect(controller, SIGNAL(raiseFramestackViews()), SIGNAL(requestRaise()));

    setWhatsThis(i18n("<b>Frame stack</b><p>"
                    "Often referred to as the \"call stack\", "
                    "this is a list showing which function is "
                    "currently active, and what called each "
                    "function to get to this point in your "
                    "program. By clicking on an item you "
                    "can see the values in any of the "
                    "previous calling functions.</p>"));
    setWindowIcon(KIcon("view-list-text"));
    m_threadsWidget = new QWidget(this);
    m_threads = new QListView(m_threadsWidget);
    m_threads->setModel(controller->frameStackModel());
    m_frames = new QTreeView(this);
    m_frames->setRootIsDecorated(false);
    m_frames->setModel(controller->frameStackModel());

    m_threadsWidget->setLayout(new QVBoxLayout());
    m_threadsWidget->layout()->addWidget(new QLabel(i18n("Threads:")));
    m_threadsWidget->layout()->addWidget(m_threads);
    m_threadsWidget->hide();
    addWidget(m_threadsWidget);
    addWidget(m_frames);

    setStretchFactor(1, 3);
    connect(m_frames->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(checkFetchMoreFrames()));
    connect(m_threads, SIGNAL(clicked(QModelIndex)), this, SLOT(setThreadShown(QModelIndex)));
    connect(m_frames, SIGNAL(clicked(QModelIndex)), SLOT(openFile(QModelIndex)));
}

FramestackWidget::~FramestackWidget() {}

void FramestackWidget::sessionAdded(KDevelop::IDebugSession* session)
{
    Q_UNUSED(session);
    kDebug() << "Adding session:" << isVisible();
    if (isVisible()) {
        showEvent(0);
    }
}

void KDevelop::FramestackWidget::hideEvent(QHideEvent* e)
{
    QWidget::hideEvent(e);
    m_controller->frameStackModel()->setAutoUpdate(false);
}

void KDevelop::FramestackWidget::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);

    m_controller->frameStackModel()->setAutoUpdate(true);

    IDebugSession *session = m_controller->currentSession();
    if (session && session->state() != KDevelop::IDebugSession::EndedState) {
        connect(m_controller->frameStackModel(), SIGNAL(rowsInserted(QModelIndex, int, int)),
                SLOT(assignSomeThread()));
        assignSomeThread();
    }
}

void KDevelop::FramestackWidget::setThreadShown(const QModelIndex& idx)
{
    m_frames->setRootIndex(idx);
    m_controller->frameStackModel()->setActiveThread(idx);
}

void KDevelop::FramestackWidget::checkFetchMoreFrames()
{
    int val = m_frames->verticalScrollBar()->value();
    int max = m_frames->verticalScrollBar()->maximum();
    const int offset = 20;

    kDebug() << val << max << m_frames->verticalScrollBar()->minimum();
    if (val + offset > max) {
        m_controller->frameStackModel()->fetchMoreFrames();
    }
}

void KDevelop::FramestackWidget::assignSomeThread()
{
    FrameStackModel* model = m_controller->frameStackModel();
    if (model->activeThread() && m_threads->selectionModel()->selectedRows().isEmpty()) {
        QModelIndex idx = model->activeThreadIndex();
        m_threads->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        setThreadShown(idx);
    }
    if (model->rowCount() <= 1) {
        m_threadsWidget->hide();
    } else {
        m_threadsWidget->show();
    }
}

void FramestackWidget::openFile(const QModelIndex& idx)
{
    FrameStackModel::FrameItem f = m_controller->frameStackModel()->frame(idx);
    /* If line is -1, then it's not a source file at all.  */
    if (f.line != -1)
        ICore::self()->documentController()->openDocument(f.file, KTextEditor::Cursor(f.line, 0));
}

}

#include "framestackwidget.moc"
