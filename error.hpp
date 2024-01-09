#pragma once

#include "line_position_counter.hpp"

#include <stdexcept>


namespace NCompileTimeJsonParser::NError {
    enum class ErrorCode : uint8_t {
        SyntaxError,
        TypeError, 
        MissingValueError,
        ArrayIndexOutOfRange,
        MappingKeyNotFound,
        IteratorDereferenceError,
        CompileTimeStackCapacityExceededError,
    };
    struct TError {
        size_t LineNumber = 0;
        size_t Position = 0;
        ErrorCode Code; 
    };

    constexpr auto Error(TLinePositionCounter lpCounter, ErrorCode code) -> TError {
        return {
            .LineNumber = lpCounter.LineNumber,
            .Position = lpCounter.Position,
            .Code = code,
        };
    }

    struct IteratorIncrementError : std::runtime_error {
        using std::runtime_error::runtime_error;
    };
}
