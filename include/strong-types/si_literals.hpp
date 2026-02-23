#pragma once

#include "si.hpp"

namespace strong_types::si_literals
{

// NOLINTBEGIN(cppcoreguidelines-macro-usage) — macro required for UDL token-pasting
#define STRONG_TYPES_DEFINE_UDL(suffix, Tag)                                                                           \
    [[nodiscard]] constexpr unit_t<double, Tag> operator""_##suffix(long double val)                                   \
    {                                                                                                                  \
        return unit_t<double, Tag>{static_cast<double>(val)};                                                          \
    }                                                                                                                  \
    [[nodiscard]] constexpr unit_t<double, Tag> operator""_##suffix(unsigned long long val)                            \
    {                                                                                                                  \
        return unit_t<double, Tag>{static_cast<double>(val)};                                                          \
    }

STRONG_TYPES_DEFINE_UDL(m, LengthTag)
STRONG_TYPES_DEFINE_UDL(kg, MassTag)
STRONG_TYPES_DEFINE_UDL(s, TimeTag)
STRONG_TYPES_DEFINE_UDL(m2, AreaTag)
STRONG_TYPES_DEFINE_UDL(mps, SpeedTag)
STRONG_TYPES_DEFINE_UDL(mps2, AccelerationTag)
STRONG_TYPES_DEFINE_UDL(N, ForceTag)
STRONG_TYPES_DEFINE_UDL(J, EnergyTag)
STRONG_TYPES_DEFINE_UDL(Hz, HertzTag)
STRONG_TYPES_DEFINE_UDL(degC, CelsiusTag)
STRONG_TYPES_DEFINE_UDL(V, VoltTag)
STRONG_TYPES_DEFINE_UDL(rad, RadianTag)
STRONG_TYPES_DEFINE_UDL(sr, SteradianTag)
STRONG_TYPES_DEFINE_UDL(W, PowerTag)
STRONG_TYPES_DEFINE_UDL(Pa, PressureTag)
STRONG_TYPES_DEFINE_UDL(rps, AngularVelocityTag)
STRONG_TYPES_DEFINE_UDL(m3, VolumeTag)
STRONG_TYPES_DEFINE_UDL(Nm, TorqueTag)

#undef STRONG_TYPES_DEFINE_UDL
// NOLINTEND(cppcoreguidelines-macro-usage)

} // namespace strong_types::si_literals
