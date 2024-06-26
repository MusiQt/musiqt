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

#include <QList>

#define IFACTORY iFactory::instance()

class input;
class inputConfig;

class iFactory
{
    using inputFactory = input*(*)(const QString& fileName);
    using configFactory = inputConfig*(*)();
    using extFunc = QStringList(*)();

    struct inputs_t
    {
        const char* name;
        extFunc supportedExt;
        inputFactory factory;
        configFactory cFactory;
    };

private:
    template <class backend>
    void regBackend();

private:
    iFactory(const iFactory&) = delete;
    iFactory& operator= (const iFactory&) = delete;

    QList<inputs_t> m_inputs;

protected:
    iFactory();
    ~iFactory() = default;

public:
    /// Get singleton instance
    static iFactory* instance();

    /// Get number of registered backends
    int num() const { return m_inputs.size(); }

    /// Get backend's name
    const char* name(const int i) const { return m_inputs[i].name; }

    /// Get supported extensions
    QStringList getExtensions() const;

    /// Instantiate empty backend
    input* get();

    /// Instantiate backend
    input* get(const QString& filename);

    /// Instantiate backend config
    inputConfig* getConfig(const int i);
};

#endif
