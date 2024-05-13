#pragma once


#include "line_position_counter.hpp"


namespace NJsonParser {
    // A mixin class that provides the functionality of
    //   1. holding a `std::string_view` to a (part of) text containing json struct representation and
    //   2. keeping track of the current line and position in the original text using `LinePositionCounter`
    //
    // Inheriting publicly from `DataHolderMixin` allows classes such as `JsonValue` to provide the
    // information about the location of the error in the text (i.e. line number and position in this line)
    // when an error is encountered
    class DataHolderMixin {
    protected:
        std::string_view Data;
        LinePositionCounter LpCounter;
    public:
        constexpr DataHolderMixin(std::string_view data, LinePositionCounter lpCounter) noexcept
            : Data(data), LpCounter(lpCounter) {}
        constexpr auto GetData() const noexcept -> std::string_view {
            return Data;
        }
        constexpr auto GetLpCounter() const noexcept -> LinePositionCounter {
            return LpCounter;
        }
    };
}
