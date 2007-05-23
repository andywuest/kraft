/***************************************************************************
                          docassistant.cpp  - Assistant widget
                             -------------------
    begin                : April 2007
    copyright            : (C) 2007 by Klaas Freitag
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
#include <qwidgetstack.h>
#include <qtooltip.h>
#include <qasciidict.h>
#include <qtimer.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

#include "docassistant.h"
#include "docpostcard.h"
#include "catalogselection.h"
#include "headerselection.h"
#include "kraftsettings.h"
#include "kataloglistview.h"
#include "doctext.h"
#include "defaultprovider.h"
#include "headertemplateprovider.h"

DocAssistant::DocAssistant( QWidget *parent ):
  QSplitter( parent ), mFullPreview( true ),
  mActivePage( KraftDoc::Header )
{
  setOrientation( Vertical );
  QVBox *vb = new QVBox( this );

  QHBox *hb = new QHBox( vb );
  hb->setFrameStyle( Box + Sunken );
  hb->setMargin( KDialog::marginHint()/2 );

  KPushButton *pb = new KPushButton( i18n( "show Templates" ),  hb );
  connect( pb, SIGNAL( toggled( bool ) ),
           this, SLOT( slotToggleShowTemplates( bool ) ) );
  pb->setToggleButton( true );
  QToolTip::add( pb, i18n( "Show mask to create or select templates to be used in the document" ) );

  QWidget *w = new QWidget( hb );
  w->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

  mPostCard =  new DocPostCard( vb );

  mPostCard->slotSetMode( DocPostCard::Full, KraftDoc::Header );
  setResizeMode( vb /* mPostCard->view() */, KeepSize );

  connect( mPostCard, SIGNAL( completed() ),
           this,  SLOT( slotRenderCompleted() ) );

  QVBox *stackVBox = new QVBox( this );
  mTemplatePane = stackVBox;
  stackVBox->setSpacing( KDialog::spacingHint() );
  mWidgetStack = new QWidgetStack( stackVBox );
  mWidgetStack->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

  mCatalogSelection = new CatalogSelection( mWidgetStack );
  connect( mCatalogSelection,  SIGNAL( positionSelected( Katalog*, void* ) ),
           this,  SIGNAL( positionSelected( Katalog*, void* ) ) );

  mHeaderSelection = new HeaderSelection( mWidgetStack );
  connect( mHeaderSelection, SIGNAL( addressSelectionChanged() ),
           this, SLOT( slotAddressSelectionChanged() ) );
  connect( mHeaderSelection, SIGNAL( textSelectionChanged() ),
           this, SLOT( slotTextsSelectionChanged() ) );

  mWidgetStack->raiseWidget( mHeaderSelection );
  connect( mPostCard, SIGNAL( selectPage( int ) ),
           this,  SLOT( slotSelectDocPart( int ) ) );

  QHBox *butBox = new QHBox( stackVBox );
  butBox->setSpacing( KDialog::spacingHint() );
  QIconSet icons = BarIconSet( "back" );
  mPbAdd  = new KPushButton( icons, i18n(""), butBox );
  mPbAdd->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbAdd, SIGNAL( clicked() ), this, SLOT( slotAddToDocument() ) );

  QToolTip::add( mPbAdd, i18n( "Add a template to the document" ) );

  w = new QWidget( butBox );
  w->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );

  mPbNew  = new KPushButton( BarIconSet( "filenew" ), i18n(""),  butBox );
  mPbNew->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbNew, SIGNAL( clicked() ), this, SLOT( slotNewTemplate() ) );
  QToolTip::add( mPbNew, i18n( "Create a new template (type depending)" ) );

  mPbEdit  = new KPushButton( BarIconSet( "edit" ), i18n(""),  butBox );
  mPbEdit->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbEdit, SIGNAL( clicked() ), this, SLOT( slotEditTemplate() ) );
  QToolTip::add( mPbEdit, i18n( "Edit the selected template (type depending)" ) );

  mPbDel  = new KPushButton( BarIconSet( "editdelete" ), i18n(""),  butBox );
  mPbDel->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  connect( mPbDel, SIGNAL( clicked() ), this, SLOT( slotDeleteTemplate() ) );
  QToolTip::add( mPbDel, i18n( "Delete the selected template (type depending)" ) );

  mPbAdd->setEnabled( false );
  mPbEdit->setEnabled( false );
  mPbDel->setEnabled( false );

  /* Template Provider initialisation */
  mHeaderTemplateProvider = new HeaderTemplateProvider( parent );

  /* get a new header text from the default provider */
  connect( mHeaderTemplateProvider, SIGNAL( newHeaderText( const DocText& ) ),
           this,  SLOT( slotNewHeaderDocText( const DocText& ) ) );
  connect( mHeaderTemplateProvider, SIGNAL( updateHeaderText( const DocText& ) ),
           this,  SLOT( slotUpdateHeaderDocText( const DocText& ) ) );
  connect( mHeaderTemplateProvider, SIGNAL( headerTextToDocument( const DocText& ) ),
           this,  SLOT( slotHeaderTextToDocument( const DocText& ) ) );
  connect( mHeaderTemplateProvider, SIGNAL( deleteHeaderText( const DocText& ) ),
           this,  SLOT( slotTextDeleted( const DocText& ) ) );

  mCurrTemplateProvider = mHeaderTemplateProvider;

  setSizes( KraftSettings::self()->assistantSplitterSetting() );
  mTemplatePane->hide();
}

