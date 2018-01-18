/***************************************************************************
        newdocassistant  - widget to select header data for the doc
                             -------------------
    begin                : 2008-02-12
    copyright            : (C) 2008 by Klaas Freitag
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

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>

#include <kcontacts/addressee.h>
#include <QComboBox>
#include <QDialog>
#include <QDateEdit>
#include <QDebug>
#include <QTextEdit>

#include <klocalizedstring.h>
#include <kassistantdialog.h>

#include "newdocassistant.h"
#include "defaultprovider.h"
#include "filterheader.h"
#include "doctype.h"
#include "kraftsettings.h"
#include "addressselectorwidget.h"

CustomerSelectPage::CustomerSelectPage( QWidget *parent )
  :QWidget( parent )
{
  QVBoxLayout *vbox = new QVBoxLayout;
  parent->setLayout( vbox );
//TODO PORT QT5   vbox->setSpacing( QDialog::spacingHint() );
//TODO PORT QT5   vbox->setMargin( QDialog::marginHint() );

  QLabel *help = new QLabel;
  help->setText( i18n( "Please select a customer as addressee for the document. "
                   "If there is no entry for the customer in the addressbook yet, it can be opened "
                       "by clicking on the button below." ) );
  // help->setTextFormat( Qt::RichText );
  help->setWordWrap( true );
  help->setSizePolicy( QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed ));

  vbox->addWidget( help );

  mAddresses = new AddressSelectorWidget(this);

  connect( mAddresses,  SIGNAL( addressSelected( const KContacts::Addressee& ) ),
           SIGNAL( addresseeSelected( const KContacts::Addressee& ) ) );

  vbox->addWidget( mAddresses );
}

void CustomerSelectPage::saveState()
{
  mAddresses->saveState();
}

void CustomerSelectPage::setupAddresses()
{

}

CustomerSelectPage:: ~CustomerSelectPage()
{

}

// ###########################################################################

DocDetailsPage::DocDetailsPage( QWidget *parent )
  :mCustomerLabel( 0 )
{
  QVBoxLayout *vbox = new QVBoxLayout;
  parent->setLayout( vbox );

//TODO PORT QT5   vbox->setSpacing( QDialog::spacingHint() );
//TODO PORT QT5   vbox->setMargin( QDialog::marginHint() );

  QLabel *help = new QLabel;
  help->setTextFormat( Qt::RichText );
  help->setText( i18n( "Select a document type and a date. A comment on the whiteboard "
                       "helps to classify the document." ) );
  vbox->addWidget( help );

  mCustomerLabel = new QLabel;
  mCustomerLabel->setFrameStyle( QFrame::Box + QFrame::Sunken );
  mCustomerLabel->setTextFormat( Qt::RichText );
//TODO PORT QT5   mCustomerLabel->setMargin( QDialog::marginHint() );
  mCustomerLabel->setText( i18n( "Customer: Not yet selected!" ) );
  vbox->addWidget( mCustomerLabel );

  QFormLayout *grid = new QFormLayout;
//TODO PORT QT5   grid->setSpacing( QDialog::marginHint() );
  vbox->addLayout( grid );

//   QLabel *l = new QLabel( i18n( "Some Document Details: " ), vbox );
//TODO PORT QT5 //  l->setMargin( QDialog::marginHint() );

  mTypeCombo = new QComboBox;
  mTypeCombo->insertItems( 0, DocType::allLocalised() );
  mTypeCombo->setCurrentIndex( mTypeCombo->findText( DefaultProvider::self()->docType() ));
  grid->addRow( i18n("Document &Type:"), mTypeCombo );

  mDateEdit = new QDateEdit;
  mDateEdit->setDate( QDate::currentDate() );
  grid->addRow( i18n( "Document Date: " ), mDateEdit );

  mWhiteboardEdit = new QTextEdit;
  grid->addRow( i18n( "Whiteboard Content:" ), mWhiteboardEdit );

  vbox->addStretch( 1 );


}

DocDetailsPage::~DocDetailsPage()
{

}

// ###########################################################################

KraftWizard::KraftWizard(QWidget *parent, const char* name, bool modal )
  :KAssistantDialog( parent ),
   mCustomerPage( 0 ),
   mCustomerBox( 0 ),
   mParent( parent )
{
  setObjectName( name );
  setModal( modal );

  const QByteArray geo = QByteArray::fromBase64( KraftSettings::self()->newDocWizardGeometry().toAscii() );
  restoreGeometry(geo);
}

KraftWizard::~KraftWizard()
{

}

void KraftWizard::init()
{
    QScopedPointer<AddressProvider> addressProvider;
    addressProvider.reset(new AddressProvider);

    setWindowTitle( i18n( "Document Creation Wizard" ) );
    QWidget *w1 = new QWidget;
    mDetailsPageItem = addPage( w1, i18n( "<h2>Document Details</h2>" ) );
    mDetailsPage = new DocDetailsPage( w1 );

    if( addressProvider->backendUp() ) {
        QWidget *w = new QWidget;
        mCustomerPageItem = addPage( w, i18n( "<h2>Select an Addressee</h2>" ) );

        mCustomerPage = new CustomerSelectPage( w );
        mCustomerPage->setupAddresses();
        connect( mCustomerPage, SIGNAL( addresseeSelected(KContacts::Addressee)),
                 this,  SLOT( slotAddressee(KContacts::Addressee)));
    }
}

void KraftWizard::done( int r )
{
    if( mCustomerPage ) {
        mCustomerPage->saveState();
    }
    const QByteArray geo = saveGeometry().toBase64();
    KraftSettings::self()->setNewDocWizardGeometry(geo);

    KAssistantDialog::done(r);
}

void KraftWizard::slotAddressee(const KContacts::Addressee& addressee)
{
  // qDebug () << "Addressee Changed!";
  mAddressee = addressee;
}

QDate KraftWizard::date() const
{
  return mDetailsPage->mDateEdit->date();
}

QString KraftWizard::addressUid() const
{
  return mAddressee.uid();
}

QString KraftWizard::docType() const
{
  return mDetailsPage->mTypeCombo->currentText();
}

QString KraftWizard::whiteboard() const
{
  return mDetailsPage->mWhiteboardEdit->toPlainText();
}

void KraftWizard::setDocIdentifier( const QString& ident )
{
  // we already know the customer, disable the customer select page.
  setAppropriate( mCustomerPageItem, false );

  if ( mDetailsPage->mCustomerLabel ) {
    mDetailsPage->mCustomerLabel->setText( ident );
  }
}

void KraftWizard::setAvailDocTypes( const QStringList& list )
{
  mDetailsPage->mTypeCombo->clear();
  mDetailsPage->mTypeCombo->insertItems( -1, list );
}
