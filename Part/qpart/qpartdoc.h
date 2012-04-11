/***************************************************************************
                          qpartdoc.h  -  description
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
#ifndef QPARTDOC_H
#define QPARTDOC_H

// include files for QT
#include <qobject.h>

class Netlist;

// application specific includes

/**
  * the Document Class
  */

class QPartDoc : public QObject
{
  Q_OBJECT

  public:
    QPartDoc();
    ~QPartDoc();
    void newDoc();
    bool save();
    bool saveAs(const QString &filename);
    bool load(const QString &filename);
    bool isModified() const;
    Netlist* getNetList() { return m_H; }

  signals:
    void documentChanged();

  protected:
    bool modified;
    Netlist* m_H;
};

#endif
