/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2020 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004 Dag Lem <resid@nimrod.no>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#define EXTERNALFILTER_CPP

#include "ExternalFilter.h"

namespace reSIDfpII
{

/**
 * Get the 3 dB attenuation point.
 *
 * @param res the resistance value in Ohms
 * @param cap the capacitance value in Farads
 */
constexpr inline double getRC(double res, double cap)
{
    return res * cap;
}

ExternalFilter::ExternalFilter()
{
    reset();
}

void ExternalFilter::setClockFrequency(double frequency)
{
    const double dt = 1. / frequency;

    // Low-pass:  R = 10kOhm, C = 1000pF; w0l = dt/(dt+RC) = 1e-6/(1e-6+1e4*1e-9) = 0.091
    // Cutoff 1/2*PI*RC = 1/2*PI*1e4*1e-9 = 15915.5 Hz
    w0lp = static_cast<float>(dt / (dt + getRC(10e3, 1000e-12)));

    // High-pass: R = 10kOhm, C = 10uF;   w0h = dt/(dt+RC) = 1e-6/(1e-6+1e4*1e-5) = 0.00000999
    // Cutoff 1/2*PI*RC = 1/2*PI*1e4*1e-5 = 1.59155 Hz
    w0hp = static_cast<float>(dt / (dt + getRC(10e3, 10e-6)));
}

void ExternalFilter::reset()
{
    // State of filter.
    Vlp = 0.f;
    Vhp = 0.f;
}

} // namespace reSIDfpII
