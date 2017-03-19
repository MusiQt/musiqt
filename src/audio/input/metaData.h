/*
 *  Copyright (C) 2010-2017 Leandro Nini
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef METADATA_H
#define METADATA_H

#include "imageData.h"
#include "utils.h"

#include <QHash>

class metaData
{
public:
enum mpris_t
{
    LOCATION,
    TITLE,
    ARTIST,
    ALBUM,
    TRACK,
    TIME,
    MTIME,
    GENRE,
    COMMENT,
    RATING,
    YEAR,
    ARTURL,
    LAST_ID
};

public:
    virtual int moreInfo(const int i) const =0;
    virtual QString getKey(const unsigned int num) const =0;
    virtual QString getKey(const mpris_t info) const =0;
    virtual QString getInfo(unsigned int num) const =0;
    virtual QString getInfo(const char* info) const =0;
    virtual QString getInfo(const mpris_t info) const =0;
    virtual imageData* getImage(void) const =0;
};

/*****************************************************************/

class metaDataImpl : public metaData
{
private:
    typedef QHash<QString, QString> StringDict;
private:
    static const char* mprisTags[LAST_ID];

private:
    StringDict _infos;

    imageData *_img;

public:
    metaDataImpl(void) : _img(0) {}

    /// Append song info
    void addInfo(QString type, QString info);
    void addInfo(QString type, unsigned int info);
    void addInfo(const mpris_t type, QString info);
    void addInfo(const mpris_t type, unsigned int info);
    void addInfo(imageData* img) { _img = img; }

    /// Remove all info
    void clearInfo() { _infos.clear(); delPtr(_img); }

    /// Find song info
    QString findInfo(QString type) const { return _infos.find(type).value(); }

    /// Get song info
    int moreInfo(const int i) const;

    QString getKey(const unsigned int num) const { return _infos.keys()[num]; }
    QString getKey(const mpris_t info) const { return mprisTags[info]; }
    QString getInfo(unsigned int num) const { return _infos.values()[num]; }
    QString getInfo(const char* info) const { auto it=_infos.find(info); return it!=_infos.end()?it.value():QString::null; }
    QString getInfo(const mpris_t info) const { return getInfo(mprisTags[info]); }
    imageData* getImage(void) const { return _img; }
};

#endif
