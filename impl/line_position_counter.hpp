#pragma once


#include <cstddef>
#include <string_view>


namespace NCompileTimeJsonParser {
    struct TLinePositionCounter {
        size_t LineNumber = 0;
        size_t Position = 0;

        constexpr auto Copy() const -> TLinePositionCounter {
            return *this;
        }

        constexpr auto Process(char ch) -> TLinePositionCounter& {
            if (ch == '\n') {
                ++LineNumber;
                Position = 0;
            } else {
                ++Position;
            }
            return *this;
        }
        constexpr auto Process(std::string_view str) -> TLinePositionCounter& {
            for (char ch : str) Process(ch);
            return *this;
        }
    };
}