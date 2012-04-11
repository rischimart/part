/***************************************************************************
                          qpart.cpp  -  description
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qaccel.h>
#include "qpart.h"
#include "filesave.xpm"
#include "fileopen.xpm"
#include "filenew.xpm"
#include "constraints.h"
#include <MlPartTmpl.hpp>
#include <FMParam.hpp>
#include <FMPartTmpl.hpp>
#include <FMPartTmpl4.hpp>
#include <FMBiPartCore.hpp>
#include <FMBiPartCore4.hpp>
#include <FMBiGainMgr.hpp>
#include <FMBiGainMgr2.hpp>
#include <Netlist.hpp>
#include <qmessagebox.h>

QPartApp::QPartApp()
{
  setCaption(trUtf8("QPart 0.1"));

  setGeometry(100,100,800,600);

  ///////////////////////////////////////////////////////////////////
  // call inits to invoke all other construction parts
  initActions();
  initMenuBar();
  initToolBar();
  initStatusBar();

  initDoc();
  initView();

  viewToolBar->setOn(true);
  viewStatusBar->setOn(true);
}

QPartApp::~QPartApp()
{
}

/** initializes all QActions of the application */
void QPartApp::initActions(){

  QPixmap openIcon, saveIcon, newIcon;
  newIcon = QPixmap(filenew);
  openIcon = QPixmap(fileopen);
  saveIcon = QPixmap(filesave);


  fileNew = new QAction(tr("New File"), newIcon, tr("&New"), QAccel::stringToKey(tr("Ctrl+N")), this);
  fileNew->setStatusTip(tr("Creates a new document"));
  fileNew->setWhatsThis(tr("New File\n\nCreates a new document"));
  connect(fileNew, SIGNAL(activated()), this, SLOT(slotFileNew()));

  fileOpen = new QAction(tr("Open File"), openIcon, tr("&Open..."), 0, this);
  fileOpen->setStatusTip(tr("Opens an existing document"));
  fileOpen->setWhatsThis(tr("Open File\n\nOpens an existing document"));
  connect(fileOpen, SIGNAL(activated()), this, SLOT(slotFileOpen()));

  fileSave = new QAction(tr("Save File"), saveIcon, tr("&Save"), QAccel::stringToKey(tr("Ctrl+S")), this);
  fileSave->setStatusTip(tr("Saves the actual document"));
  fileSave->setWhatsThis(tr("Save File.\n\nSaves the actual document"));
  connect(fileSave, SIGNAL(activated()), this, SLOT(slotFileSave()));

  fileSaveAs = new QAction(tr("Save File As"), tr("Save &as..."), 0, this);
  fileSaveAs->setStatusTip(tr("Saves the actual document under a new filename"));
  fileSaveAs->setWhatsThis(tr("Save As\n\nSaves the actual document under a new filename"));
  connect(fileSaveAs, SIGNAL(activated()), this, SLOT(slotFileSave()));

  fileClose = new QAction(tr("Close File"), tr("&Close"), QAccel::stringToKey(tr("Ctrl+W")), this);
  fileClose->setStatusTip(tr("Closes the actual document"));
  fileClose->setWhatsThis(tr("Close File\n\nCloses the actual document"));
  connect(fileClose, SIGNAL(activated()), this, SLOT(slotFileClose()));

  filePrint = new QAction(tr("Print File"), tr("&Print"), QAccel::stringToKey(tr("Ctrl+P")), this);
  filePrint->setStatusTip(tr("Prints out the actual document"));
  filePrint->setWhatsThis(tr("Print File\n\nPrints out the actual document"));
  connect(filePrint, SIGNAL(activated()), this, SLOT(slotFilePrint()));

  fileQuit = new QAction(tr("Exit"), tr("E&xit"), QAccel::stringToKey(tr("Ctrl+Q")), this);
  fileQuit->setStatusTip(tr("Quits the application"));
  fileQuit->setWhatsThis(tr("Exit\n\nQuits the application"));
  connect(fileQuit, SIGNAL(activated()), this, SLOT(slotFileQuit()));

  editCut = new QAction(tr("Cut"), tr("Cu&t"), QAccel::stringToKey(tr("Ctrl+X")), this);
  editCut->setStatusTip(tr("Cuts the selected section and puts it to the clipboard"));
  editCut->setWhatsThis(tr("Cut\n\nCuts the selected section and puts it to the clipboard"));
  connect(editCut, SIGNAL(activated()), this, SLOT(slotEditCut()));

  editCopy = new QAction(tr("Copy"), tr("&Copy"), QAccel::stringToKey(tr("Ctrl+C")), this);
  editCopy->setStatusTip(tr("Copies the selected section to the clipboard"));
  editCopy->setWhatsThis(tr("Copy\n\nCopies the selected section to the clipboard"));
  connect(editCopy, SIGNAL(activated()), this, SLOT(slotEditCopy()));

  editPaste = new QAction(tr("Paste"), tr("&Paste"), QAccel::stringToKey(tr("Ctrl+V")), this);
  editPaste->setStatusTip(tr("Pastes the clipboard contents to actual position"));
  editPaste->setWhatsThis(tr("Paste\n\nPastes the clipboard contents to actual position"));
  connect(editPaste, SIGNAL(activated()), this, SLOT(slotEditPaste()));

  partConstraints = new QAction(tr("Constraints"), tr("&Constraints"), 0, this);
  partConstraints->setStatusTip(tr("Setup balance constraints"));
  partConstraints->setWhatsThis(tr("Constraints\n\nSetup balance constraints"));
  connect(partConstraints, SIGNAL(activated()), this, SLOT(slotPartConstraints()));

  partRun = new QAction(tr("Run"), tr("&Run"), 0, this);
  partRun->setStatusTip(tr("Run the partition"));
  partRun->setWhatsThis(tr("Run\n\nRun the partition"));
  connect(partRun, SIGNAL(activated()), this, SLOT(slotPartRun()));

  viewToolBar = new QAction(tr("Toolbar"), tr("Tool&bar"), 0, this, 0, true);
  viewToolBar->setStatusTip(tr("Enables/disables the toolbar"));
  viewToolBar->setWhatsThis(tr("Toolbar\n\nEnables/disables the toolbar"));
  connect(viewToolBar, SIGNAL(toggled(bool)), this, SLOT(slotViewToolBar(bool)));

  viewStatusBar = new QAction(tr("Statusbar"), tr("&Statusbar"), 0, this, 0, true);
  viewStatusBar->setStatusTip(tr("Enables/disables the statusbar"));
  viewStatusBar->setWhatsThis(tr("Statusbar\n\nEnables/disables the statusbar"));
  connect(viewStatusBar, SIGNAL(toggled(bool)), this, SLOT(slotViewStatusBar(bool)));

  helpAboutApp = new QAction(tr("About"), tr("&About..."), 0, this);
  helpAboutApp->setStatusTip(tr("About the application"));
  helpAboutApp->setWhatsThis(tr("About\n\nAbout the application"));
  connect(helpAboutApp, SIGNAL(activated()), this, SLOT(slotHelpAbout()));

}

