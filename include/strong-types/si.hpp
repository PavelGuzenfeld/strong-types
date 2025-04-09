#pragma once

#include "strong.hpp"
#include "tag_traits.hpp"

namespace strong_types
{

    // ---- tag -> internal type & Strong type mapping ----

    template <typename Tag>
    struct tag_traits; // forward declaration for customization

    template <typename Tag>
    using unit_t = Strong<typename strong_types::tag_traits<Tag>::value_type, Tag>;

    // ---- literals using unit_t<T> ----
    constexpr unit_t<LengthTag> operator"" _m(long double v) { return unit_t<LengthTag>{static_cast<typename tag_traits<LengthTag>::value_type>(v)}; }
    constexpr unit_t<MassTag> operator"" _kg(long double v) { return unit_t<MassTag>{static_cast<typename tag_traits<MassTag>::value_type>(v)}; }
    constexpr unit_t<TimeTag> operator"" _s(long double v) { return unit_t<TimeTag>{static_cast<typename tag_traits<TimeTag>::value_type>(v)}; }

    constexpr unit_t<LengthTag> operator"" _m(unsigned long long v) { return unit_t<LengthTag>{static_cast<typename tag_traits<LengthTag>::value_type>(v)}; }
    constexpr unit_t<MassTag> operator"" _kg(unsigned long long v) { return unit_t<MassTag>{static_cast<typename tag_traits<MassTag>::value_type>(v)}; }
    constexpr unit_t<TimeTag> operator"" _s(unsigned long long v) { return unit_t<TimeTag>{static_cast<typename tag_traits<TimeTag>::value_type>(v)}; }


// additive closure for all types
#define DEFINE_ADD_SUB(Tag)                                    \
    template <>                                                \
    struct sum_result<unit_t<Tag>, unit_t<Tag>>        \
    {                                                          \
        using type = unit_t<Tag>;                          \
    };                                                         \
    template <>                                                \
    struct difference_result<unit_t<Tag>, unit_t<Tag>> \
    {                                                          \
        using type = unit_t<Tag>;                          \
    };

    DEFINE_ADD_SUB(LengthTag)
    DEFINE_ADD_SUB(TimeTag)
    DEFINE_ADD_SUB(SpeedTag)
    DEFINE_ADD_SUB(AccelerationTag)
    DEFINE_ADD_SUB(ForceTag)
    DEFINE_ADD_SUB(EnergyTag)

#undef DEFINE_ADD_SUB

    template <>
    struct quotient_result<unit_t<LengthTag>, unit_t<TimeTag>>
    {
        using type = unit_t<SpeedTag>;
    };

    template <>
    struct quotient_result<unit_t<SpeedTag>, unit_t<TimeTag>>
    {
        using type = unit_t<AccelerationTag>;
    };

    template <>
    struct product_result<unit_t<MassTag>, unit_t<AccelerationTag>>
    {
        using type = unit_t<ForceTag>;
    };

    template <>
    struct product_result<unit_t<ForceTag>, unit_t<LengthTag>>
    {
        using type = unit_t<EnergyTag>;
    };

    template <>
    struct product_result<unit_t<SpeedTag>, unit_t<TimeTag>>
    {
        using type = unit_t<LengthTag>;
    };

    // scalar output
    template <>
    struct quotient_result<unit_t<LengthTag>, unit_t<LengthTag>>
    {
        using type = typename tag_traits<LengthTag>::value_type;
    };

}
