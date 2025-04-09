#pragma once

namespace strong_types
{

    struct LengthTag;
    struct MassTag;
    struct TimeTag;
    struct SpeedTag;
    struct AccelerationTag;
    struct ForceTag;
    struct EnergyTag;

    template <typename Tag>
    struct tag_traits; // forward decl only

    // 🧠 actual specializations:
    template <>
    struct tag_traits<LengthTag>
    {
        using value_type = float;
    };

    template <>
    struct tag_traits<MassTag>
    {
        using value_type = float;
    };

    template <>
    struct tag_traits<TimeTag>
    {
        using value_type = float;
    };

    template <>
    struct tag_traits<SpeedTag>
    {
        using value_type = float;
    };

    template <>
    struct tag_traits<AccelerationTag>
    {
        using value_type = float;
    };

    template <>
    struct tag_traits<ForceTag>
    {
        using value_type = float;
    };

    template <>
    struct tag_traits<EnergyTag>
    {
        using value_type = float;
    };

} // namespace strong_types
