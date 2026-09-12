// NOLINTBEGIN(readability-magic-numbers,readability-identifier-length) — test file
#include "strong-types/si_chrono.hpp"
#include "strong-types/si_literals.hpp"
#include "strong-types/si_scaled_literals.hpp"

#include <chrono>
#include <cstdint>
#include <type_traits>

using namespace strong_types;
using namespace strong_types::si_literals;
using namespace strong_types::si_scaled_literals;

// ---- Nanoseconds basics ----

static_assert((500.0_ns).get() == 500.0, "ns construction");
static_assert((1000.0_ns).to_base().get() == 0.000001, "1000ns = 1us = 0.000001s");
static_assert((1.0_us).in<std::nano>().get() == 1000.0, "1us = 1000ns");

// ---- from_chrono: chrono → seconds ----

static_assert(from_chrono(std::chrono::duration<double>(2.5)).get() == 2.5, "2.5s chrono → 2.5s strong");
static_assert(from_chrono(std::chrono::milliseconds(500)).get() == 0.5, "500ms chrono → 0.5s strong");
static_assert(from_chrono(std::chrono::microseconds(1000)).get() == 0.001, "1000us chrono → 0.001s");
static_assert(from_chrono(std::chrono::hours(1)).get() == 3600.0, "1hr chrono → 3600s");

// ---- from_chrono_as: chrono → specific scale ----

static_assert(from_chrono_as<std::milli>(std::chrono::duration<double>(1.5)).get() == 1500.0, "1.5s chrono → 1500ms");
static_assert(from_chrono_as<std::micro>(std::chrono::milliseconds(10)).get() == 10000, "10ms chrono → 10000us");
static_assert(std::is_same_v<decltype(from_chrono_as<std::micro>(std::chrono::milliseconds(10)))::value_type,
                             std::chrono::milliseconds::rep>,
              "an integral chrono rep stays integral");
static_assert(from_chrono_as<std::nano>(std::chrono::milliseconds(3)).get() == 3'000'000,
              "3ms chrono → 3000000ns exact");

template <typename TargetRatio, typename Dur>
concept CanFromChronoAs = requires(Dur d) { from_chrono_as<TargetRatio>(d); };

static_assert(!CanFromChronoAs<std::ratio<1>, std::chrono::milliseconds>, "integral 1ms → s would truncate, refused");
static_assert(CanFromChronoAs<std::ratio<1>, std::chrono::duration<double, std::milli>>,
              "floating 1ms → s stays available");
static_assert(from_chrono_as<std::ratio<1>>(std::chrono::duration<double, std::milli>(1.0)).get() == 0.001,
              "floating 1ms → 0.001s");

// ---- to_chrono: unit_t → chrono::duration<double> ----

static_assert(to_chrono(2.5_s).count() == 2.5, "2.5s strong → 2.5s chrono");
static_assert(to_chrono(0.001_s).count() == 0.001, "0.001s strong → chrono");

// ---- to_chrono: ScaledUnit → chrono::duration<double, R> ----

static_assert(to_chrono(500.0_ms).count() == 500.0, "500ms strong → 500 chrono-ms");
static_assert(to_chrono(2.0_hr).count() == 2.0, "2hr strong → 2 chrono-hr");

// ---- to_chrono_as: unit_t → specific chrono type ----

static_assert(to_chrono_as<std::chrono::duration<double, std::milli>>(1.5_s).count() == 1500.0, "1.5s → 1500ms chrono");

// ---- to_chrono_as: ScaledUnit → specific chrono type ----

static_assert(to_chrono_as<std::chrono::duration<double>>(500.0_ms).count() == 0.5, "500ms → 0.5s chrono");

// ---- round-trip: chrono → strong → chrono ----

static_assert(
    [] {
        constexpr auto orig = std::chrono::duration<double>(3.14);
        constexpr auto strong = from_chrono(orig);
        constexpr auto back = to_chrono(strong);
        return back.count() == orig.count();
    }(),
    "round-trip chrono→strong→chrono");

// ---- round-trip: strong → chrono → strong ----

static_assert(
    [] {
        constexpr auto orig = 42.0_s;
        constexpr auto dur = to_chrono(orig);
        constexpr auto back = from_chrono(dur);
        return back.get() == orig.get();
    }(),
    "round-trip strong→chrono→strong");

// ---- from_timespec ----

static_assert(
    [] {
        constexpr struct timespec ts = {2, 500000000L};
        return from_timespec(ts).get() == 2.5;
    }(),
    "timespec{2, 500000000} → 2.5s");

// ---- from_timespec_as_ns ----

static_assert(
    [] {
        constexpr struct timespec ts = {1, 500000000L};
        return from_timespec_as_ns(ts).get() == 1'500'000'000;
    }(),
    "timespec{1, 500000000} → 1500000000ns");

// ---- integral nanoseconds are exact where double loses the low bits ----

static_assert(std::is_same_v<decltype(from_timespec_as_ns(timespec{}))::value_type, std::int64_t>,
              "timespec → ns keeps an integral rep");