void QPartApp::initMenuBar()
{
  ///////////////////////////////////////////////////////////////////
  // MENUBAR

  ///////////////////////////////////////////////////////////////////
  // menuBar entry fileMenu
  fileMenu=new QPopupMenu();
  fileNew->addTo(fileMenu);
  fileOpen->addTo(fileMenu);
  fileClose->addTo(fileMenu);
  fileMenu->insertSeparator();
  fileSave->addTo(fileMenu);
  fileSaveAs->addTo(fileMenu);
  fileMenu->insertSeparator();
  filePrint->addTo(fileMenu);
  fileMenu->insertSeparator();
  fileQuit->addTo(fileMenu);

  ///////////////////////////////////////////////////////////////////
  // menuBar entry editMenu
  editMenu=new QPopupMenu();
  editCut->addTo(editMenu);
  editCopy->addTo(editMenu);
  editPaste->addTo(editMenu);

  ///////////////////////////////////////////////////////////////////
  // menuBar entry partMenu
  partMenu=new QPopupMenu();
  partConstraints->addTo(partMenu);
  partRun->addTo(partMenu);

  ///////////////////////////////////////////////////////////////////
  // menuBar entry viewMenu
  viewMenu=new QPopupMenu();
  viewMenu->setCheckable(true);
  viewToolBar->addTo(viewMenu);
  viewStatusBar->addTo(viewMenu);
  ///////////////////////////////////////////////////////////////////
  // EDIT YOUR APPLICATION SPECIFIC MENUENTRIES HERE

  ///////////////////////////////////////////////////////////////////
  // menuBar entry helpMenu
  helpMenu=new QPopupMenu();
  helpAboutApp->addTo(helpMenu);

  ///////////////////////////////////////////////////////////////////
  // MENUBAR CONFIGURATION
  menuBar()->insertItem(tr("&File"), fileMenu);
  menuBar()->insertItem(tr("&Edit"), editMenu);
  menuBar()->insertItem(tr("&Partition"), partMenu);
  menuBar()->insertItem(tr("&View"), viewMenu);
  menuBar()->insertSeparator();
  menuBar()->insertItem(tr("&Help"), helpMenu);

}

void QPartApp::initToolBar()
{
  ///////////////////////////////////////////////////////////////////
  // TOOLBAR
  fileToolbar = new QToolBar(this, "file operations");
  fileNew->addTo(fileToolbar);
  fileOpen->addTo(fileToolbar);
  fileSave->addTo(fileToolbar);
  fileToolbar->addSeparator();
  QWhatsThis::whatsThisButton(fileToolbar);

}

void QPartApp::initStatusBar()
{
  ///////////////////////////////////////////////////////////////////
  //STATUSBAR
  statusBar()->message(tr("Ready."), 2000);
}

void QPartApp::initDoc()
{
   doc=new QPartDoc();
}

void QPartApp::initView()
{
  ////////////////////////////////////////////////////////////////////
  // set the main widget here
  view=new QPartView(this, doc);
  setCentralWidget(view);
}

