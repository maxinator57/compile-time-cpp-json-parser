#pragma once

#include "api.hpp"
#include "line_position_counter.hpp"

#include <type_traits>


namespace NCompileTimeJsonParser::NError {
    enum class ErrorCode : uint8_t {
        SyntaxError = 1,
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

    template <ErrorCode code, size_t lineNumber, size_t position>
    struct Print {
        static_assert(static_cast<int>(code) == 0);
    };
    template <TError err>
    struct PrintErr : Print<err.Code, err.LineNumber, err.Position> {};
}
