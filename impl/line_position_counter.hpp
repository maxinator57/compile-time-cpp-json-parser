#pragma once


#include <cstddef>
#include <string_view>


namespace NCompileTimeJsonParser {
    struct TLinePositionCounter {
        size_t LineNumber = 0;
        size_t Position = 0;
        size_t PrevPosition = 0;

        constexpr auto Copy() const noexcept -> TLinePositionCounter {
            return *this;
        }

        constexpr auto Process(char ch) noexcept -> TLinePositionCounter& {
            PrevPosition = Position;
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

        // Can only revert one last operation
        constexpr auto StepBack() noexcept -> TLinePositionCounter& {
            if (Position == 0 && LineNumber != 0) --LineNumber;
            Position = PrevPosition;
            return *this;
        }
    };
}