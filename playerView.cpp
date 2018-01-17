#include "playerView.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QPalette>

#include <QtCore/QEvent>

#include "game.h"
#include "viewUtils.h"

PlayerView::PlayerView( Player* p, QWidget* parent ) : QWidget( parent ), player_( p ) {
    auto vl = new QVBoxLayout( this );
    auto l = new QLabel( "<b>Player " + QString::number( p->number_ + 1 ) + "</b>" );
    l->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    l->setStyleSheet( "background-color:" + playerColor[ p->number_ ].name( QColor::HexRgb ) );
    vl->addWidget( l );
    resources_ = new QLabel( "0 cards" );
    vl->addWidget( resources_ );
    auto hl = new QHBoxLayout();
    vl->addLayout( hl );
    trade_ = new QPushButton( "Trade" );
    trade_->setToolTip( "Not implemented" );
    hl->addWidget( trade_ );
    build_ = new QPushButton( "Build" );
    hl->addWidget( build_ );
    card_ = new QPushButton( "Card" );
    card_->setToolTip( "Not implemented" );
    hl->addWidget( card_ );
    pass_ = new QPushButton( "Pass" );
    hl->addWidget( pass_ ); 
}


void PlayerView::updateView() {
    QString text;
    int total = 0;
    for( int i = 0; i < player_->resources_.size(); i++ ) {
        int n = player_->resources_[ i ];
        if( n > 0 ) {
            text += QString::number( n ) + " " + Hex::typeName[ i ] + " ";
        }
        total += n;
    }
    resources_->setText( QString::number( total ) + " cards: " + text );
}


void PlayerView::enableButtons( bool enabled ) {
    trade_->setEnabled( false );
    build_->setEnabled( enabled && player_->game_->canBuild() );
    build_->setToolTip( enabled && ! player_->game_->canBuild() ? "Nothing can be built" : "" );
    card_->setEnabled( false );
    pass_->setEnabled( enabled );
}