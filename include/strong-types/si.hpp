#pragma once

#include "strong.hpp"

namespace strong_types
{

// ---- dimension exponent vector ----

// Exponents of length, mass, time, current, temperature, amount, luminous intensity, plane angle.
// Angle is a dimension here so Hertz differs from AngularVelocity and Energy from Torque (J/rad).
template <int L = 0, int M = 0, int T = 0, int I = 0, int Th = 0, int N = 0, int J = 0, int A = 0>
struct Dim
{
    using dimension = Dim;
};

template <typename Lhs, typename Rhs>
struct dim_product;
template <int... L, int... R>
struct dim_product<Dim<L...>, Dim<R...>>
{
    using type = Dim<(L + R)...>;
};

template <typename Lhs, typename Rhs>
struct dim_quotient;
template <int... L, int... R>
struct dim_quotient<Dim<L...>, Dim<R...>>
{
    using type = Dim<(L - R)...>;
};

// ---- tag -> dimension; void is the scalar tag ----

template <typename Tag>
struct dimension_of;

template <typename Tag>
    requires requires { typename Tag::dimension; }
struct dimension_of<Tag>
{
    using type = typename Tag::dimension;
};

template <>
struct dimension_of<void>
{
    using type = Dim<>;
};

template <typename Tag>
concept Dimensioned = requires { typename dimension_of<Tag>::type; };

// ---- dimension -> result tag; an unnamed dimension is its own tag ----

template <typename D>
struct canonical_tag
{
    using type = D;
};

template <>
struct canonical_tag<Dim<>>
{
    using type = void;
};

// ---- SI tags: each owns its dimension and is the result tag for it ----

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define STRONG_TYPES_SI_TAG(Tag, ...)                                                                                  \
    struct Tag : Dim<__VA_ARGS__>                                                                                      \
    {                                                                                                                  \
    };                                                                                                                 \
    template <>                                                                                                        \
    struct canonical_tag<Dim<__VA_ARGS__>>                                                                             \
    {                                                                                                                  \
        using type = Tag;                                                                                              \
    }

STRONG_TYPES_SI_TAG(LengthTag, 1);
STRONG_TYPES_SI_TAG(MassTag, 0, 1);
STRONG_TYPES_SI_TAG(TimeTag, 0, 0, 1);
STRONG_TYPES_SI_TAG(AreaTag, 2);
STRONG_TYPES_SI_TAG(VolumeTag, 3);
STRONG_TYPES_SI_TAG(SpeedTag, 1, 0, -1);
STRONG_TYPES_SI_TAG(AccelerationTag, 1, 0, -2);
STRONG_TYPES_SI_TAG(ForceTag, 1, 1, -2);
STRONG_TYPES_SI_TAG(PressureTag, -1, 1, -2);
STRONG_TYPES_SI_TAG(EnergyTag, 2, 1, -2);
STRONG_TYPES_SI_TAG(PowerTag, 2, 1, -3);
STRONG_TYPES_SI_TAG(HertzTag, 0, 0, -1);
STRONG_TYPES_SI_TAG(DensityTag, -3, 1);
STRONG_TYPES_SI_TAG(VoltTag, 2, 1, -3, -1);
STRONG_TYPES_SI_TAG(TemperatureTag, 0, 0, 0, 0, 1);
STRONG_TYPES_SI_TAG(RadianTag, 0, 0, 0, 0, 0, 0, 0, 1);
STRONG_TYPES_SI_TAG(SteradianTag, 0, 0, 0, 0, 0, 0, 0, 2);
STRONG_TYPES_SI_TAG(AngularVelocityTag, 0, 0, -1, 0, 0, 0, 0, 1);
STRONG_TYPES_SI_TAG(TorqueTag, 2, 1, -2, 0, 0, 0, 0, -1);
#undef STRONG_TYPES_SI_TAG
// NOLINTEND(cppcoreguidelines-macro-usage)

// alias
template <typename T, typename Tag>
using unit_t = Strong<T, Tag>;

// ---- tag algebra for dimensioned tags; explicit specializations still override ----

template <Dimensioned Tag>
struct tag_sum_result<Tag, Tag>
{
    using type = Tag;
};

template <Dimensioned Tag>
struct tag_difference_result<Tag, Tag>
{
    using type = Tag;
};

template <Dimensioned L, Dimensioned R>
struct tag_product_result<L, R>
{
    using type = typename canonical_tag<
        typename dim_product<typename dimension_of<L>::type, typename dimension_of<R>::type>::type>::type;
};

template <Dimensioned L, Dimensioned R>
struct tag_quotient_result<L, R>
{
    using type = typename canonical_tag<
        typename dim_quotient<typename dimension_of<L>::type, typename dimension_of<R>::type>::type>::type;
};

} // namespace strong_types
