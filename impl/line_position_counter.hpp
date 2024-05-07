#pragma once


#include <cstdint>
#include <string_view>


namespace NJsonParser {
    struct LinePositionCounter {
        uint16_t LineNumber = 0;
        uint16_t Position = 0;

        constexpr auto Copy() const noexcept -> LinePositionCounter {
            return *this;
        }

        constexpr auto Process(char ch) noexcept -> LinePositionCounter& {
            if (ch == '\n') {
                ++LineNumber;
                Position = 0;
            } else {
                ++Position;
            }
            return *this;
        }
        constexpr auto Process(std::string_view str) noexcept -> LinePositionCounter& {
            for (char ch : str) Process(ch);
            return *this;
        }
    };
}