static_assert(
    [] {
        constexpr struct timespec epoch = {1'700'000'000, 123'456'789L};
        return from_timespec_as_ns(epoch).get() == 1'700'000'000'123'456'789;
    }(),
    "an epoch timespec converts to ns without losing a nanosecond");
static_assert(
    [] {
        constexpr struct timespec orig = {1'700'000'000, 123'456'789L};
        constexpr auto back = to_timespec(from_timespec_as_ns(orig));
        return back.tv_sec == orig.tv_sec && back.tv_nsec == orig.tv_nsec;
    }(),
    "round-trip timespec → int64 ns → timespec is exact at epoch magnitude");
static_assert(to_chrono(Nanoseconds<std::int64_t>{1'700'000'000'123'456'789}).count() == 1'700'000'000'123'456'789,
              "int64 ns → chrono keeps every nanosecond");
static_assert(std::is_same_v<decltype(to_chrono(Nanoseconds<std::int64_t>{0}))::rep, std::int64_t>,
              "int64 ns → chrono keeps an integral rep");
static_assert(to_chrono_as<std::chrono::milliseconds>(Nanoseconds<std::int64_t>{1'500'000}).count() == 1,
              "int64 ns → chrono ms truncates like duration_cast");
static_assert(
    [] {
        constexpr auto ts = to_timespec(Nanoseconds<std::int64_t>{-1});
        return ts.tv_sec == -1 && ts.tv_nsec == 999'999'999L;
    }(),
    "-1 ns splits to {-1, 999999999}, tv_nsec stays non-negative");
static_assert(
    [] {
        constexpr auto ts = to_timespec(unit_t<std::int64_t, TimeTag>{5});
        return ts.tv_sec == 5 && ts.tv_nsec == 0;
    }(),
    "integral seconds → timespec has no fraction");
static_assert(
    [] {
        constexpr auto ts = to_timespec(Minutes<int>{2});
        return ts.tv_sec == 120 && ts.tv_nsec == 0;
    }(),
    "int minutes → timespec");

// ---- a fraction that rounds up to a whole second carries into tv_sec ----

static_assert(
    [] {
        constexpr auto ts = to_timespec(unit_t<double, TimeTag>{0.99999999999});
        return ts.tv_sec == 1 && ts.tv_nsec == 0;
    }(),
    "0.99999999999 s → {1, 0}, never tv_nsec == 1e9");

// ---- to_timespec: basic ----

static_assert(
    [] {
        constexpr auto ts = to_timespec(2.5_s);
        return ts.tv_sec == 2 && ts.tv_nsec == 500000000L;
    }(),
    "2.5s → timespec{2, 500000000}");

// ---- to_timespec: from ScaledUnit ----

static_assert(
    [] {
        constexpr auto ts = to_timespec(1500.0_ms);
        return ts.tv_sec == 1 && ts.tv_nsec == 500000000L;
    }(),
    "1500ms → timespec{1, 500000000}");

// ---- round-trip: timespec → strong → timespec ----

static_assert(
    [] {
        constexpr struct timespec orig = {10, 250000000L};
        constexpr auto strong = from_timespec(orig);
        constexpr auto back = to_timespec(strong);
        return back.tv_sec == orig.tv_sec && back.tv_nsec == orig.tv_nsec;
    }(),
    "round-trip timespec→strong→timespec");

// ---- timeval tests (POSIX only) ----

#if STRONG_TYPES_HAS_TIMEVAL

// ---- from_timeval ----

static_assert(
    [] {
        constexpr struct timeval tv = {3, 500000};
        return from_timeval(tv).get() == 3.5;
    }(),
    "timeval{3, 500000} → 3.5s");

// ---- from_timeval_as_us ----

static_assert(
    [] {
        constexpr struct timeval tv = {1, 500000};
        return from_timeval_as_us(tv).get() == 1'500'000;
    }(),
    "timeval{1, 500000} → 1500000us");

static_assert(std::is_same_v<decltype(from_timeval_as_us(timeval{}))::value_type, std::int64_t>,
              "timeval → us keeps an integral rep");
static_assert(
    [] {
        constexpr struct timeval orig = {1'700'000'000, 123'456};
        constexpr auto back = to_timeval(from_timeval_as_us(orig));
        return from_timeval_as_us(orig).get() == 1'700'000'000'123'456 && back.tv_sec == orig.tv_sec &&
               back.tv_usec == orig.tv_usec;
    }(),
    "round-trip timeval → int64 us → timeval is exact at epoch magnitude");
static_assert(
    [] {
        constexpr auto tv = to_timeval(unit_t<double, TimeTag>{0.9999999999});
        return tv.tv_sec == 1 && tv.tv_usec == 0;
    }(),
    "0.9999999999 s → {1, 0}, never tv_usec == 1e6");

// ---- to_timeval: basic ----

static_assert(
    [] {
        constexpr auto tv = to_timeval(2.5_s);
        return tv.tv_sec == 2 && tv.tv_usec == 500000;
    }(),
    "2.5s → timeval{2, 500000}");

// ---- to_timeval: from ScaledUnit ----

static_assert(
    [] {
        constexpr auto tv = to_timeval(1500.0_ms);
        return tv.tv_sec == 1 && tv.tv_usec == 500000;
    }(),
    "1500ms → timeval{1, 500000}");

// ---- round-trip: timeval → strong → timeval ----

static_assert(
    [] {
        constexpr struct timeval orig = {10, 250000};
        constexpr auto strong = from_timeval(orig);
        constexpr auto back = to_timeval(strong);
        return back.tv_sec == orig.tv_sec && back.tv_usec == orig.tv_usec;
    }(),
    "round-trip timeval→strong→timeval");

#endif // STRONG_TYPES_HAS_TIMEVAL

// NOLINTEND(readability-magic-numbers,readability-identifier-length)
