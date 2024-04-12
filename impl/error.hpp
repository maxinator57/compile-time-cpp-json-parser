#pragma once

#include "line_position_counter.hpp"

#include <algorithm>
#include <array>
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

    struct TMappingKeyNotFoundAdditionalInfo {
        // Just a null-terminated array of bytes
    private:
        static constexpr size_t kMaxLenToSave = 15;
        std::array<char, kMaxLenToSave + 1> RequestedKey = {}; // initialized with zeros,
                                                               // hence the invariant that
                                                               // `RequestedKey` is null-terminated
                                                               // always holds
        using TSelf = TMappingKeyNotFoundAdditionalInfo;
    public:
        constexpr auto GetRequestedKey() const -> decltype((RequestedKey)) {
            return RequestedKey;
        }
        constexpr TMappingKeyNotFoundAdditionalInfo(std::string_view key) {
            // Do an actual copy instead of just storing the given string_view
            // so that `TError` objects don't depend on the lifetime of strings
            // containing json data from the parsing of which this error emerged
            std::copy_n(
                key.begin(),
                std::min(key.size(), kMaxLenToSave),
                RequestedKey.begin()
            );
        }
        constexpr auto operator==(const TSelf& other) const -> bool = default;
    };
    template <class TOstream>
    constexpr auto operator<<(
        TOstream& out,
        const TMappingKeyNotFoundAdditionalInfo& info
    ) -> TOstream& {
        out << "key \""
            << static_cast<const char*>(&info.GetRequestedKey()[0])
            << "\" doesn't exist in mapping";
        return out;
    }

    struct TError { 
        struct TBasicInfo {
            uint16_t LineNumber = 0;
            uint16_t Position = 0;
            ErrorCode Code;
            constexpr auto operator==(const TBasicInfo& other) const -> bool = default;
        } BasicInfo;
        using TAdditionalInfo = std::variant<
            std::string_view,
            TArrayIndexOutOfRangeAdditionalInfo,
            TMappingKeyNotFoundAdditionalInfo
        >;
        TAdditionalInfo AdditionalInfo;
        constexpr auto operator==(const TError& other) const -> bool = default;
    };

    // For `TInfo` == `std::string_view` or `const char*` must
    // be used only with objects with static storage duration
    template <class TInfo = std::string_view>
    requires std::convertible_to<TInfo, TError::TAdditionalInfo> 
    constexpr auto Error(
        TLinePositionCounter lpCounter,
        ErrorCode code,
        TInfo additionalInfo = {}
    ) -> TError {
        return {
            .BasicInfo = {
                .LineNumber = lpCounter.LineNumber,
                .Position = lpCounter.Position,
                .Code = code,
            },
            .AdditionalInfo = additionalInfo,
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
            out << " (" << error.AdditionalInfo << ")";
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
