#pragma once


#include "line_position_counter.hpp"

#include <string_view>


namespace NCompileTimeJsonParser {
    // A mixin class that provides the functionality of
    //   1. holding a std::string_view to a (part of) text containing json struct representation and
    //   2. keeping track of the current line and position in the original text using `TLinePositionCounter`
    //
    // Inheriting publicly from `TDataholderMixin` allows classes such as `TJsonValue`
    // provide the information about the location of the error in the text (i.e. line number and position in this line)
    // when an error is encountered 
    class TDataHolderMixin {
    protected:
        std::string_view Data;
        TLinePositionCounter LpCounter;
    public:
        constexpr TDataHolderMixin(std::string_view data, const TLinePositionCounter& lpCounter)
            : Data(data), LpCounter(lpCounter) {}
        constexpr auto GetData() const -> std::string_view {
            return Data;
        }
        constexpr auto GetLpCounter() const -> const TLinePositionCounter& {
            return LpCounter;
        }
    };
}
