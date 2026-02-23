#pragma once

#include "si.hpp"
#include "si_scaled.hpp"

#include <fmt/format.h>
#include <string_view>

namespace strong_types
{

// ---- tag → suffix mapping ----

template <typename Tag>
struct tag_suffix
{
    static constexpr std::string_view value = "";
};

template <>
struct tag_suffix<LengthTag>
{
    static constexpr std::string_view value = "m";
};
template <>
struct tag_suffix<MassTag>
{
    static constexpr std::string_view value = "kg";
};
template <>
struct tag_suffix<TimeTag>
{
    static constexpr std::string_view value = "s";
};
template <>
struct tag_suffix<AreaTag>
{
    static constexpr std::string_view value = "m2";
};
template <>
struct tag_suffix<SpeedTag>
{
    static constexpr std::string_view value = "m/s";
};
template <>
struct tag_suffix<AccelerationTag>
{
    static constexpr std::string_view value = "m/s2";
};
template <>
struct tag_suffix<ForceTag>
{
    static constexpr std::string_view value = "N";
};
template <>
struct tag_suffix<EnergyTag>
{
    static constexpr std::string_view value = "J";
};
template <>
struct tag_suffix<HertzTag>
{
    static constexpr std::string_view value = "Hz";
};
template <>
struct tag_suffix<CelsiusTag>
{
    static constexpr std::string_view value = "degC";
};
template <>
struct tag_suffix<VoltTag>
{
    static constexpr std::string_view value = "V";
};
template <>
struct tag_suffix<RadianTag>
{
    static constexpr std::string_view value = "rad";
};
template <>
struct tag_suffix<SteradianTag>
{
    static constexpr std::string_view value = "sr";
};

// ---- scaled unit → suffix override ----

template <typename Tag, typename Ratio>
struct scaled_suffix
{
    static constexpr std::string_view value = tag_suffix<Tag>::value;
};

// Length scales
template <>
struct scaled_suffix<LengthTag, std::kilo>
{
    static constexpr std::string_view value = "km";
};
template <>
struct scaled_suffix<LengthTag, std::centi>
{
    static constexpr std::string_view value = "cm";
};
template <>
struct scaled_suffix<LengthTag, std::milli>
{
    static constexpr std::string_view value = "mm";
};

// Time scales
template <>
struct scaled_suffix<TimeTag, std::ratio<3600>>
{
    static constexpr std::string_view value = "hr";
};
template <>
struct scaled_suffix<TimeTag, std::ratio<60>>
{
    static constexpr std::string_view value = "min";
};
template <>
struct scaled_suffix<TimeTag, std::milli>
{
    static constexpr std::string_view value = "ms";
};
template <>
struct scaled_suffix<TimeTag, std::micro>
{
    static constexpr std::string_view value = "us";
};
template <>
struct scaled_suffix<TimeTag, std::nano>
{
    static constexpr std::string_view value = "ns";
};

// Mass scales (base = kg)
template <>
struct scaled_suffix<MassTag, std::ratio<1, 1000>>
{
    static constexpr std::string_view value = "g";
};
template <>
struct scaled_suffix<MassTag, std::ratio<1, 1000000>>
{
    static constexpr std::string_view value = "mg";
};

} // namespace strong_types

// ---- fmt::formatter for Strong<T, Tag> ----

template <typename T, typename Tag>
struct fmt::formatter<strong_types::Strong<T, Tag>> : fmt::formatter<T>
{
    template <typename FormatContext>
    auto format(const strong_types::Strong<T, Tag> &val, FormatContext &ctx) const
    {
        fmt::formatter<T>::format(val.get(), ctx);
        constexpr auto suffix = strong_types::tag_suffix<Tag>::value;
        if constexpr (!suffix.empty())
        {
            fmt::format_to(ctx.out(), " {}", suffix);
        }
        return ctx.out();
    }
};

// ---- fmt::formatter for ScaledUnit<T, Tag, Ratio> ----

template <typename T, typename Tag, typename Ratio>
struct fmt::formatter<strong_types::ScaledUnit<T, Tag, Ratio>> : fmt::formatter<T>
{
    template <typename FormatContext>
    auto format(const strong_types::ScaledUnit<T, Tag, Ratio> &val, FormatContext &ctx) const
    {
        fmt::formatter<T>::format(val.get(), ctx);
        constexpr auto suffix = strong_types::scaled_suffix<Tag, Ratio>::value;
        if constexpr (!suffix.empty())
        {
            fmt::format_to(ctx.out(), " {}", suffix);
        }
        return ctx.out();
    }
};
