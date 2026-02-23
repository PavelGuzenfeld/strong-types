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
    struct PowerTag;
    struct PressureTag;
    struct AngularVelocityTag;
    struct VolumeTag;
    struct DensityTag;
    struct TorqueTag;

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
    DEFINE_ADD_SUB(MassTag);
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
    DEFINE_ADD_SUB(PowerTag);
    DEFINE_ADD_SUB(PressureTag);
    DEFINE_ADD_SUB(AngularVelocityTag);
    DEFINE_ADD_SUB(VolumeTag);
    DEFINE_ADD_SUB(DensityTag);
    DEFINE_ADD_SUB(TorqueTag);
#undef DEFINE_ADD_SUB

    // ---- same-tag quotient → scalar ----

#define DEFINE_SELF_QUOTIENT(Tag)                    \
    template <>                                      \
    struct tag_quotient_result<Tag, Tag>             \
    {                                                \
        using type = void;                           \
    }

    DEFINE_SELF_QUOTIENT(LengthTag);
    DEFINE_SELF_QUOTIENT(MassTag);
    DEFINE_SELF_QUOTIENT(TimeTag);
    DEFINE_SELF_QUOTIENT(SpeedTag);
    DEFINE_SELF_QUOTIENT(AccelerationTag);
    DEFINE_SELF_QUOTIENT(ForceTag);
    DEFINE_SELF_QUOTIENT(EnergyTag);
    DEFINE_SELF_QUOTIENT(AreaTag);
    DEFINE_SELF_QUOTIENT(HertzTag);
    DEFINE_SELF_QUOTIENT(RadianTag);
    DEFINE_SELF_QUOTIENT(SteradianTag);
    DEFINE_SELF_QUOTIENT(PowerTag);
    DEFINE_SELF_QUOTIENT(PressureTag);
    DEFINE_SELF_QUOTIENT(AngularVelocityTag);
    DEFINE_SELF_QUOTIENT(VolumeTag);
    DEFINE_SELF_QUOTIENT(DensityTag);
    DEFINE_SELF_QUOTIENT(TorqueTag);
#undef DEFINE_SELF_QUOTIENT

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
    template <>
    struct tag_product_result<PowerTag, TimeTag>
    {
        using type = EnergyTag;
    };
    template <>
    struct tag_product_result<PressureTag, AreaTag>
    {
        using type = ForceTag;
    };
    template <>
    struct tag_product_result<AngularVelocityTag, TimeTag>
    {
        using type = RadianTag;
    };
    template <>
    struct tag_product_result<LengthTag, AreaTag>
    {
        using type = VolumeTag;
    };
    template <>
    struct tag_product_result<DensityTag, VolumeTag>
    {
        using type = MassTag;
    };
    template <>
    struct tag_product_result<TorqueTag, AngularVelocityTag>
    {
        using type = PowerTag;
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
    template <>
    struct tag_product_result<TimeTag, PowerTag>
    {
        using type = EnergyTag;
    };
    template <>
    struct tag_product_result<AreaTag, PressureTag>
    {
        using type = ForceTag;
    };
    template <>
    struct tag_product_result<TimeTag, AngularVelocityTag>
    {
        using type = RadianTag;
    };
    template <>
    struct tag_product_result<AreaTag, LengthTag>
    {
        using type = VolumeTag;
    };
    template <>
    struct tag_product_result<VolumeTag, DensityTag>
    {
        using type = MassTag;
    };
    template <>
    struct tag_product_result<AngularVelocityTag, TorqueTag>
    {
        using type = PowerTag;
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
    struct tag_quotient_result<EnergyTag, TimeTag>
    {
        using type = PowerTag;
    };
    template <>
    struct tag_quotient_result<ForceTag, AreaTag>
    {
        using type = PressureTag;
    };
    template <>
    struct tag_quotient_result<RadianTag, TimeTag>
    {
        using type = AngularVelocityTag;
    };
    template <>
    struct tag_quotient_result<VolumeTag, LengthTag>
    {
        using type = AreaTag;
    };
    template <>
    struct tag_quotient_result<VolumeTag, AreaTag>
    {
        using type = LengthTag;
    };
    template <>
    struct tag_quotient_result<MassTag, VolumeTag>
    {
        using type = DensityTag;
    };
    template <>
    struct tag_quotient_result<PowerTag, AngularVelocityTag>
    {
        using type = TorqueTag;
    };
    template <>
    struct tag_quotient_result<PowerTag, TorqueTag>
    {
        using type = AngularVelocityTag;
    };

} // namespace strong_types
