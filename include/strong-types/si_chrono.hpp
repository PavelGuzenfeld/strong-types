#pragma once

#include "si_scaled.hpp"

#include <chrono>
#include <ctime>

#if __has_include(<sys/time.h>)
#include <sys/time.h>
#define STRONG_TYPES_HAS_TIMEVAL 1
#else
#define STRONG_TYPES_HAS_TIMEVAL 0
#endif

namespace strong_types
{

// ---- concept: any time-tagged unit_t or ScaledUnit ----

template <typename U>
concept TimeUnit = (is_strong_v<U> && std::is_same_v<typename U::tag_type, TimeTag>) ||
                   (is_scaled_v<U> && std::is_same_v<typename U::tag_type, TimeTag>);

// ---- from_chrono: chrono::duration → unit_t<double, TimeTag> (seconds) ----

template <typename Rep, typename Period>
[[nodiscard]] constexpr unit_t<double, TimeTag> from_chrono(std::chrono::duration<Rep, Period> dur) noexcept
{
    using Seconds = std::chrono::duration<double>;
    return unit_t<double, TimeTag>{std::chrono::duration_cast<Seconds>(dur).count()};
}

// ---- from_chrono_as: chrono::duration → ScaledUnit<double, TimeTag, TargetRatio> ----

template <typename TargetRatio, typename Rep, typename Period>
[[nodiscard]] constexpr ScaledUnit<double, TimeTag, TargetRatio>
from_chrono_as(std::chrono::duration<Rep, Period> dur) noexcept
{
    using Target = std::chrono::duration<double, TargetRatio>;
    return ScaledUnit<double, TimeTag, TargetRatio>{std::chrono::duration_cast<Target>(dur).count()};
}

// ---- to_chrono: unit_t<T, TimeTag> → chrono::duration<double> (seconds) ----

template <typename T>
[[nodiscard]] constexpr std::chrono::duration<double> to_chrono(unit_t<T, TimeTag> val) noexcept
{
    return std::chrono::duration<double>{static_cast<double>(val.get())};
}

// ---- to_chrono: ScaledUnit<T, TimeTag, R> → chrono::duration<double, R> ----

template <typename T, typename R>
[[nodiscard]] constexpr std::chrono::duration<double, R> to_chrono(ScaledUnit<T, TimeTag, R> val) noexcept
{
    return std::chrono::duration<double, R>{static_cast<double>(val.get())};
}

// ---- to_chrono_as: unit_t<T, TimeTag> → specific chrono duration type ----

template <typename Dur, typename T>
[[nodiscard]] constexpr Dur to_chrono_as(unit_t<T, TimeTag> val) noexcept
{
    return std::chrono::duration_cast<Dur>(std::chrono::duration<double>{static_cast<double>(val.get())});
}

// ---- to_chrono_as: ScaledUnit<T, TimeTag, R> → specific chrono duration type ----

template <typename Dur, typename T, typename R>
[[nodiscard]] constexpr Dur to_chrono_as(ScaledUnit<T, TimeTag, R> val) noexcept
{
    return std::chrono::duration_cast<Dur>(std::chrono::duration<double, R>{static_cast<double>(val.get())});
}

// ---- from_timespec: struct timespec → unit_t<double, TimeTag> (seconds) ----

[[nodiscard]] constexpr unit_t<double, TimeTag> from_timespec(const struct timespec &ts) noexcept
{
    return unit_t<double, TimeTag>{static_cast<double>(ts.tv_sec) +
                                   static_cast<double>(ts.tv_nsec) / 1'000'000'000.0};
}

// ---- from_timespec_as_ns: struct timespec → Nanoseconds<double> ----

[[nodiscard]] constexpr Nanoseconds<double> from_timespec_as_ns(const struct timespec &ts) noexcept
{
    return Nanoseconds<double>{static_cast<double>(ts.tv_sec) * 1'000'000'000.0 + static_cast<double>(ts.tv_nsec)};
}

// ---- to_timespec: unit_t<T, TimeTag> → struct timespec ----

template <typename T>
[[nodiscard]] constexpr struct timespec to_timespec(unit_t<T, TimeTag> val) noexcept
{
    auto secs = static_cast<double>(val.get());
    auto whole = static_cast<std::time_t>(secs);
    auto frac = secs - static_cast<double>(whole);
    // Handle negative fractions: timespec requires tv_nsec in [0, 999999999]
    if (frac < 0.0)
    {
        --whole;
        frac += 1.0;
    }
    return {whole, static_cast<long>(frac * 1'000'000'000.0 + 0.5)};
}

// ---- to_timespec: ScaledUnit<T, TimeTag, R> → struct timespec ----

template <typename T, typename R>
[[nodiscard]] constexpr struct timespec to_timespec(ScaledUnit<T, TimeTag, R> val) noexcept
{
    return to_timespec(val.to_base());
}

// ---- timeval functions (POSIX only) ----

#if STRONG_TYPES_HAS_TIMEVAL

// ---- from_timeval: struct timeval → unit_t<double, TimeTag> (seconds) ----

[[nodiscard]] constexpr unit_t<double, TimeTag> from_timeval(const struct timeval &tv) noexcept
{
    return unit_t<double, TimeTag>{static_cast<double>(tv.tv_sec) +
                                   static_cast<double>(tv.tv_usec) / 1'000'000.0};
}

// ---- from_timeval_as_us: struct timeval → Microseconds<double> ----

[[nodiscard]] constexpr Microseconds<double> from_timeval_as_us(const struct timeval &tv) noexcept
{
    return Microseconds<double>{static_cast<double>(tv.tv_sec) * 1'000'000.0 + static_cast<double>(tv.tv_usec)};
}

// ---- to_timeval: unit_t<T, TimeTag> → struct timeval ----

template <typename T>
[[nodiscard]] constexpr struct timeval to_timeval(unit_t<T, TimeTag> val) noexcept
{
    auto secs = static_cast<double>(val.get());
    auto whole = static_cast<std::time_t>(secs);
    auto frac = secs - static_cast<double>(whole);
    if (frac < 0.0)
    {
        --whole;
        frac += 1.0;
    }
    return {whole, static_cast<suseconds_t>(frac * 1'000'000.0 + 0.5)};
}

// ---- to_timeval: ScaledUnit<T, TimeTag, R> → struct timeval ----

template <typename T, typename R>
[[nodiscard]] constexpr struct timeval to_timeval(ScaledUnit<T, TimeTag, R> val) noexcept
{
    return to_timeval(val.to_base());
}

#endif // STRONG_TYPES_HAS_TIMEVAL

} // namespace strong_types
