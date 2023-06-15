/*
   Copyright 2009 Last.fm Ltd.
      - Primarily authored by Max Howell, Jono Cole and Doug Mansell

   This file is part of liblastfm.

   liblastfm is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   liblastfm is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with liblastfm.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "UrlBuilder.h"
#include <QRegularExpression>
#include <QStringList>


class lastfm::UrlBuilderPrivate
{
public:
    QByteArray path;
};


lastfm::UrlBuilder::UrlBuilder( const QString& base )
    : d( new UrlBuilderPrivate )
{
    d->path = '/' + base.toLatin1();
}


lastfm::UrlBuilder::UrlBuilder( const UrlBuilder& that )
    : d( new UrlBuilderPrivate( *that.d ) )
{
}


lastfm::UrlBuilder::~UrlBuilder()
{
    delete d;
}


lastfm::UrlBuilder&
lastfm::UrlBuilder::slash( const QString& path )
{
    this->d->path += '/' + encode( path );
    return *this;
}


QUrl
lastfm::UrlBuilder::url() const
{
    QUrl url( "https://" + host() );
    url.setPath( url.path() + d->path );
    return url;
}


QByteArray //static
lastfm::UrlBuilder::encode( QString s )
{
    foreach (QChar c, QList<QChar>() << '%' << '&' << '/' << ';' << '+' << '#' << '"')
        if (s.contains( c ))
            // the middle step may seem odd but this is what the site does
            // eg. search for the exact string "Radiohead 2 + 2 = 5"
            return QUrl::toPercentEncoding( s ).replace( "%20", "+" );

    return QUrl::toPercentEncoding( s.replace( ' ', '+' ), "+" );
}


QString //static
lastfm::UrlBuilder::host( const QLocale& locale )
{
    switch (locale.language())
    {
        case QLocale::Portuguese: return "www.last.fm/pt";
        case QLocale::Turkish:    return "www.last.fm/tr";
        case QLocale::French:     return "www.last.fm/fr";
        case QLocale::Italian:    return "www.last.fm/it";
        case QLocale::German:     return "www.last.fm/de";
        case QLocale::Spanish:    return "www.last.fm/es";
        case QLocale::Polish:     return "www.last.fm/pl";
        case QLocale::Russian:    return "www.last.fm/ru";
        case QLocale::Japanese:   return "www.last.fm/jp";
        case QLocale::Swedish:    return "www.last.fm/se";
        case QLocale::Chinese:    return "www.last.fm/zh";
        default:                  return "www.last.fm";
    }
}


bool // static
lastfm::UrlBuilder::isHost( const QUrl& url )
{
    QStringList hosts = QStringList() << "www.last.fm";

    return hosts.contains( url.host() );
}

QUrl //static
lastfm::UrlBuilder::localize( QUrl url)
{
    url.setHost( url.host().replace( QRegularExpression("^(www.)?last.fm"), host() ) );
    return url;
}


QUrl //static
lastfm::UrlBuilder::mobilize( QUrl url )
{
    url.setHost( url.host().replace( QRegularExpression("^(www.)?last"), "m.last" ) );
    return url;
}

lastfm::UrlBuilder&
lastfm::UrlBuilder::operator=( const UrlBuilder& that )
{
    d->path = that.d->path;
    return *this;
}
