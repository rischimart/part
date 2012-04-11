/***************************************************************************
                          qpartview.h  -  description
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

#ifndef QPARTVIEW_H
#define QPARTVIEW_H

// include files for QT
#include <qwidget.h>

// application specific includes
#include "qpartdoc.h"

/**
 * This class provides an incomplete base for your application view. 
 */

class QPartView : public QWidget
{
  Q_OBJECT
  public:
    QPartView(QWidget *parent=0, QPartDoc* doc=0);
    ~QPartView();
  
  protected slots:
    void slotDocumentChanged();
  
};

#endif
