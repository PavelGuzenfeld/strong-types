#pragma once

#include "strong.hpp"

namespace strong_types
{

    // SI base + derived tags
    struct LengthTag;
    struct MassTag;
    struct TimeTag;
    struct AreaTag;
    struct SpeedTag;
    struct AccelerationTag;
    struct ForceTag;
    struct EnergyTag;
    struct HertzTag;
    struct CelsiusTag;
    struct VoltTag;
    struct RadianTag;
    struct SteradianTag;

    // alias
    template <typename T, typename Tag>
    using unit_t = Strong<T, Tag>;

    // --- tag-level traits ---
    template <typename L, typename R>
    struct tag_sum_result;
    template <typename L, typename R>
    struct tag_difference_result;
    template <typename L, typename R>
    struct tag_product_result;
    template <typename L, typename R>
    struct tag_quotient_result;

    template <>
    struct tag_quotient_result<void, TimeTag>
    {
        using type = HertzTag;
    };

#define DEFINE_ADD_SUB(Tag)                \
    template <>                            \
    struct tag_sum_result<Tag, Tag>        \
    {                                      \
        using type = Tag;                  \
    };                                     \
    template <>                            \
    struct tag_difference_result<Tag, Tag> \
    {                                      \
        using type = Tag;                  \
    }

    DEFINE_ADD_SUB(LengthTag);
    DEFINE_ADD_SUB(TimeTag);
    DEFINE_ADD_SUB(SpeedTag);
    DEFINE_ADD_SUB(AccelerationTag);
    DEFINE_ADD_SUB(ForceTag);
    DEFINE_ADD_SUB(EnergyTag);
    DEFINE_ADD_SUB(AreaTag);
    DEFINE_ADD_SUB(HertzTag);
    DEFINE_ADD_SUB(CelsiusTag);
    DEFINE_ADD_SUB(VoltTag);
    DEFINE_ADD_SUB(RadianTag);
    DEFINE_ADD_SUB(SteradianTag);
#undef DEFINE_ADD_SUB

    // ---- tag-level product results ----
    template <>
    struct tag_product_result<LengthTag, LengthTag>
    {
        using type = AreaTag;
    };
    template <>
    struct tag_product_result<SpeedTag, TimeTag>
    {
        using type = LengthTag;
    };
    template <>
    struct tag_product_result<MassTag, AccelerationTag>
    {
        using type = ForceTag;
    };
    template <>
    struct tag_product_result<ForceTag, LengthTag>
    {
        using type = EnergyTag;
    };

    // commutative
    template <>
    struct tag_product_result<TimeTag, SpeedTag>
    {
        using type = LengthTag;
    };
    template <>
    struct tag_product_result<AccelerationTag, MassTag>
    {
        using type = ForceTag;
    };
    template <>
    struct tag_product_result<LengthTag, ForceTag>
    {
        using type = EnergyTag;
    };

    // ---- tag-level quotient results ----
    template <>
    struct tag_quotient_result<LengthTag, TimeTag>
    {
        using type = SpeedTag;
    };
    template <>
    struct tag_quotient_result<SpeedTag, TimeTag>
    {
        using type = AccelerationTag;
    };
    template <>
    struct tag_quotient_result<LengthTag, LengthTag>
    {
        using type = void;
    }; // scalar
    template <>
    struct tag_quotient_result<TimeTag, TimeTag>
    {
        using type = void;
    }; // scalar

} // namespace strong_types
