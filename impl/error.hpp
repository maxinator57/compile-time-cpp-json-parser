#pragma once

#include "line_position_counter.hpp"

#include <cstddef>
#include <iterator>
#include <string_view>
#include <variant>


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
                return "\"missing value\" error";
            case ArrayIndexOutOfRange:
                return "\"array index out of range\" error";
            case MappingKeyNotFound:
                return "\"mapping key not found\" error";
            case IteratorDereferenceError:
                return "\"iterator dereference\" error";
            case CompileTimeStackCapacityExceededError:
                return "\"compile-time stack capacity exceeded\" error";
        }
        return {}; // to get rid of compiler warning
    }; 
    template <class TOstream>
    constexpr auto operator<<(TOstream& out, ErrorCode code) -> TOstream& {
        out << ToStr(code);
        return out;
    }

    struct TArrayIndexOutOfRangeAdditionalInfo {
        size_t Index =  0;
        size_t ArrayLen = 0;
        constexpr auto operator==(
            const TArrayIndexOutOfRangeAdditionalInfo& other
        ) const -> bool = default;
    };
    template <class TOstream>
    constexpr auto operator<<(
        TOstream& out,
        const TArrayIndexOutOfRangeAdditionalInfo& info
    ) -> TOstream& {
        out << "index " << info.Index << " is out of range for array of length " << info.ArrayLen;
        return out;
    }

    struct TError { 
        struct TBasicInfo {
            size_t LineNumber = 0;
            size_t Position = 0;
            ErrorCode Code;
            constexpr auto operator==(const TBasicInfo& other) const -> bool = default;
        } BasicInfo;
        using TAdditionalInfoBase = std::variant<
            std::string_view,
            TArrayIndexOutOfRangeAdditionalInfo
        >;
        struct TAdditionalInfo : public TAdditionalInfoBase {
            using TAdditionalInfoBase::variant;
        } AdditionalInfo;
        constexpr auto operator==(const TError& other) const -> bool = default;
    };
    constexpr auto Error(
        TLinePositionCounter lpCounter,
        ErrorCode code,
        std::string_view message = {}
    ) -> TError {
        return {
            .BasicInfo = {
                .LineNumber = lpCounter.LineNumber,
                .Position = lpCounter.Position,
                .Code = code,
            },
            .AdditionalInfo = message,
        };
    }
    constexpr auto Error(
        TLinePositionCounter lpCounter,
        ErrorCode code,
        TArrayIndexOutOfRangeAdditionalInfo info
    ) -> TError {
        return {
            .BasicInfo = {
                .LineNumber = lpCounter.LineNumber,
                .Position = lpCounter.Position,
                .Code = code,
            },
            .AdditionalInfo = info,
        };
    }
    template <class TOstream>
    constexpr auto operator<<(TOstream& out, const TError::TAdditionalInfo& info) -> TOstream& {
        std::visit([&out](auto&& val) { out << val; }, info);
        return out;
    }
    template <class TOstream>
    constexpr auto operator<<(TOstream& out, const TError& error) -> TOstream& {
        out << "Got " << ToStr(error.BasicInfo.Code);
        if (error.AdditionalInfo.index() != std::variant_npos) {
            if (error.BasicInfo.Code == ErrorCode::MappingKeyNotFound) {
                out << " (key \"" << error.AdditionalInfo << "\" doesn't exist in mapping)";
            } else {
                out << " (" << error.AdditionalInfo << ")";
            }
        }
        out << " at line " << error.BasicInfo.LineNumber
            << ", position " << error.BasicInfo.Position;
        return out;
    }

    template <ErrorCode code, size_t lineNumber, size_t position>
    struct PrintErrAtCompileTimeImpl {
        static_assert(static_cast<int>(code) == 0);
    };
    template <TError::TBasicInfo errInfo>
    struct PrintErrAtCompileTime : PrintErrAtCompileTimeImpl<
        errInfo.Code,
        errInfo.LineNumber,
        errInfo.Position
    > {};
}
