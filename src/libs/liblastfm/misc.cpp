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
#include "misc.h"

#include <QCryptographicHash>
#include <QDir>
#include <QDebug>
#ifdef WIN32
    #include <shlobj.h>
#endif


#ifdef Q_OS_MAC
#include <CoreFoundation/CoreFoundation.h>

QDir
lastfm::dir::bundle()
{
    // Trolltech provided example
    CFURLRef appUrlRef = CFBundleCopyBundleURL( CFBundleGetMainBundle() );
    CFStringRef macPath = CFURLCopyFileSystemPath( appUrlRef, kCFURLPOSIXPathStyle );
    QString path = CFStringToQString( macPath );
    CFRelease(appUrlRef);
    CFRelease(macPath);
    return QDir( path );
}
#endif


QDir ensurePathExists( QDir dir )
{
    if ( !dir.exists() )
        dir.mkpath( QString( "." ) );

    return dir;
}


static QDir dataDotDot()
{
#ifdef WIN32
    // Use this for non-DOS-based Windowses
    char path[MAX_PATH];
    HRESULT h = SHGetFolderPathA( NULL, 
                                  CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE,
                                  NULL, 
                                  0, 
                                  path );
    if (h == S_OK)
        return QString::fromLocal8Bit( path );
    else
        return QDir::home();
#elif defined(Q_OS_MAC)
    return ensurePathExists( QDir::home().filePath( "Library/Application Support" ) );
#elif defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    return ensurePathExists( QDir::home().filePath( ".local/share" ) );
#else
    return QDir::home();
#endif
}


QDir
lastfm::dir::runtimeData()
{
    return ensurePathExists( dataDotDot().filePath( "Last.fm" ) );
}


QDir
lastfm::dir::logs()
{
#ifdef Q_OS_MAC
    return ensurePathExists( QDir::home().filePath( "Library/Logs/Last.fm" ) );
#else
    return runtimeData();    
#endif
}


QDir
lastfm::dir::cache()
{
#ifdef Q_OS_MAC
    return ensurePathExists( QDir::home().filePath( "Library/Caches/Last.fm" ) );
#else
    return ensurePathExists( runtimeData().filePath( "cache" ) );
#endif
}


#ifdef WIN32
QDir
lastfm::dir::programFiles()
{
    char path[MAX_PATH];

    // TODO: this call is dependant on a specific version of shell32.dll.
    // Need to degrade gracefully. Need to bundle SHFolder.exe with installer
    // and execute it on install for this to work on Win98.
    HRESULT h = SHGetFolderPathA( NULL,
                                 CSIDL_PROGRAM_FILES, 
                                 NULL,
                                 0, // current path
                                 path );

    if (h != S_OK)
    {
        qCritical() << "Couldn't get Program Files dir. Possibly Win9x?";
        return QDir();
    }

    return QString::fromLocal8Bit( path );
}
#endif

#ifdef Q_OS_MAC
CFStringRef
lastfm::QStringToCFString( const QString &s )
{
    return CFStringCreateWithCharacters( 0, (UniChar*)s.unicode(), s.length() );
}

QByteArray
lastfm::CFStringToUtf8( CFStringRef s )
{
    QByteArray result;

    if (s != NULL) 
    {
        CFIndex length;
        length = CFStringGetLength( s );
        length = CFStringGetMaximumSizeForEncoding( length, kCFStringEncodingUTF8 ) + 1;
        char* buffer = new char[length];

        if (CFStringGetCString( s, buffer, length, kCFStringEncodingUTF8 ))
            result = QByteArray( buffer );
        else
            qWarning() << "CFString conversion failed.";

        delete[] buffer;
    }

    return result;
}
#endif


const char*
lastfm::platform()
{
    static QString platform = QSysInfo::prettyProductName();
    return qPrintable(platform);
}

QString lastfm::
md5( const QByteArray& src )
{
    QByteArray const digest = QCryptographicHash::hash( src, QCryptographicHash::Md5 );
    return QString::fromLatin1( digest.toHex() ).rightJustified( 32, '0' ).toLower();
}

#ifdef Q_OS_MAC
QString
lastfm::CFStringToQString( CFStringRef s )
{
    return QString::fromUtf8( CFStringToUtf8( s ) );
}
#endif

