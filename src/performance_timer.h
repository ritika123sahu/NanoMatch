#pragma once

#include <mach/mach_time.h>
#include <cstdint>

namespace NanoMatch {

/**
 * @brief High-precision timer for macOS using mach_absolute_time().
 */
class PerformanceTimer {
public:
    PerformanceTimer() {
        mach_timebase_info(&timebase_info_);
    }

    inline uint64_t now() const {
        return mach_absolute_time();
    }

    inline uint64_t to_nanoseconds(uint64_t ticks) const {
        return (ticks * timebase_info_.numer) / timebase_info_.denom;
    }

private:
    mach_timebase_info_data_t timebase_info_;
};

} // namespace NanoMatch
