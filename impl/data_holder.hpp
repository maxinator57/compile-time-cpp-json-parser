#pragma once


#include "line_position_counter.hpp"

#include <string_view>


namespace NCompileTimeJsonParser {
    class TDataHolderMixin {
    protected:
        std::string_view Data;
        TLinePositionCounter LpCounter;
    public:
        constexpr TDataHolderMixin(const std::string_view& data, const TLinePositionCounter& lpCounter)
            : Data(data), LpCounter(lpCounter) {}
        constexpr auto GetData() const -> const std::string_view& {
            return Data;
        }
        constexpr auto GetLpCounter() const -> const TLinePositionCounter& {
            return LpCounter;
        }
    };
}
