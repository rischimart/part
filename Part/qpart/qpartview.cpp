/***************************************************************************
                          qpartview.cpp  -  description
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

#include "qpartview.h"

QPartView::QPartView(QWidget *parent, QPartDoc *doc) : QWidget(parent)
{
  /** connect doc with the view*/
  connect(doc, SIGNAL(documentChanged()), this, SLOT(slotDocumentChanged()));
}

QPartView::~QPartView()
{
}

void QPartView::slotDocumentChanged()
{
  //TODO update the view

}
