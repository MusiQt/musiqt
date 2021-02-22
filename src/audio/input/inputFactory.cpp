/*
 *  Copyright (C) 2007-2019 Leandro Nini
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

#include "inputFactory.h"

#ifdef HAVE_MPG123
#  include "mpg123Backend.h"
#endif
#ifdef HAVE_VORBIS
#  include "oggBackend.h"
#endif
#ifdef HAVE_OPUS
#  include "opusBackend.h"
#endif
#ifdef HAVE_OPENMPT
#  include "openmptBackend.h"
#endif
#ifdef HAVE_SIDPLAYFP
#  include "sidBackend.h"
#endif
#ifdef HAVE_GME
#  include "gmeBackend.h"
#endif
#ifdef BUILD_HVL
#  include "hvlBackend.h"
#endif
#ifdef HAVE_WAVPACK
#  include "wvBackend.h"
#endif
#ifdef HAVE_SNDFILE
#  include "sndBackend.h"
#endif
#ifdef HAVE_LIBAVFORMAT_AVFORMAT_H
#  include "ffmpegBackend.h"
#endif
#ifdef HAVE_LIBMPCDEC
#  include "mpcBackend.h"
#endif

#include <QDebug>

iFactory* iFactory::instance()
{
    static iFactory i;
    return &i;
}

template <class backend>
void iFactory::regBackend()
{
    qDebug() << "Adding input backend " << backend::name;
    inputs_t temp;

    temp.name = backend::name;
    temp.supports = backend::supports;
    temp.factory = &backend::factory;
    _inputs.append(temp);
}

iFactory::iFactory()
{
    // Register backends

#ifdef HAVE_MPG123
    regBackend<mpg123Backend>();
#endif

#ifdef HAVE_VORBIS
    regBackend<oggBackend>();
#endif

#ifdef HAVE_OPUS
    regBackend<opusBackend>();
#endif

#ifdef HAVE_OPENMPT
    if (openmptBackend::init())
        regBackend<openmptBackend>();
#endif

#ifdef HAVE_SIDPLAYFP
    regBackend<sidBackend>();
#endif

#ifdef HAVE_GME
    if (gmeBackend::init())
        regBackend<gmeBackend>();
#endif

#ifdef BUILD_HVL
    regBackend<hvlBackend>();
#endif

#ifdef HAVE_WAVPACK
    regBackend<wvBackend>();
#endif

#ifdef HAVE_SNDFILE
    if (sndBackend::init())
        regBackend<sndBackend>();
#endif

#ifdef HAVE_LIBAVFORMAT_AVFORMAT_H
    if (ffmpegBackend::init())
        regBackend<ffmpegBackend>();
#endif
    
#ifdef HAVE_LIBMPCDEC
    regBackend<mpcBackend>();
#endif
}

input* iFactory::get(const int i)
{
    return _inputs[i].factory();
}
