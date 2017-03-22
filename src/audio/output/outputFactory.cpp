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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "outputFactory.h"

#include "alsaBackend.h"
#include "ossBackend.h"
#include "wmmBackend.h"

oFactory* oFactory::instance()
{
    static oFactory o;
    return &o;
}

template <class backend>
void oFactory::regBackend()
{
    outputs_t temp;

    temp.name = backend::name;
    temp.factory = &backend::factory;
    _outputs.append(temp);
}

oFactory::oFactory()
{
    // Register backends

#ifdef HAVE_ALSA
    regBackend<alsaBackend>();
#endif

#if defined (HAVE_SYS_SOUNDCARD_H) || defined (HAVE_MACHINE_SOUNDCARD_H)
    regBackend<ossBackend>();
#endif

#ifdef _WIN32
    regBackend<wmmBackend>();
#endif
}

output* oFactory::get(const int i) const
{
    return _outputs[i].factory();
}

output* oFactory::get(const QString& api) const
{
    for (int i=0; i<_outputs.size(); i++)
    {
        if (!api.compare(_outputs[i].name))
            return _outputs[i].factory();
    }

    return _outputs[0].factory();
}
