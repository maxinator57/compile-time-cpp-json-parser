#pragma once

#include "api.hpp"
#include "line_position_counter.hpp"

#include <string_view>
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
    constexpr auto ToStr(ErrorCode code) -> std::string_view {
        using enum ErrorCode;
        switch (code) {
            case SyntaxError: return "Syntax error";
            case TypeError: return "Type error";
            case MissingValueError: return "Missing value error";
            case ArrayIndexOutOfRange: return "Array index out of range";
            case MappingKeyNotFound: return "Mapping key not found";
            case IteratorDereferenceError: return "Iterator dereference error";
            case CompileTimeStackCapacityExceededError: return "Compile-time stack capacity exceeded";
        }
        return {}; // to get rid of compiler warning
    };

    
    template <class TOstream>
    constexpr auto operator<<(TOstream& out, ErrorCode code) -> TOstream& {
        out << ToStr(code);
        return out;
    }

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
