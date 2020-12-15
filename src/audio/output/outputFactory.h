/*
 *  Copyright (C) 2009-2017 Leandro Nini
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

#ifndef OFACTORY_H
#define OFACTORY_H

#include "output.h"

#include <new>
#include <QList>

#define OFACTORY oFactory::instance()

class oFactory
{
typedef output* (*outputFactory)();

typedef struct
{
    const char* name;
    outputFactory factory;
} outputs_t;

private:
    template <class backend>
    void regBackend();

private:
    QList<outputs_t> _outputs;

protected:
    oFactory();
    oFactory(const oFactory&);
    oFactory& operator=(const oFactory&);
    ~oFactory() {}

public:
    /// Get singleton instance
    static oFactory* instance();

    /// Get backend's name
    const char* name(const int i) const { return _outputs[i].name; }

    /// Instantiate backend
    output* get() const;
};

#endif
