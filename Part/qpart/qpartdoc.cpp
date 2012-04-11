/***************************************************************************
                          qpartdoc.cpp  -  description
                             -------------------
    begin                : Sat Mar 15 17:26:45 PST 2003
    copyright            : (C) 2003 by Wai-Shing Luk
    email                : waishing_luk@ieee.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qpartdoc.h"
#include <Netlist.hpp>

QPartDoc::QPartDoc()
{
  modified = false;
  m_H = new Netlist;  // luk
}

QPartDoc::~QPartDoc()
{
  delete m_H;  // luk
}

void QPartDoc::newDoc()
{
}

bool QPartDoc::save()
{
  return true;
}

bool QPartDoc::saveAs(const QString &)
{
  return true;
}

bool QPartDoc::load(const QString &filename)
{
  emit documentChanged();
  return m_H->readNetD(filename);  // luk
}

bool QPartDoc::isModified() const
{
  return modified;
}
