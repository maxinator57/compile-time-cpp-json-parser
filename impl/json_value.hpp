#pragma once


#include "api.hpp"
#include "error.hpp"
#include "expected.hpp"
#include "utils.hpp"
#include <charconv>
#include <system_error>
#include <type_traits>


namespace NJsonParser {
    constexpr TJsonValue::TJsonValue(std::string_view data, const TLinePositionCounter& lpCounter)
        : TDataHolderMixin(NUtils::StripSpaces(data), lpCounter) {}

    constexpr auto TJsonValue::AsInt() const -> TExpected<Int> {
        if (Data.empty()) return Error(
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
            for (auto ch : Data.substr(isNegative ? 1 : 0)) {
                if (!isDigit(ch)) return Error(
                    LpCounter,
                    NError::ErrorCode::TypeError,
                    "expected int, got something else"
                );
                result = result * 10 + (ch - '0') * sign;
            }
        } else {
            // At run-time we use the fast library function `std::from_chars`
            const auto fromCharsResult = std::from_chars(
                Data.data(), Data.data() + Data.size(), result, 10
            );
            if (fromCharsResult.ec == std::errc::invalid_argument) return Error(
                LpCounter,
                NError::ErrorCode::TypeError,
                "expected int, got something else"
            );
            if (fromCharsResult.ec == std::errc::result_out_of_range) return Error(
                LpCounter,
                NError::ErrorCode::ResultOutOfRangeError
            );
        }
        return result;
    }

    constexpr auto TJsonValue::AsDouble() const -> TExpected<Double> {
        if (std::is_constant_evaluated()) {
            // At compile-time we have to parse a double by hand
            const auto dotPosition = Data.find_first_of('.');
            if (dotPosition == std::string_view::npos || dotPosition == Data.size() - 1) {
                auto intOrErr = TJsonValue{Data.substr(0, dotPosition)}.AsInt();
                if (intOrErr.HasError()) return Error(
                    LpCounter,
                    NError::ErrorCode::TypeError,
                    "expected double, got something else"
                );
                return static_cast<double>(intOrErr.Value());
            }

            // Parse the integral part:
            auto intPartOrErr = TJsonValue{Data.substr(0, dotPosition)}.AsInt();
            if (intPartOrErr.HasError()) return std::move(intPartOrErr.Error());
            const auto intPart = intPartOrErr.Value();

            // Parse the fractional part:
            const auto fracPartLen = std::min(
                Data.size() - dotPosition - 1,
                std::string_view::size_type{10}
            );
            auto fracPartOrErr = TJsonValue{
                Data.substr(dotPosition + 1, fracPartLen)
            }.AsInt();
            if (fracPartOrErr.HasError()) return std::move(fracPartOrErr.Error());
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

            const auto fracPartDouble =
                static_cast<double>(fracPart) / static_cast<double>(pow(uint64_t{10}, fracPartLen));
            return static_cast<double>(intPart) + (intPart < 0 ? -fracPartDouble : fracPartDouble);
        } else {
            // At run-time we use the fast library function `std::from_chars`
            Double result = 0;
            const auto [_, ec] = std::from_chars(
                Data.data(),
                Data.data() + Data.size(),
                result,
                std::chars_format::general
            );
            if (ec == std::errc::invalid_argument) return Error(
                LpCounter,
                NError::ErrorCode::TypeError,
                "expected double, got something else"
            );
            if (ec == std::errc::result_out_of_range) return Error(
                LpCounter,
                NError::ErrorCode::ResultOutOfRangeError
            );
            return result;
        }
    }

    constexpr auto TJsonValue::AsString() const -> TExpected<String> {
        if (Data.empty()) return Error(
            LpCounter,
            NError::ErrorCode::MissingValueError,
            "empty underlying data while expecting a string"
        );
        if (Data.size() == 1) {
            if (Data.front() == '"') return Error(
                LpCounter,
                NError::ErrorCode::SyntaxError,
                "a double quote (\") is probably missing "
                "at the end of a string"
            ); else return Error(
                LpCounter,
                NError::ErrorCode::TypeError,
                "expected string, got something else"
            );
        }
        if (Data.front() == '"' && Data.back() != '"') return Error(
            LpCounter.Copy().Process(Data),
            NError::ErrorCode::SyntaxError,
            "a double quote (\") is probably missing "
            "at the end of a string"
        );
        if (Data.front() != '"' && Data.back() == '"') return Error(
            LpCounter,
            NError::ErrorCode::SyntaxError,
            "a double quote (\") is probably missing "
            "at the start of a string"
        );
        if (Data.front() != '"' && Data.back() != '"') return Error(
            LpCounter,
            NError::ErrorCode::TypeError,
            "either both double quotes are missing or the "
            "underlying data does not represent a string"
        ); 
        return Data.substr(1, Data.size() - 2);
    }

