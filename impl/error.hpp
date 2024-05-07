#pragma once


#include "line_position_counter.hpp"

#include <algorithm>
#include <array>
#include <variant>


namespace NJsonParser::NError {
    // Contains codes for all errors that may occur
    // when using this json parser
    enum class ErrorCode : uint8_t {
        SyntaxError = 1,
        TypeError,
        MissingValueError,
        ArrayIndexOutOfRange,
        MappingKeyNotFound,
        EndIteratorDereferenceError,
        ResultOutOfRangeError,
    };
    // Maps `ErrorCode` values to string representations
    constexpr auto ToStr(ErrorCode code) noexcept -> std::string_view {
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
            case EndIteratorDereferenceError:
                return "\"dereference of an iterator pointing to an end of a container\" error";
            case ResultOutOfRangeError:
                return "\"provided int/double value is out of range of representable values "
                       "of int/double type used by this library\" error";
        }
        // To avoid compiler warning; should rather be `std::unreachable()` from c++23.
        // This project is written in c++20 on purpose, so, can't use it here.
        return "\"invalid error code\"";
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
        constexpr auto operator==(const TSelf& other) const noexcept -> bool = default;
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
    public:
        static constexpr size_t kMaxLenToSave = sizeof(TArrayIndexOutOfRangeAdditionalInfo) - 1;
        using TStorage = std::array<char, kMaxLenToSave + 1>;
    private:
        TStorage RequestedKey = {}; // initialized with zeros, hence, the invariant that
                                    // `RequestedKey` is null-terminated always holds
        using TSelf = TMappingKeyNotFoundAdditionalInfo;
    public:
        constexpr auto GetRequestedKey() const noexcept -> const char* {
            return &RequestedKey[0];
        }
        constexpr TMappingKeyNotFoundAdditionalInfo(std::string_view key) noexcept {
            // Do an actual copy instead of just storing the given `string_view`
            // so that `TError` objects don't depend on the lifetime of strings
            // containing json data from the parsing of which this error emerged
            std::copy_n(
                key.begin(),
                std::min(key.size(), kMaxLenToSave),
                RequestedKey.begin()
            );
        }
        constexpr auto operator==(const TSelf& other) const noexcept -> bool = default;
    };
    // The `operator<<` here is declared in this weird way so that it matches only second arguments that
    // have the exact type `TMappingKeyNotFoundAdditionalInfo` and not the ones convertible to this type
    template <class TOstream, class TInfo>
    requires std::same_as<TInfo, TMappingKeyNotFoundAdditionalInfo>
    constexpr auto operator<<(TOstream&& out, const TInfo& info) -> TOstream {
        out << "key \"" << info.GetRequestedKey() << "\" doesn't exist in mapping";
        return std::forward<TOstream>(out);
    }

    struct TError { 
        struct TBasicInfo {
            uint16_t LineNumber = 0;
            uint16_t Position = 0;
            ErrorCode Code;
            constexpr auto operator==(const TBasicInfo& other) const -> bool = default;
        } BasicInfo;
        // On x86-64 architectures `sizeof(std::string_view)`
        // == `sizeof(TArrayIndexOutOfRangeAdditionalInfo)`
        // == `sizeof(TMappingKeyNotFoundAdditionalInfo)` == 16 bytes
        // `TMappingKeyNotFoundAdditionalInfo` was designed to have
        // the same size as the other two on purpose, so that the
        // memory layout of this `std::variant` would be efficient
        using TAdditionalInfo = std::variant<
            std::string_view,
            TArrayIndexOutOfRangeAdditionalInfo,
            TMappingKeyNotFoundAdditionalInfo
        >;
        TAdditionalInfo AdditionalInfo;
        constexpr auto operator==(const TError& other) const noexcept -> bool = default;
    };
    // Okay, this is a bit ugly, but it's impossible to construct an empty
    // `std::variant` except for the "valueless by exception" case.
    // So, `error.AdditionalInfo` being an empty `std::string::view` is equivalent
    // to having no additional info.
    constexpr auto IsEmpty(const TError::TAdditionalInfo& info) -> bool {
        return std::holds_alternative<std::string_view>(info)
            && std::get<std::string_view>(info).empty();
    }
    // The `operator<<` here is declared in this weird way so that it matches only second arguments that
    // have the exact type `TError::TAdditionalInfo` and not the ones just convertible to this type
    template <class TOstream, class TAdditionalInfo>
    requires std::same_as<TAdditionalInfo, TError::TAdditionalInfo>
    constexpr auto operator<<(TOstream&& out, const TAdditionalInfo& info) -> TOstream {
        std::visit([&out](auto&& val) { out << val; }, info);
        return std::forward<TOstream>(out);
    }
    constexpr auto Error(
        TLinePositionCounter lpCounter,
        ErrorCode code,
        // initializes the `std::variant` with an empty `std::string_view`:
        TError::TAdditionalInfo additionalInfo = {}
    ) noexcept -> TError {
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
    constexpr auto operator<<(TOstream&& out, const TError& error) -> TOstream {
        out << ToStr(error.BasicInfo.Code);
        if (error.AdditionalInfo.index() != std::variant_npos
            && !IsEmpty(error.AdditionalInfo)
        ) out << " (" << error.AdditionalInfo << ")";
        out << " at line " << error.BasicInfo.LineNumber
            << ", position " << error.BasicInfo.Position;
        return std::forward<TOstream>(out);
    }

    // Two metafunctions for printing error messages at compile time
    // (at least the `BasicInfo` part)
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
