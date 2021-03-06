/* This file is part of KDevelop
    Copyright 2010 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_DOCUMENTRANGE_H
#define KDEVPLATFORM_DOCUMENTRANGE_H

#include <language/languageexport.h>
#include <serialization/indexedstring.h>

#include <KTextEditor/Range>

namespace KDevelop {
/**
 * Lightweight object that extends a range with information about the URL to which the range refers.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DocumentRange
    : public KTextEditor::Range
{
public:
    DocumentRange()
    {
    }

    inline DocumentRange(const IndexedString& document, const KTextEditor::Range& range)
        : KTextEditor::Range(range)
        , document(document)
    {
        Q_ASSERT(document.toUrl() == document.toUrl().adjusted(QUrl::NormalizePathSegments));
    }

    inline bool operator==(const DocumentRange& rhs) const
    {
        return document == rhs.document && *static_cast<const KTextEditor::Range*>(this) == rhs;
    }

    static DocumentRange invalid()
    {
        return DocumentRange(IndexedString(), KTextEditor::Range::invalid());
    }

    IndexedString document;
};
}
Q_DECLARE_TYPEINFO(KDevelop::DocumentRange, Q_MOVABLE_TYPE);

namespace QTest {
template <>
inline char* toString(const KDevelop::DocumentRange& documentRange)
{
    QByteArray ba = "DocumentRange[range=" +
                    QByteArray(QTest::toString(*static_cast<const KTextEditor::Range*>(&documentRange)))
                    + ", document=" + documentRange.document.toUrl().toDisplayString().toLatin1()
                    + "]";
    return qstrdup(ba.data());
}
}

#endif // KDEVPLATFORM_DOCUMENTRANGE_H
