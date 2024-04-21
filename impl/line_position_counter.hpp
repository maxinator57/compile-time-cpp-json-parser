#pragma once


#include <cstdint>
#include <string_view>


namespace NCompileTimeJsonParser {
    struct TLinePositionCounter {
        uint16_t LineNumber = 0;
        uint16_t Position = 0;

        constexpr auto Copy() const noexcept -> TLinePositionCounter {
            return *this;
        }

        constexpr auto Process(char ch) noexcept -> TLinePositionCounter& {
            if (ch == '\n') {
                ++LineNumber;
                Position = 0;
            } else {
                ++Position;
            }
            return *this;
        }
        constexpr auto Process(std::string_view str) noexcept -> TLinePositionCounter& {
            for (char ch : str) Process(ch);
            return *this;
        }
    };
}