void DocAssistant::slotAddToDocument()
{
  kdDebug() << "SlotAddToDocument called!" << endl;
  mCurrTemplateProvider->slotTemplateToDocument();

#if 0
  if ( mWidgetStack->visibleWidget() == mHeaderSelection ) {
    /* Header page */
    if ( mHeaderSelection->textPageActive() ) {
      kdDebug() << "Text Page active: " << mHeaderSelection->currentText() << endl;
      emit headerTextTemplate( mHeaderSelection->currentText() );
    } else if ( mHeaderSelection->addressPageActive() ) {
      kdDebug() << "Address Page active" << endl;
      KABC::Addressee adr = mHeaderSelection->currentAddressee();
      emit addressTemplate( adr );
    }

  } else if ( mWidgetStack->visibleWidget() == mCatalogSelection ) {


  } else if ( mWidgetStack->visibleWidget() == mFooterSelection ) {

  }
#endif
}

void DocAssistant::slotAddressSelectionChanged()
{
  kdDebug() << "A address template was selected!" << endl;
  if ( mHeaderSelection->addressListView()->currentItem() ) {
    mPbAdd->setEnabled( true );
    mPbEdit->setEnabled( true );
    mPbDel->setEnabled( false );

  }
}

void DocAssistant::slotTextsSelectionChanged()
{
  mHeaderTemplateProvider->slotSetCurrentDocText( mHeaderSelection->currentDocText() );

  if ( mHeaderSelection->textsListView()->currentItem() ) {
    mPbAdd->setEnabled( true );
    mPbEdit->setEnabled( true );
    mPbDel->setEnabled( true );

  }
}

void DocAssistant::slotNewTemplate()
{
  /* always set the doc type in case the provider benefits from that */
  mCurrTemplateProvider->slotSetDocType( mDocType );
  mCurrTemplateProvider->slotNewTemplate();
}

/* a new header doc text was created and should go to the document */
void DocAssistant::slotNewHeaderDocText( const DocText& dt )
{
  /* show in list of texts in the GUI */
  QListViewItem *item = mHeaderSelection->addNewDocText( dt );
  if ( item ) {
    item->setSelected( true );
    slotTextsSelectionChanged();

    // FIXME: Here slotaddtodocument should be called but that's not
    //        working somehow...
    emit headerTextTemplate( dt.text() );
  }
}

/* called with a changed text that needs to be updated in the view */
void DocAssistant::slotUpdateHeaderDocText( const DocText& dt )
{
  mHeaderSelection->updateDocText( dt );
}

