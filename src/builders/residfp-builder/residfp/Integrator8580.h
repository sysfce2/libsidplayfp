/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2020 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004, 2010 Dag Lem <resid@nimrod.no>
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

#ifndef INTEGRATOR8580_H
#define INTEGRATOR8580_H

#include "InterpolatedLUT.h"

#include <stdint.h>
#include <cassert>

#include "siddefs-fp.h"

namespace reSIDfp
{

/**
 * 8580 integrator
 *
 *                    ---C---
 *                   |       |
 *     vi -----Rfc------[A>----- vo
 *                   vx
 *
 *     IRfc + ICr = 0
 *     IRfc + C*(vc - vc0)/dt = 0
 *     dt/C*(IRfc) + vc - vc0 = 0
 *     vc = vc0 - n*(IRfc(vi,vx))
 *     vc = vc0 - n*(IRfc(vi,g(vc)))
 *
 * IRfc = K/2*W/L*(Vgst^2 - Vgdt^2) = n*((Vddt - vx)^2 - (Vddt - vi)^2)
 *
 * Rfc gate voltage is generated by an OP Amp and depends on chip temperature.
 */
class Integrator8580
{
private:
    const LUT* opamp_rev;

    mutable float vx;
    mutable float vc;

    float nVgt;
    float n_dac;

    const double Vth;
    const double nKp;
    const double vmin;
    const double N16;

public:
    Integrator8580(const LUT* opamp_rev, double Vth, double denorm, double C, double uCox, double vmin, double N16) :
        opamp_rev(opamp_rev),
        vx(0.f),
        vc(0.f),
        Vth(Vth),
        nKp(denorm* (uCox / 2. * 1.0e-6 / C)),
        vmin(vmin),
        N16(N16)
    {
        setV(1.5);
    }

    void setFc(double wl)
    {
        // Normalized current factor, 1 cycle at 1MHz.
        const double tmp = nKp * wl;
        //assert(tmp > -0.5 && tmp < 65535.5);
        n_dac = static_cast<float>(tmp);
    }

    /**
     * Set FC gate voltage multiplier.
     */
    void setV(double v)
    {
        // Gate voltage is controlled by the switched capacitor voltage divider
        // Ua = Ue * v = 4.76v  1<v<2
        const double Vg = 4.76 * v;
        const double Vgt = Vg - Vth;

        // Vg - Vth, normalized so that translated values can be subtracted:
        // Vgt - x = (Vgt - t) - (x - t)
        const double tmp = N16 * (Vgt - vmin);
        assert(tmp > -0.5 && tmp < 65535.5);
        nVgt = static_cast<float>(tmp);
    }

    float solve(float vi) const;
};

} // namespace reSIDfp

#if RESID_INLINING || defined(INTEGRATOR8580_CPP)

namespace reSIDfp
{

RESID_INLINE
float Integrator8580::solve(float vi) const
{
    // Make sure we're not in subthreshold mode
    assert(vx < nVgt);

    // DAC voltages
    const float Vgst = nVgt - vx;
    const float Vgdt = (vi < nVgt) ? nVgt - vi : 0.f;  // triode/saturation mode

    const float Vgst_2 = Vgst * Vgst;
    const float Vgdt_2 = Vgdt * Vgdt;

    // DAC current, scaled by (1/m)*m*2^16*m*2^16 = m*2^32
    const float n_I_dac = n_dac * (Vgst_2 - Vgdt_2);

    // Change in capacitor charge.
    vc += n_I_dac;

    // vx = g(vc)
    const float tmp = (vc / 65536.f / 2.f) + (1 << 15);
    assert(tmp < (1 << 16));
    vx = opamp_rev->output(tmp);

    // Return vo.
    return vx - (vc / 65536.f);
}

} // namespace reSIDfp

#endif

#endif