bool QPartApp::queryExit()
{
  int exit=QMessageBox::information(this, tr("Quit..."),
                                    tr("Do your really want to quit?"),
                                    QMessageBox::Ok, QMessageBox::Cancel);

  if (exit==1)
  {

  }
  else
  {

  };

  return (exit==1);
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////


void QPartApp::slotFileNew()
{
  statusBar()->message(tr("Creating new file..."));
  doc->newDoc();
  statusBar()->message(tr("Ready."));
}

void QPartApp::slotFileOpen()
{
  statusBar()->message(tr("Opening file..."));

  QString fileName = QFileDialog::getOpenFileName(0,"IBM netD (*.netD)",this);
  if (!fileName.isEmpty())
  {
    doc->load(fileName);
    setCaption(fileName);
    QString message=tr("Loaded document: ")+fileName;
    statusBar()->message(message, 2000);
  }
  else
  {
    statusBar()->message(tr("Opening aborted"), 2000);
  }
}


void QPartApp::slotFileSave()
{
  statusBar()->message(tr("Saving file..."));
  doc->save();
  statusBar()->message(tr("Ready."));
}

void QPartApp::slotFileSaveAs()
{
  statusBar()->message(tr("Saving file under new filename..."));
  QString fn = QFileDialog::getSaveFileName(0, 0, this);
  if (!fn.isEmpty())
  {
    doc->saveAs(fn);
  }
  else
  {
    statusBar()->message(tr("Saving aborted"), 2000);
  }

  statusBar()->message(tr("Ready."));
}

void QPartApp::slotFileClose()
{
  statusBar()->message(tr("Closing file..."));

  statusBar()->message(tr("Ready."));
}

void QPartApp::slotFilePrint()
{
  statusBar()->message(tr("Printing..."));
  QPrinter printer;
  if (printer.setup(this))
  {
    QPainter painter;
    painter.begin(&printer);

    ///////////////////////////////////////////////////////////////////
    // TODO: Define printing by using the QPainter methods here

    painter.end();
  };

  statusBar()->message(tr("Ready."));
}

void QPartApp::slotFileQuit()
{
  statusBar()->message(tr("Exiting application..."));
  ///////////////////////////////////////////////////////////////////
  // exits the Application
  if(doc->isModified())
  {
    if(queryExit())
    {
      qApp->quit();
    }
    else
    {

    };
  }
  else
  {
    qApp->quit();
  };

  statusBar()->message(tr("Ready."));
}

void QPartApp::slotEditCut()
{
  statusBar()->message(tr("Cutting selection..."));

  statusBar()->message(tr("Ready."));
}

void QPartApp::slotEditCopy()
{
  statusBar()->message(tr("Copying selection to clipboard..."));

  statusBar()->message(tr("Ready."));
}

void QPartApp::slotEditPaste()
{
  statusBar()->message(tr("Inserting clipboard contents..."));

  statusBar()->message(tr("Ready."));
}

void QPartApp::slotPartConstraints()
{
  statusBar()->message(tr("Setup constraints..."));
  Constraints* pConstraintDlg = new Constraints;
  pConstraintDlg->show();

  statusBar()->message(tr("Ready."));
}

void QPartApp::slotPartRun()
{
  statusBar()->message(tr("Running Partition..."));
  Netlist* H = doc->getNetList();

  typedef
  MlPartTmpl
  < FMBiPartMgrBase, 
    FMPartTmpl<FMBiPartCore, FMBiGainMgr2>, 
    FMPartTmpl4<FMBiPartCore4, FMBiGainMgr2> 
  > MlBiPartMgr;

  const FMParam param(*H,2,0);
  MlBiPartMgr aPartMgr(param);
  //xxx int* part = new int[H->getNumCells()];
  std::vector<unsigned char> part(H->getNumCells());

  //xxx aPartMgr.initPart();
  aPartMgr.setBalanceTol(0.1); // this value should be input from user
  aPartMgr.doPartitionQt(part, 10);
  int cost = H->cutCost(part, aPartMgr.getNumPartitions());
  QMessageBox::information(this, "Result of Partitioning",
			   "Cut cost = " + QString::number(cost));
  //xxx delete [] part; 
  statusBar()->message(tr("Ready."));
}

void QPartApp::slotViewToolBar(bool toggle)
{
  statusBar()->message(tr("Toggle toolbar..."));
  ///////////////////////////////////////////////////////////////////
  // turn Toolbar on or off

  if (toggle== false)
  {
    fileToolbar->hide();
  }
  else
  {
    fileToolbar->show();
  };

  statusBar()->message(tr("Ready."));
}

void QPartApp::slotViewStatusBar(bool toggle)
{
  statusBar()->message(tr("Toggle statusbar..."));
  ///////////////////////////////////////////////////////////////////
  //turn Statusbar on or off

  if (toggle == false)
  {
    statusBar()->hide();
  }
  else
  {
    statusBar()->show();
  }

  statusBar()->message(tr("Ready."));
}

void QPartApp::slotHelpAbout()
{
  QMessageBox::about(this,tr("About..."),
                      tr("QPart\nVersion 0.1\n(c) 2003 by Wai-Shing Luk") );
}

