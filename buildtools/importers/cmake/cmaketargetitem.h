/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKETARGETITEM_H
#define CMAKETARGETITEM_H

#include <kdevprojectmodel.h>

#include "cmTarget.h"
#include "cmMakefile.h"
/**
 * The project model item for CMake targets.
 *
 * @author Matt Rogers <mattr@kde.org>
 */
class CMakeTargetItem : public KDevProjectTargetItem
{
public:
    CMakeTargetItem( cmTarget target, KDevProjectItem* item );
    ~CMakeTargetItem();

    virtual const DomUtil::PairList& defines() const;
    virtual const KUrl::List& includeDirectories() const;
    virtual const QHash< QString, QString >& environment() const;

private:
    cmTarget m_target;
    KUrl::List m_includeList;
    DomUtil::PairList m_defines;
    QHash<QString, QString> m_environment;

};

#endif
