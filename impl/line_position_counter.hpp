#pragma once


#include <cstdint>
#include <string_view>


namespace NCompileTimeJsonParser {
    struct TLinePositionCounter {
        bool CanStepBack : 1 = false;
        uint16_t LineNumber : 15 = 0;
        uint16_t Position = 0;
        uint16_t PrevPosition = 0;

        constexpr auto Copy() const noexcept -> TLinePositionCounter {
            return *this;
        }

        constexpr auto Process(char ch) noexcept -> TLinePositionCounter& {
            PrevPosition = Position;
            CanStepBack = true;
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
            if (!CanStepBack) return *this;
            if (Position == 0 && LineNumber != 0) --LineNumber;
            Position = PrevPosition;
            CanStepBack = false;
            return *this;
        }
    };
}