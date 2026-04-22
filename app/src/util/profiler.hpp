#ifndef GZ_PROFILER_HPP
#define GZ_PROFILER_HPP

#include <chrono>
#include <string>
#include "common_types.hpp"

namespace Geez {
    
    // TODO: Move this to something like gz_time.hpp
    // For engine time related things
    enum TimeType {
        MILLISECONDS,
        SECONDS,
        MINUTES
    };

    template <TimeType T>
    struct RaiiTimer {

        typedef std::chrono::steady_clock clock;
        typedef std::chrono::duration<F32> fseconds;
        typedef std::chrono::duration<F32, std::ratio<60>> fminutes;

        std::chrono::time_point<clock> start;
        std::string* out = nullptr;

        RaiiTimer(std::string* output) : out(output) {
            start = clock::now();
        }

        ~RaiiTimer() {
            using namespace std::chrono;
            auto d = clock::now() - start;

            if (!out) return;
            if constexpr (T == MILLISECONDS)
                *out = std::to_string(duration_cast<milliseconds>(d).count()) + "ms";
            else if constexpr (T == SECONDS)
                *out = std::to_string(duration_cast<fseconds>(d).count()) + "s";
            else if constexpr (T == MINUTES)
                *out = std::to_string(duration_cast<fminutes>(d).count()) + "min";
        }
    };
}

#endif