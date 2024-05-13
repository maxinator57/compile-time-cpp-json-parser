#pragma once


#include "api.hpp"
#include "error.hpp"
#include "expected.hpp"
#include "utils.hpp"

#include <charconv>


namespace NJsonParser {
    constexpr JsonValue::JsonValue(std::string_view data, LinePositionCounter lpCounter) noexcept
        : DataHolderMixin(NUtils::StripSpaces(data), lpCounter) {}

    template <> constexpr auto JsonValue::As<Bool>() const noexcept -> Expected<Bool> {
        if (Data == "true") return true;
        if (Data == "false") return false;
        if (Data.empty()) return MakeError(
            LpCounter,
            NError::ErrorCode::MissingValueError
        );
        return MakeError(
            LpCounter,
            NError::ErrorCode::TypeError,
            "expected bool, got something else"
        );
    }

    template <> constexpr auto JsonValue::As<Int>() const noexcept -> Expected<Int> {
        if (Data.empty()) return MakeError(
            LpCounter,
            NError::ErrorCode::MissingValueError
        );
        Int result = 0;
        if (std::is_constant_evaluated()) {
            // At compile-time we have to parse an integer by hand
            constexpr auto isDigit = [](char ch) {
                return '0' <= ch && ch <= '9';
            };
            const auto isNegative = (Data.front() == '-');
            const auto sign = isNegative ? -1 : 1;
            for (const auto ch : Data.substr(isNegative ? 1 : 0)) {
                if (!isDigit(ch)) return MakeError(
                    LpCounter,
                    NError::ErrorCode::TypeError,
                    "expected int, got something else"
                );
                result = result * 10 + (ch - '0') * sign;
            }
        } else {
            // At run-time we use the fast library function `std::from_chars`
            const auto [_, ec] = std::from_chars(
                Data.data(), Data.data() + Data.size(), result, 10
            );
            if (ec == std::errc::invalid_argument) return MakeError(
                LpCounter,
                NError::ErrorCode::TypeError,
                "expected int, got something else"
            );
            if (ec == std::errc::result_out_of_range) return MakeError(
                LpCounter,
                NError::ErrorCode::ResultOutOfRangeError
            );
        }
        return result;
    }

    template <> constexpr auto JsonValue::As<Float>() const noexcept -> Expected<Float> {
        if (std::is_constant_evaluated()) {
            // At compile-time we have to parse a double by hand
            const auto dotPosition = Data.find_first_of('.');
            if (dotPosition == std::string_view::npos || dotPosition == Data.size() - 1) {
                auto intOrErr = JsonValue{Data.substr(0, dotPosition)}.As<Int>();
                if (intOrErr.HasError()) return MakeError(
                    LpCounter,
                    NError::ErrorCode::TypeError,
                    "expected double, got something else"
                );
                return static_cast<double>(intOrErr.Value());
            }

            // Parse the integral part:
            auto intPartOrErr = JsonValue{Data.substr(0, dotPosition)}.As<Int>();
            if (intPartOrErr.HasError()) return intPartOrErr.Error();
            const auto intPart = intPartOrErr.Value();

            // Parse the fractional part:
            const auto fracPartLen = std::min(
                Data.size() - dotPosition - 1,
                std::string_view::size_type{10}
            );
            auto fracPartOrErr = JsonValue{
                Data.substr(dotPosition + 1, fracPartLen)
            }.As<Int>();
            if (fracPartOrErr.HasError()) return fracPartOrErr.Error();
            const auto fracPart = fracPartOrErr.Value();

            constexpr auto pow = [](auto base, std::unsigned_integral auto exp) {
                auto result = static_cast<decltype(base)>(1);
                while (exp) {
                    if (exp & 1) result *= base;
                    exp >>= 1;
                    if (exp) base *= base;
                }
                return result;
            };

            const auto fracPartFloat =
                static_cast<double>(fracPart) / static_cast<double>(pow(uint64_t{10}, fracPartLen));
            return static_cast<double>(intPart) + (intPart < 0 ? -fracPartFloat : fracPartFloat);
        } else {
            // At run-time we use the fast library function `std::from_chars`
            Float result = 0;
            const auto [_, ec] = std::from_chars(
                Data.data(),
                Data.data() + Data.size(),
                result,
                std::chars_format::general
            );
            if (ec == std::errc::invalid_argument) return MakeError(
                LpCounter,
                NError::ErrorCode::TypeError,
                "expected double, got something else"
            );
            if (ec == std::errc::result_out_of_range) return MakeError(
                LpCounter,
                NError::ErrorCode::ResultOutOfRangeError
            );
            return result;
        }
    }

