/***************************************************************************
                          kraftdoc.h  -
                             -------------------
    begin                : Mit Dez 31 19:24:05 CET 2003
    copyright            : (C) 2003 by Klaas Freitag
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

#ifndef KRAFTDOC_H
#define KRAFTDOC_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files for QT
#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>

// #include <qguardedptr.h>

// include files for KDE
#include <kurl.h>

#include "docposition.h"
#include "dbids.h"

// forward declaration of the Kraft classes

class DocumentSaverBase;
class Geld;

class KraftView;

/**	KraftDoc provides a document object for a document-view model.
  *
  * The KraftDoc class provides a document object that can be used in conjunction with the classes KraftApp and KraftView
  * to create a document-view model for standard KDE applications based on KApplication and KMainWindow. Thereby, the document object
  * is created by the KraftApp instance and contains the document structure with the according methods for manipulation of the document
  * data by KraftView objects. Also, KraftDoc contains the methods for serialization of the document data from and to files.
  *
  * @author Source Framework Automatically Generated by KDevelop, (c) The KDevelop Team.
  * @version KDevelop version 1.2 code generation
  */
class KraftDoc : public QObject
{
  Q_OBJECT
  public:
    enum Part { Header,  Positions, Footer, Unknown };

    /** Constructor for the fileclass of the application */
    KraftDoc(QWidget *parent = 0, const char *name = 0);
    /** Destructor for the fileclass of the application */
    ~KraftDoc();

    /** adds a view to the document which represents the document contents.
     *  Usually this is your main view. */
    void addView(KraftView *view);
    /** removes a view from the list of currently connected views */
    void removeView(KraftView *view);
    /** sets the modified flag for the document after a modifying action on the view connected to the document.*/
    void setModified(bool _m=true){ modified=_m; };
    /** returns if the document is modified or not. Use this to determine if your document needs saving by the user on closing.*/
    bool isModified(){ return modified; };
    /** "save modified" - asks the user for saving if the document is modified */
    bool saveModified();
    /** deletes the document's contents */
    void deleteContents();
    /** initializes the document generally */
    bool newDocument();
    /** closes the actual document */
    void closeDocument();
    /** loads the document by filename and format and emits the updateViews() signal */
    bool openDocument(const QString& );
    /** fetch the document from database back */
    bool reloadDocument();
    /** saves the document under filename and format.*/
    bool saveDocument( );
    /** returns the KURL of the document */
    const KURL& URL() const;
    /** sets the URL of the document */
    void setURL(const KURL& url);

    DocPosition* createPosition();
    DocPositionList positions() { return mPositions; }
    void setPositionList( const DocPositionList& );

    QDate date() { return mDate; }
    void setDate( QDate d ) { mDate = d; }

    QString docType() { return mDocType; }
    void setDocType( const QString& s ) {  mDocType = s; }

    QString addressUid() { return mAddressUid; }
    void setAddressUid( const QString& id ) { mAddressUid = id; }

    QString& address() { return mAddress; } const
    void setAddress( const QString& adr ) { mAddress = adr; }

    bool isNew() { return mIsNew; }

    QString ident()    { return mIdent;    }
    void setIdent( const QString& str ) { mIdent = str; }

    QString salut()    { return mSalut;    }
    void setSalut( const QString& str ) { mSalut = str; }

    QString goodbye()    { return mGoodbye;    }
    void setGoodbye( const QString& str ) { mGoodbye = str; }

    QString preText()  { return mPreText;  }
    void setPreText( const QString& str ) { mPreText = str; }

    QString postText() { return mPostText; }
    void setPostText( const QString& str ) { mPostText = str; }

    QString whiteboard() { return mWhiteboard; }
    void setWhiteboard( const QString& w ) { mWhiteboard = w; }

    void setDocID( dbID id ) { mDocID = id; }
    dbID docID() { return mDocID; }

    QString docIdentifier();
    DBIdList removePositionList() { return mRemovePositions; }

    KraftView* firstView();

  Geld nettoSum();
  Geld bruttoSum();
  Geld vatSum();

  public slots:
    /** calls repaint() on all views connected to the document object and is called by the view by which the document has been changed.
     * As this view normally repaints itself, it is excluded from the paintEvent.
     */
    void slotUpdateAllViews( KraftView *sender );
    int slotAppendPosition( const DocPosition& );

    // The following slots take get the db id as argument
    void slotRemovePosition( int );
    void slotMoveUpPosition( int );
    void slotMoveDownPosition( int );
  private:
    /** the list of the views currently connected to the document */
    QList<KraftView> *pViewList;

    DocumentSaverBase* getSaver( const QString& saverHint = QString() );
    /** the modified flag of the current document */
    bool modified;
    bool mIsNew;

    KURL doc_url;

    QString mAddressUid;
    QString mAddress;
    QString mPreText;
    QString mPostText;
    QString mDocType;
    QString mSalut;
    QString mGoodbye;
    QString mIdent;
    QString mWhiteboard;

    QDate   mDate;
    DocPositionList mPositions;
    DBIdList mRemovePositions;
    DocumentSaverBase *mSaver;
    dbID    mDocID;
};

typedef QGuardedPtr<KraftDoc> DocGuardedPtr;
#endif // KraftDoc_H
