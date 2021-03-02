/*
 *  Copyright (C) 2007-2021 Leandro Nini
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

#ifndef FACTORY_H
#define FACTORY_H

#include "input.h"

#include <new>
#include <QList>

#define IFACTORY iFactory::instance()

class iFactory
{
typedef input* (*inputFactory)();
typedef bool (*supportsFunc)(const QString&);

typedef struct
{
    const char* name;
    supportsFunc supports;
    inputFactory factory;
} inputs_t;

private:
    template <class backend>
    void regBackend();

private:
    QList<inputs_t> _inputs;

protected:
    iFactory();
    iFactory(const iFactory&);
    iFactory& operator= (const iFactory&);
    ~iFactory() {}

public:
    /// Get singleton instance
    static iFactory* instance();

    /// Get number of registered backends
    int num() const { return _inputs.size(); }

    /// Get backend's name
    const char* name(const int i) const { return _inputs[i].name; }

    /// Check if backend supports file
    bool supports(const int i, const QString& file) const { return _inputs[i].supports(file); }

    /// Instantiate backend
    input* get(const int i);

    /// Instantiate backend
    input* get(const QString& filename);
};

#endif