/* the user hit "add to document" to use a header text template */
void DocAssistant::slotHeaderTextToDocument( const DocText& dt )
{
  emit headerTextTemplate( dt.text() );
}

/* Slot that initiates an edit */
void DocAssistant::slotEditTemplate()
{
  kdDebug() << "Editing a template using the currentTemplProvider" << endl;
  mCurrTemplateProvider->slotSetDocType( mDocType );
  mCurrTemplateProvider->slotEditTemplate();
}

/* slot that initialises a delete, called from the delete button */
void DocAssistant::slotDeleteTemplate()
{
  if ( KMessageBox::warningYesNo( this, i18n( "Do you really want to delete the "
                                            "Template permanently? There is no way "
                                              "to recover!" ) )
       == KMessageBox::No  )
  {
    return;
  }

  mCurrTemplateProvider->slotDeleteTemplate();
#if 0
  DocText dt = mHeaderSelection->currentDocText();
  DefaultProvider::self()->deleteDocumentText( dt );
  mHeaderSelection->deleteCurrentText();
#endif
}

void DocAssistant::slotTextDeleted( const DocText& /* dt */)
{
  mHeaderSelection->deleteCurrentText();
}

/* slot that opens the template details in case on == true */
void DocAssistant::slotToggleShowTemplates( bool on )
{
  if ( on ) {
    if ( mActivePage == KraftDoc::Header ) {
      slotShowHeaderTemplates();
    } else if ( mActivePage == KraftDoc::Positions ) {
      slotShowCatalog();
    } else if ( mActivePage == KraftDoc::Footer ) {

    }
  } else {
    // hide the details
    setFullPreview( true, mActivePage );
  }
  emit toggleShowTemplates( on );
}

void DocAssistant::slotRenderCompleted()
{
  // kdDebug() << "Render completed: " << mPostCard->view()->contentsHeight() << endl;

#if 0
  /* This is unfortunately not working because contentsHeight is always as
     heigh as the viewport is. Must be fixed in khtmlpart. */
  QValueList<int> sizes;
  sizes << mPostCard->view()->contentsHeight();
  setSizes( sizes );
#endif

}

DocPostCard *DocAssistant::postCard()
{
  return mPostCard;
}

CatalogSelection* DocAssistant::catalogSelection()
{
  return mCatalogSelection;
}

/* sets the Part of the doc, eg. Header, Footer */
void DocAssistant::slotSelectDocPart( int p )
{
  mActivePage = p;
  emit selectPage( p ) ;
}

/* Doc Type like offer, invoice etc. */
void DocAssistant::slotSetDocType( const QString& type )
{
  mDocType = type;
  mHeaderSelection->slotSelectDocType( type );
}

void DocAssistant::slotShowCatalog( )
{
  setFullPreview( false, KraftDoc::Positions );
  mWidgetStack->raiseWidget( mCatalogSelection );
}

void DocAssistant::slotShowHeaderTemplates()
{
  setFullPreview( false, KraftDoc::Header );
  mWidgetStack->raiseWidget( mHeaderSelection );
}

void DocAssistant::setFullPreview( bool setFull, int id )
{
  if ( setFull ) {
    /* remember the sizes used before */
    if ( mTemplatePane->isVisible() ) {
      kdDebug() << "Writing mSplitterSizes: " << sizes() << endl;
      KraftSettings::self()->setAssistantSplitterSetting( sizes() );
      KraftSettings::self()->writeConfig();
    }

    mTemplatePane->hide();
    mPostCard->slotSetMode( DocPostCard::Full, id );
    mFullPreview = true;
  } else {
    mTemplatePane->show();
    mPostCard->slotSetMode( DocPostCard::Mini, id );

    if ( KraftSettings::self()->assistantSplitterSetting().size() == 2 ) {
      setSizes( KraftSettings::self()->assistantSplitterSetting() );
    }
    mFullPreview = false;
  }
}

#include "docassistant.moc"