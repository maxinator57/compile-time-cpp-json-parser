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
    template <class Ostream>
    constexpr auto operator<<(Ostream& out, ErrorCode code) -> Ostream& {
        out << ToStr(code);
        return out;
    }

    struct ArrayIndexOutOfRangeAdditionalInfo {
        using Self = ArrayIndexOutOfRangeAdditionalInfo;
        size_t Index =  0;
        size_t ArrayLen = 0;
        constexpr auto operator==(const Self& other) const noexcept -> bool = default;
    }; 
    template <class Ostream>
    constexpr auto operator<<(
        Ostream& out,
        const ArrayIndexOutOfRangeAdditionalInfo& info
    ) -> Ostream& {
        out << "index " << info.Index << " is out of range for array of length " << info.ArrayLen;
        return out;
    }

    struct MappingKeyNotFoundAdditionalInfo {
        // Just a null-terminated array of bytes
    public:
        static constexpr size_t kMaxLenToSave = sizeof(ArrayIndexOutOfRangeAdditionalInfo) - 1;
        using Storage = std::array<char, kMaxLenToSave + 1>;
    private:
        Storage RequestedKey = {}; // initialized with zeros, hence, the invariant that
                                    // `RequestedKey` is null-terminated always holds
        using Self = MappingKeyNotFoundAdditionalInfo;
    public:
        constexpr auto GetRequestedKey() const noexcept -> const char* {
            return &RequestedKey[0];
        }
        constexpr MappingKeyNotFoundAdditionalInfo(std::string_view key) noexcept {
            // Do an actual copy instead of just storing the given `string_view`
            // so that `Error` objects don't depend on the lifetime of strings
            // containing json data from the parsing of which this error emerged
            std::copy_n(
                key.begin(),
                std::min(key.size(), kMaxLenToSave),
                RequestedKey.begin()
            );
        }
        constexpr auto operator==(const Self& other) const noexcept -> bool = default;
    };
    // The `operator<<` here is declared in this weird way so that it matches only second arguments that
    // have the exact type `MappingKeyNotFoundAdditionalInfo` and not the ones convertible to this type
    template <class Ostream, class TInfo>
    requires std::same_as<TInfo, MappingKeyNotFoundAdditionalInfo>
    constexpr auto operator<<(Ostream&& out, const TInfo& info) -> Ostream {
        out << "key \"" << info.GetRequestedKey() << "\" doesn't exist in mapping";
        return std::forward<Ostream>(out);
    }

    struct Error { 
        struct TBasicInfo {
            uint16_t LineNumber = 0;
            uint16_t Position = 0;
            ErrorCode Code;
            constexpr auto operator==(const TBasicInfo& other) const -> bool = default;
        } BasicInfo;
        // On x86-64 architectures `sizeof(std::string_view)`
        // == `sizeof(ArrayIndexOutOfRangeAdditionalInfo)`
        // == `sizeof(MappingKeyNotFoundAdditionalInfo)` == 16 bytes
        // `MappingKeyNotFoundAdditionalInfo` was designed to have
        // the same size as the other two on purpose, so that the
        // memory layout of this `std::variant` would be efficient
        using TAdditionalInfo = std::variant<
            std::string_view,
            ArrayIndexOutOfRangeAdditionalInfo,
            MappingKeyNotFoundAdditionalInfo
        >;
        TAdditionalInfo AdditionalInfo;
        constexpr auto operator==(const Error& other) const noexcept -> bool = default;
    };
    // Okay, this is a bit ugly, but it's impossible to construct an empty
    // `std::variant` except for the "valueless by exception" case.
    // So, `error.AdditionalInfo` being an empty `std::string::view` is equivalent
    // to having no additional info.
    constexpr auto IsEmpty(const Error::TAdditionalInfo& info) -> bool {
        return std::holds_alternative<std::string_view>(info)
            && std::get<std::string_view>(info).empty();
    }
    // The `operator<<` here is declared in this weird way so that it matches only second arguments that
    // have the exact type `Error::TAdditionalInfo` and not the ones just convertible to this type
    template <class Ostream, class TAdditionalInfo>
    requires std::same_as<TAdditionalInfo, Error::TAdditionalInfo>
    constexpr auto operator<<(Ostream&& out, const TAdditionalInfo& info) -> Ostream {
        std::visit([&out](auto&& val) { out << val; }, info);
        return std::forward<Ostream>(out);
    }
    constexpr auto MakeError(
        LinePositionCounter lpCounter,
        ErrorCode code,
        // initializes the `std::variant` with an empty `std::string_view`:
        Error::TAdditionalInfo additionalInfo = {}
    ) noexcept -> Error {
        return {
            .BasicInfo = {
                .LineNumber = lpCounter.LineNumber,
                .Position = lpCounter.Position,
                .Code = code,
            },
            .AdditionalInfo = additionalInfo,
        };
    } 
    template <class Ostream>
    constexpr auto operator<<(Ostream&& out, const Error& error) -> Ostream {
        out << ToStr(error.BasicInfo.Code);
        if (error.AdditionalInfo.index() != std::variant_npos
            && !IsEmpty(error.AdditionalInfo)
        ) out << " (" << error.AdditionalInfo << ")";
        out << " at line " << error.BasicInfo.LineNumber
            << ", position " << error.BasicInfo.Position;
        return std::forward<Ostream>(out);
    }

    // Two metafunctions for printing error messages at compile time
    // (at least the `BasicInfo` part)
    template <ErrorCode code, size_t lineNumber, size_t position>
    struct PrintErrAtCompileTimeImpl {
        static_assert(static_cast<int>(code) == 0);
    };
    template <Error::TBasicInfo errInfo>
    struct PrintErrAtCompileTime : PrintErrAtCompileTimeImpl<
        errInfo.Code,
        errInfo.LineNumber,
        errInfo.Position
    > {};
}
