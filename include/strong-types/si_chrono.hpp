#pragma once

#include "si_scaled.hpp"

#include <chrono>
#include <cstdint>
#include <ctime>
#include <exception>
#include <ratio>
#include <type_traits>

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#if __has_include(<sys/time.h>)
#include <sys/time.h>
#define STRONG_TYPES_HAS_TIMEVAL 1
#else
#define STRONG_TYPES_HAS_TIMEVAL 0
#endif
// NOLINTEND(cppcoreguidelines-macro-usage)

namespace strong_types
{

// ---- concept: any time-tagged unit_t or ScaledUnit ----

template <typename U>
concept TimeUnit = (is_strong_v<U> && std::is_same_v<typename U::tag_type, TimeTag>) ||
                   (is_scaled_v<U> && std::is_same_v<typename U::tag_type, TimeTag>);

// An integral rep crosses to chrono unchanged; everything else travels as double.
template <typename T>
using chrono_rep_t = std::conditional_t<std::is_integral_v<T>, T, double>;

// ---- from_chrono: chrono::duration → unit_t<double, TimeTag> (seconds) ----

template <typename Rep, typename Period>
[[nodiscard]] constexpr unit_t<double, TimeTag> from_chrono(std::chrono::duration<Rep, Period> dur) noexcept
{
    using Seconds = std::chrono::duration<double>;
    return unit_t<double, TimeTag>{std::chrono::duration_cast<Seconds>(dur).count()};
}

// ---- from_chrono_as: chrono::duration → ScaledUnit<rep, TimeTag, TargetRatio>; integral reps convert exactly ----

template <typename TargetRatio, typename Rep, typename Period>
    requires ExactScale<chrono_rep_t<Rep>, std::ratio_divide<Period, TargetRatio>>
[[nodiscard]] constexpr ScaledUnit<chrono_rep_t<Rep>, TimeTag, TargetRatio> from_chrono_as(
    std::chrono::duration<Rep, Period> dur) noexcept
{
    using R = chrono_rep_t<Rep>;
    return ScaledUnit<R, TimeTag, TargetRatio>{
        rescale<std::ratio_divide<Period, TargetRatio>>(static_cast<R>(dur.count()))};
}

// ---- to_chrono: unit_t<T, TimeTag> → chrono::duration<rep> (seconds) ----

template <typename T>
[[nodiscard]] constexpr std::chrono::duration<chrono_rep_t<T>> to_chrono(unit_t<T, TimeTag> val) noexcept
{
    return std::chrono::duration<chrono_rep_t<T>>{static_cast<chrono_rep_t<T>>(val.get())};
}

// ---- to_chrono: ScaledUnit<T, TimeTag, R> → chrono::duration<rep, R> ----

template <typename T, typename R>
[[nodiscard]] constexpr std::chrono::duration<chrono_rep_t<T>, R> to_chrono(ScaledUnit<T, TimeTag, R> val) noexcept
{
    return std::chrono::duration<chrono_rep_t<T>, R>{static_cast<chrono_rep_t<T>>(val.get())};
}

// ---- to_chrono_as: any time unit → specific chrono duration type, with chrono's own duration_cast rules ----

template <typename Dur, TimeUnit U>
[[nodiscard]] constexpr Dur to_chrono_as(U val) noexcept
{
    return std::chrono::duration_cast<Dur>(to_chrono(val));
}

// ---- from_timespec: struct timespec → unit_t<double, TimeTag> (seconds) ----

[[nodiscard]] constexpr unit_t<double, TimeTag> from_timespec(const struct timespec &spec) noexcept
{
    return unit_t<double, TimeTag>{static_cast<double>(spec.tv_sec) +
                                   static_cast<double>(spec.tv_nsec) / static_cast<double>(std::nano::den)};
}

// ---- from_timespec_as_ns: struct timespec → Nanoseconds<int64>, exact; terminates past the int64 range ----

[[nodiscard]] constexpr Nanoseconds<std::int64_t> from_timespec_as_ns(const struct timespec &spec) noexcept
{
    const auto total =
        safe_multiply(static_cast<std::int64_t>(spec.tv_sec), static_cast<std::int64_t>(std::nano::den))
            .and_then([&spec](std::int64_t whole) { return safe_add(whole, static_cast<std::int64_t>(spec.tv_nsec)); });
    if (!total)
    {
        std::terminate();
    }
    return Nanoseconds<std::int64_t>{*total};
}

// ---- to_timespec: any time unit → struct timespec, rounded to the nearest nanosecond ----

// floor keeps tv_nsec in [0, 1e9) for negative times; the round-then-floor order carries 0.9999999999 s into tv_sec.
template <TimeUnit U>
[[nodiscard]] constexpr struct timespec to_timespec(U val) noexcept
{
    const auto total = std::chrono::round<std::chrono::nanoseconds>(to_chrono(val));
    const auto whole = std::chrono::floor<std::chrono::seconds>(total);
    return {static_cast<std::time_t>(whole.count()), static_cast<long>((total - whole).count())};
}

// ---- timeval functions (POSIX only) ----

#if STRONG_TYPES_HAS_TIMEVAL

// ---- from_timeval: struct timeval → unit_t<double, TimeTag> (seconds) ----

[[nodiscard]] constexpr unit_t<double, TimeTag> from_timeval(const struct timeval &tval) noexcept
{
    return unit_t<double, TimeTag>{static_cast<double>(tval.tv_sec) +
                                   static_cast<double>(tval.tv_usec) / static_cast<double>(std::micro::den)};
}

// ---- from_timeval_as_us: struct timeval → Microseconds<int64>, exact; terminates past the int64 range ----

[[nodiscard]] constexpr Microseconds<std::int64_t> from_timeval_as_us(const struct timeval &tval) noexcept
{
    const auto total =
        safe_multiply(static_cast<std::int64_t>(tval.tv_sec), static_cast<std::int64_t>(std::micro::den))
            .and_then([&tval](std::int64_t whole) { return safe_add(whole, static_cast<std::int64_t>(tval.tv_usec)); });
    if (!total)
    {
        std::terminate();
    }
    return Microseconds<std::int64_t>{*total};
}

// ---- to_timeval: any time unit → struct timeval, rounded to the nearest microsecond ----

template <TimeUnit U>
[[nodiscard]] constexpr struct timeval to_timeval(U val) noexcept
{
    const auto total = std::chrono::round<std::chrono::microseconds>(to_chrono(val));
    const auto whole = std::chrono::floor<std::chrono::seconds>(total);
    return {static_cast<std::time_t>(whole.count()), static_cast<suseconds_t>((total - whole).count())};
}

#endif // STRONG_TYPES_HAS_TIMEVAL

} // namespace strong_types
