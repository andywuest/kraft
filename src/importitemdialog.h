/***************************************************************************
  importitemdialog.h  - small dialog to import items into the document
                             -------------------
    begin                : Nov 2008
    copyright            : (C) 2008 Klaas Freitag
    email                : freitag@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef IMPORTITEMDIALOG_H
#define IMPORTITEMDIALOG_H

#include <kdialogbase.h>
#include <qmap.h>

#include "templtopositiondialogbase.h"
#include "docposition.h"
#include "importfilter.h"

class importToDocBase;

class ImportItemDialog: public KDialogBase
{
  Q_OBJECT

public:
  ImportItemDialog( QWidget* );
  ~ImportItemDialog();

  void setPositionList( DocPositionList, int );
  QValueList<DocPosition> positionList();
  
  QComboBox *getPositionCombo();

signals:
  void positionImported( const DocPosition& );

public slots:
  void slotOk();

protected slots:
  void slotSchemaChanged( const QString& );

protected:
  QString readFilterSpecs();

private:
  importToDocBase *mBaseWidget;
  QMap<QString, DocPositionImportFilter> mFilterMap;
  QMap<int, QString> mTagMap;
};

#endif