    template <> constexpr auto JsonValue::As<String>() const noexcept -> Expected<String> {
        if (Data.empty()) return MakeError(
            LpCounter,
            NError::ErrorCode::MissingValueError,
            "empty underlying data while expecting a string"
        );
        if (Data.size() == 1) {
            if (Data.front() == '"') return MakeError(
                LpCounter,
                NError::ErrorCode::SyntaxError,
                "a double quote (\") is probably missing "
                "at the end of a string"
            ); else return MakeError(
                LpCounter,
                NError::ErrorCode::TypeError,
                "expected string, got something else"
            );
        }
        if (Data.front() == '"' && Data.back() != '"') return MakeError(
            LpCounter.Copy().Process(Data),
            NError::ErrorCode::SyntaxError,
            "a double quote (\") is probably missing "
            "at the end of a string"
        );
        if (Data.front() != '"' && Data.back() == '"') return MakeError(
            LpCounter,
            NError::ErrorCode::SyntaxError,
            "a double quote (\") is probably missing "
            "at the start of a string"
        );
        if (Data.front() != '"' && Data.back() != '"') return MakeError(
            LpCounter,
            NError::ErrorCode::TypeError,
            "either both double quotes are missing or the "
            "underlying data does not represent a string"
        ); 
        return Data.substr(1, Data.size() - 2);
    }

    template <> constexpr auto JsonValue::As<Array>() const noexcept -> Expected<Array> {
        if (Data.empty()) return MakeError(
            LpCounter,
            NError::ErrorCode::MissingValueError,
            "empty underlying data while expecting an array"
        );
        if (Data.front() == '[' && Data.back() != ']') return MakeError(
            LpCounter.Copy().Process(Data.substr(0, Data.size() - 1)),
            NError::ErrorCode::SyntaxError,
            "a closing square bracket is probably missing "
            "at the end of an array"
        );
        if (Data.front() != '[' && Data.back() == ']') return MakeError(
            LpCounter,
            NError::ErrorCode::SyntaxError,
            "an opening square bracket is probably missing "
            "at the start of the array"
        );
        if (Data.front() != '[' && Data.back() != ']') return MakeError(
            LpCounter,
            NError::ErrorCode::TypeError,
            "either both square brackets are missing or the "
            "underlying data does not represent an array"
        );
        return Array{
            Data.substr(1, Data.size() - 2),
            LpCounter
        };
    }

    constexpr auto JsonValue::operator[](size_t idx) const noexcept -> Expected<JsonValue> {
        return As<Array>()[idx];
    }

    template <> constexpr auto JsonValue::As<Mapping>() const noexcept -> Expected<Mapping> {
        if (Data.empty()) return MakeError(
            LpCounter,
            NError::ErrorCode::MissingValueError,
            "empty underlying data while expecting a mapping"
        );
        if (Data.front() == '{' && Data.back() != '}') return MakeError(
            LpCounter.Copy().Process(Data.substr(0, Data.size() - 1)),
            NError::ErrorCode::SyntaxError,
            "a closing curly brace ('}') is probably missing "
            "at the end of a mapping"
        );
        if (Data.front() != '{' && Data.back() == '}') return MakeError(
            LpCounter,
            NError::ErrorCode::SyntaxError,
            "an opening curly brace ('{') is probably missing "
            "at the start of a mapping"
        );
        if (Data.front() != '{' && Data.back() != '}') return MakeError(
            LpCounter,
            NError::ErrorCode::TypeError,
            "either both curly braces ('{' and '}') are missing "
            "or the underlying data does not represent a mapping"
        );
        return Mapping{
            Data.substr(1, Data.size() - 2),
            LpCounter,
        };
    }

    constexpr auto JsonValue::operator[](std::string_view key) const noexcept -> Expected<JsonValue> {
        return As<Mapping>()[key];
    }

    // The following boilerplate is needed for syntactically nice
    // monadic operations support:
    template <> constexpr auto Expected<JsonValue>::As<Bool>() const -> Expected<Bool> {
        return HasValue() ? Value().As<Bool>() : Error();
    }
    template <> constexpr auto Expected<JsonValue>::As<Int>() const -> Expected<Int> {
        return HasValue() ? Value().As<Int>() : Error();
    }
    template <> constexpr auto Expected<JsonValue>::As<Float>() const -> Expected<Float> {
        return HasValue() ? Value().As<Float>() : Error();
    }
    template <> constexpr auto Expected<JsonValue>::As<String>() const -> Expected<String> {
        return HasValue() ? Value().As<String>() : Error();
    }
    template <> constexpr auto Expected<JsonValue>::As<Array>() const -> Expected<Array> {
        return HasValue() ? Value().As<Array>() : Error();
    }
    constexpr auto Expected<JsonValue>::operator[](size_t idx) const -> Expected<JsonValue> {
        return As<Array>()[idx];
    }
    template <> constexpr auto Expected<JsonValue>::As<Mapping>() const -> Expected<Mapping> {
        return HasValue() ? Value().As<Mapping>() : Error();
    }
    constexpr auto Expected<JsonValue>::operator[](std::string_view key) const -> Expected<JsonValue> {
        return As<Mapping>()[key];
    }
} // namespace NJsonParser
