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
            case SyntaxError:
                return "syntax error";
            case TypeError:
                return "type error";
            case MissingValueError:
                return "missing value error";
            case ArrayIndexOutOfRange:
                return "array index out of range";
            case MappingKeyNotFound:
                return "mapping key not found";
            case IteratorDereferenceError:
                return "iterator dereference error";
            case CompileTimeStackCapacityExceededError:
                return "compile-time stack capacity exceeded error";
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
    template <class TOstream>
    constexpr auto operator<<(TOstream& out, const TError& error) -> TOstream& {
        out << "Got " << ToStr(error.Code)
            << " at line " << error.LineNumber
            << ", position " << error.Position;
        return out;
    }

    template <ErrorCode code, size_t lineNumber, size_t position>
    struct Print {
        static_assert(static_cast<int>(code) == 0);
    };
    template <TError err>
    struct PrintErrAtCompileTime : Print<err.Code, err.LineNumber, err.Position> {};
}
