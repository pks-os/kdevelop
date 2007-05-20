/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "editorintegrator.h"
#include "editorintegrator_p.h"

#include <limits.h>

#include <QMutex>
#include <QMutexLocker>

#include <kglobal.h>

#include <ktexteditor/document.h>
#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>

#include "documentrange.h"
#include "documentrangeobject.h"

using namespace KTextEditor;

namespace KDevelop
{

K_GLOBAL_STATIC( EditorIntegratorPrivate, s_data)

EditorIntegrator::EditorIntegrator()
  : m_currentDocument(0)
  , m_smart(0)
  , m_currentRange(0)
{
}

EditorIntegrator::~ EditorIntegrator()
{
}

EditorIntegratorPrivate::EditorIntegratorPrivate()
  : mutex(new QMutex)
{
}

EditorIntegratorPrivate::~EditorIntegratorPrivate()
{
  QHashIterator<KUrl, QVector<KTextEditor::Range*> > it = topRanges;
  while (it.hasNext()) {
    it.next();
    foreach (KTextEditor::Range* range, it.value())
      if (range && range->isSmartRange())
        range->toSmartRange()->removeWatcher(this);
  }

  delete mutex;
}

void EditorIntegrator::addDocument( KTextEditor::Document * document )
{
  Q_ASSERT(data()->thread() == document->thread());
  QObject::connect(document, SIGNAL(completed()), data(), SLOT(documentLoaded()));
  QObject::connect(document, SIGNAL(aboutToClose(KTextEditor::Document*)), data(), SLOT(removeDocument(KTextEditor::Document*)));
  QObject::connect(document, SIGNAL(documentUrlChanged(KTextEditor::Document*)), data(), SLOT(documentUrlChanged(KTextEditor::Document*)));
}

void EditorIntegratorPrivate::documentLoaded()
{
  Document* doc = qobject_cast<Document*>(sender());
  if (!doc) {
    kWarning() << k_funcinfo << "Unexpected non-document sender called this slot!" << endl;
    return;
  }

  {
    QMutexLocker lock(mutex);

    documents.insert(doc->url(), doc);
  }
}

void EditorIntegratorPrivate::documentUrlChanged(KTextEditor::Document* document)
{
  QMutexLocker lock(mutex);

  QMutableHashIterator<KUrl, Document*>  it = documents;
  while (it.hasNext()) {
    it.next();
    if (it.value() == document) {
      if (topRanges.contains(it.key())) {
        kDebug() << k_funcinfo << "Document URL change - found corresponding document" << endl;
        topRanges.insert(document->url(), topRanges.take(it.key()));
      }

      it.remove();
      documents.insert(document->url(), document);
      // TODO trigger reparsing??
      return;
    }
  }

  //kWarning() << k_funcinfo << "Document URL change - couldn't find corresponding document!" << endl;
}

Document * EditorIntegrator::documentForUrl(const KUrl& url)
{
  QMutexLocker lock(data()->mutex);

  if (data()->documents.contains(url))
    return data()->documents[url];

  return 0;
}

bool EditorIntegrator::documentLoaded(KTextEditor::Document* document)
{
  return data()->documents.values().contains(document);
}

void EditorIntegratorPrivate::removeDocument( KTextEditor::Document* document )
{
  QMutexLocker lock(mutex);

  // TODO save smart stuff to non-smart cursors and ranges

  documents.remove(document->url());

  foreach (KTextEditor::Range* range, topRanges[document->url()])
    if (range && range->isSmartRange())
      range->toSmartRange()->removeWatcher(this);

  topRanges.remove(document->url());
}

SmartInterface* EditorIntegrator::smart() const
{
  return m_smart;
}

Cursor* EditorIntegrator::createCursor(const KTextEditor::Cursor& position)
{
  Cursor* ret = 0;

  if (SmartInterface* iface = smart()) {
    QMutexLocker lock(iface->smartMutex());
    ret = iface->newSmartCursor(position);
  }

  if (!ret)
    ret = new DocumentCursor(m_currentUrl, position);

  return ret;
}

Document* EditorIntegrator::currentDocument() const
{
  return m_currentDocument;
}

Range* EditorIntegrator::topRange( TopRangeType type )
{
  QMutexLocker lock(data()->mutex);

  if (!data()->topRanges.contains(currentUrl()))
    data()->topRanges.insert(currentUrl(), QVector<Range*>(TopRangeCount));

  // FIXME temporary until we get conversion working
  if (data()->topRanges[currentUrl()][type] && !data()->topRanges[currentUrl()][type]->isSmartRange() && smart()) {
    //delete data()->topRanges[currentUrl()][type];
    data()->topRanges[currentUrl()][type] = 0L;
  }

  if (!data()->topRanges[currentUrl()][type])
    if (currentDocument()) {
      Range* newRange = data()->topRanges[currentUrl()][type] = createRange(currentDocument()->documentRange());
      if (SmartInterface* iface = smart()) {
        QMutexLocker lock(iface->smartMutex());
        Q_ASSERT(newRange->isSmartRange());
        iface->addHighlightToDocument( newRange->toSmartRange(), false );
        newRange->toSmartRange()->addWatcher(data());
      }

    } else {
      // FIXME...
      data()->topRanges[currentUrl()][type] = createRange(Range(0,0, INT_MAX, INT_MAX));
    }

  m_currentRange = data()->topRanges[currentUrl()][type];
  return m_currentRange;
}

void EditorIntegratorPrivate::rangeDeleted(KTextEditor::SmartRange * range)
{
  QMutexLocker lock(mutex);

  QMutableHashIterator<KUrl, QVector<KTextEditor::Range*> > it = topRanges;
  while (it.hasNext()) {
    it.next();
    //kDebug() << k_funcinfo << "Searching for " << range << ", potentials " << it.value().toList() << endl;
    int index = it.value().indexOf(range);
    if (index != -1) {
      it.value()[index] = 0;
      return;
    }
  }

  // Should have found the top level range by now
  kWarning() << k_funcinfo << "Could not find record of top level range " << range << "!" << endl;
}

Range* EditorIntegrator::createRange( const KTextEditor::Range & range )
{
  Range* ret;

  if (SmartInterface* iface = smart()) {
    QMutexLocker lock(iface->smartMutex());

    if (m_currentRange && m_currentRange->isSmartRange())
      ret = iface->newSmartRange(range, m_currentRange->toSmartRange());
    else
      ret = iface->newSmartRange(range);

  } else {
    ret = new DocumentRange(m_currentUrl, range, m_currentRange);
  }

  m_currentRange = ret;
  return m_currentRange;
}


Range* EditorIntegrator::createRange( const KTextEditor::Cursor& start, const KTextEditor::Cursor& end )
{
  return createRange(Range(start, end));
}

Range* EditorIntegrator::createRange()
{
  return createRange(m_newRangeMarker);
}

void EditorIntegrator::setNewRange(const KTextEditor::Range& range)
{
  m_newRangeMarker = range;
}

void EditorIntegrator::setNewEnd( const KTextEditor::Cursor & position )
{
  m_newRangeMarker.end() = position;
}

void EditorIntegrator::setNewStart( const KTextEditor::Cursor & position )
{
  m_newRangeMarker.start() = position;
}

void EditorIntegrator::setCurrentRange( KTextEditor::Range* range )
{
  m_currentRange = range;
}

Range* EditorIntegrator::currentRange( ) const
{
  return m_currentRange;
}

KUrl EditorIntegrator::currentUrl() const
{
  return m_currentUrl;
}

void EditorIntegrator::setCurrentUrl(const KUrl& url)
{
  m_currentUrl = url;
  m_currentDocument = documentForUrl(url);
  m_smart = dynamic_cast<KTextEditor::SmartInterface*>(m_currentDocument);
}

void EditorIntegrator::releaseTopRange(KTextEditor::Range * range)
{
  QMutexLocker lock(data()->mutex);

  KUrl url = DocumentRangeObject::url(range);

  if (range->isSmartRange())
    range->toSmartRange()->removeWatcher(data());

  if (data()->topRanges.contains(url)) {
    QVector<Range*>& ranges = data()->topRanges[url];
    int index = ranges.indexOf(range);
    if (index != -1) {
      ranges[index] = 0;
      return;
    }
  }

  //kWarning() << k_funcinfo << "Could not find top range to delete." << endl;
}

void EditorIntegrator::releaseRange(KTextEditor::Range* range)
{
  if (range) {
    if (range->isSmartRange()) {
      if (SmartInterface* iface = dynamic_cast<SmartInterface*>(range->toSmartRange()->document())) {
        QMutexLocker lock(iface->smartMutex());
        delete range;
      } else {
        delete range;
      }
    } else {
      delete range;
    }
  }
}

KDevelop::EditorIntegratorPrivate * EditorIntegrator::data()
{
  return s_data;
}

void EditorIntegrator::exitCurrentRange()
{
  if (!m_currentRange)
    return;

  if (m_currentRange->isSmartRange())
    m_currentRange = m_currentRange->toSmartRange()->parentRange();
  else
    m_currentRange = static_cast<DocumentRange*>(m_currentRange)->parentRange();
}

}
#include "editorintegrator_p.moc"

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
