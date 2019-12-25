/*
 *  Copyright (C) 2009-2019 Leandro Nini
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

#ifndef INPUTTYPES_H
#define INPUTTYPES_H

/**
 * Sample types
*/
enum class sample_t
{
    U8 = 1,       ///< U8 - 8 bit unsigned
    S16,          ///< S16 - 16 bit signed
    S24,          ///< S24 - 24 bit signed
    S32,          ///< S32 - 32 bit signed
    SAMPLE_FLOAT, ///< FLOAT - 32 bit float
    SAMPLE_FIXED  ///< FIXED - 32 bit fixed point
};

#endif