    constexpr auto TJsonValue::AsArray() const -> TExpected<TJsonArray> {
        if (Data.empty()) return Error(
            LpCounter,
            NError::ErrorCode::MissingValueError,
            "empty underlying data while expecting an array"
        );
        if (Data.front() == '[' && Data.back() != ']') return Error(
            LpCounter.Copy().Process(Data.substr(0, Data.size() - 1)),
            NError::ErrorCode::SyntaxError,
            "a closing square bracket is probably missing "
            "at the end of an array"
        );
        if (Data.front() != '[' && Data.back() == ']') return Error(
            LpCounter,
            NError::ErrorCode::SyntaxError,
            "an opening square bracket is probably missing "
            "at the start of the array"
        );
        if (Data.front() != '[' && Data.back() != ']') return Error(
            LpCounter,
            NError::ErrorCode::TypeError,
            "either both square brackets are missing or the "
            "underlying data does not represent an array"
        );
        return TJsonArray{
            Data.substr(1, Data.size() - 2),
            LpCounter
        };
    }

    constexpr auto TJsonValue::operator[](size_t idx) const -> TExpected<TJsonValue> {
        return AsArray()[idx];
    }

    constexpr auto TJsonValue::AsMapping() const -> TExpected<TJsonMapping> {
        if (Data.empty()) return Error(
            LpCounter,
            NError::ErrorCode::MissingValueError,
            "empty underlying data while expecting a mapping"
        );
        if (Data.front() == '{' && Data.back() != '}') return Error(
            LpCounter.Copy().Process(Data.substr(0, Data.size() - 1)),
            NError::ErrorCode::SyntaxError,
            "a closing curly brace ('}') is probably missing "
            "at the end of a mapping"
        );
        if (Data.front() != '{' && Data.back() == '}') return Error(
            LpCounter,
            NError::ErrorCode::SyntaxError,
            "an opening curly brace ('{') is probably missing "
            "at the start of a mapping"
        );
        if (Data.front() != '{' && Data.back() != '}') return Error(
            LpCounter,
            NError::ErrorCode::TypeError,
            "either both curly braces ('{' and '}') are missing "
            "or the underlying data does not represent a mapping"
        );
        return TJsonMapping{
            Data.substr(1, Data.size() - 2),
            LpCounter.Copy().Process(Data[0])
        };
    }

    constexpr auto TJsonValue::operator[](std::string_view key) const -> TExpected<TJsonValue> {
        return AsMapping()[key];
    }

    // The following boilerplate is needed for syntactically nice
    // monadic operations support:

    constexpr auto TExpected<TJsonValue>::AsInt() const -> TExpected<Int> {
        return HasValue() ? Value().AsInt() : Error();
    }
    constexpr auto TExpected<TJsonValue>::AsDouble() const -> TExpected<Double> {
        return HasValue() ? Value().AsDouble() : Error();
    }
    constexpr auto TExpected<TJsonValue>::AsString() const -> TExpected<String> {
        return HasValue() ? Value().AsString() : Error();
    }

    constexpr auto TExpected<TJsonValue>::AsArray() const -> TExpected<TJsonArray> {
        return HasValue() ? Value().AsArray() : Error();
    }
    constexpr auto TExpected<TJsonValue>::operator[](size_t idx) const -> TExpected<TJsonValue> {
        return AsArray()[idx];
    }

    constexpr auto TExpected<TJsonValue>::AsMapping() const -> TExpected<TJsonMapping> {
        return HasValue() ? Value().AsMapping() : Error();
    }
    constexpr auto TExpected<TJsonValue>::operator[](std::string_view key) const -> TExpected<TJsonValue> {
        return AsMapping()[key];
    }
} // namespace NJsonParser
