#pragma once

#include "line_position_counter.hpp"

#include <algorithm>
#include <array>
#include <string_view>
#include <variant>


namespace NCompileTimeJsonParser::NError {
    // Contains codes for all errors that may occur
    // when using this json parser
    enum class ErrorCode : uint8_t {
        SyntaxError = 1,
        TypeError, 
        MissingValueError,
        ArrayIndexOutOfRange,
        MappingKeyNotFound,
        IteratorDereferenceError,
    };
    // Maps `ErrorCode` values to string representations
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
        }
        return "\"invalid error code\""; // to avoid compiler warning; should rather 
                                         // be `std::unreachable()` from c++23 (this
                                         // project is written in c++20 on purpose,
                                         // so, can't use it here)
    }
    template <class TOstream>
    constexpr auto operator<<(TOstream& out, ErrorCode code) -> TOstream& {
        out << ToStr(code);
        return out;
    }

    struct TArrayIndexOutOfRangeAdditionalInfo {
        using TSelf = TArrayIndexOutOfRangeAdditionalInfo;
        size_t Index =  0;
        size_t ArrayLen = 0;
        constexpr auto operator==(const TSelf& other) const -> bool = default;
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
    template <class TOstream, class TInfo>                          // the `operator<<` here is declared in this weird way so that it matches only second arguments that
    requires std::same_as<TInfo, TMappingKeyNotFoundAdditionalInfo> // have the exact type `TMappingKeyNotFoundAdditionalInfo` and not the ones convertible to this type
    constexpr auto operator<<(TOstream&& out, const TInfo& info) -> TOstream {
        out << "key \""
            << static_cast<const char*>(&info.GetRequestedKey()[0])
            << "\" doesn't exist in mapping";
        return std::forward<TOstream>(out);
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

    constexpr auto Error(
        TLinePositionCounter lpCounter,
        ErrorCode code,
        TError::TAdditionalInfo additionalInfo = std::string_view{}
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
    
    template <class TOstream, class TAdditionalInfo>                // the `operator<<` here is declared in this weird way so that it matches only second arguments
    requires std::same_as<TAdditionalInfo, TError::TAdditionalInfo> // that have the exact type `TError::TAdditionalInfo` and not the ones convertible to this type
    constexpr auto operator<<(TOstream&& out, const TAdditionalInfo& info) -> TOstream {
        std::visit([&out](auto&& val) { out << val; }, info);
        return std::forward<TOstream>(out);
    }
    template <class TOstream>
    constexpr auto operator<<(TOstream&& out, const TError& error) -> TOstream {
        out << ToStr(error.BasicInfo.Code);
        if (error.AdditionalInfo.index() != std::variant_npos
            && !(std::holds_alternative<std::string_view>(error.AdditionalInfo)
              && std::get<std::string_view>(error.AdditionalInfo).empty()
            )
        ) {
            out << " (" << error.AdditionalInfo << ")";
        }
        out << " at line " << error.BasicInfo.LineNumber
            << ", position " << error.BasicInfo.Position;
        return std::forward<TOstream>(out);
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
