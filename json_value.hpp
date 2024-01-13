#pragma once


#include "api.hpp"
#include "array.hpp"
#include "error.hpp"
#include "expected.hpp"
#include "line_position_counter.hpp"
#include "mapping.hpp"

#include <concepts>
#include <string_view>


namespace NCompileTimeJsonParser {
    constexpr TJsonValue::TJsonValue(
        std::string_view data,
        TLinePositionCounter lpCounter
    ) : Data(data), LpCounter(lpCounter) {}

    constexpr auto TJsonValue::GetData() const -> std::string_view {
        return Data;
    }
    constexpr auto TJsonValue::GetLpCounter() const -> TLinePositionCounter {
        return LpCounter;
    }

    constexpr auto TJsonValue::AsInt() const -> TExpected<int64_t> {
        if (Data.empty()) return Error(
            LpCounter,
            NError::ErrorCode::MissingValueError
        );
        constexpr auto isDigit = [](char ch) {
            return '0' <= ch && ch <= '9';
        };
        const auto isNegative = (Data[0] == '-');
        auto result = int64_t{0};
        for (auto ch : Data.substr(isNegative ? 1 : 0)) {
            if (!isDigit(ch)) return Error(LpCounter, NError::ErrorCode::TypeError);
            result = result * 10 + ch - '0';
        }
        return isNegative ? -result : result;
    }

    constexpr auto TJsonValue::AsDouble() const -> TExpected<double> {
        const auto dotPosition = Data.find_first_of('.');
        if (dotPosition == std::string_view::npos || dotPosition == Data.size() - 1) {
            auto intOrErr = TJsonValue{Data.substr(0, dotPosition)}.AsInt();
            if (intOrErr.HasError()) return NError::TError{
                .LineNumber = LpCounter.LineNumber,
                .Position = LpCounter.Position,
                .Code = NError::ErrorCode::TypeError,
            };
            return static_cast<double>(intOrErr.Value());
        }

        auto intPartOrErr = TJsonValue{Data.substr(0, dotPosition)}.AsInt();
        if (intPartOrErr.HasError()) return std::move(intPartOrErr.Error());
        const auto intPart = intPartOrErr.Value();

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

        const auto fracPartDouble = static_cast<double>(fracPart) / pow(uint64_t{10}, fracPartLen);
        return static_cast<double>(intPart) + (intPart < 0 ? -fracPartDouble : fracPartDouble);
    }

    constexpr auto TJsonValue::AsString() const -> TExpected<String> {
        if (Data.empty()) return Error(
            LpCounter,
            NError::ErrorCode::MissingValueError
        );
        if (Data.size() == 1) return Error(
            LpCounter,
            Data[0] == '"'
                ? NError::ErrorCode::SyntaxError
                : NError::ErrorCode::TypeError
        );
        if (Data[0] == '"' && Data.back() != '"') return Error(
            LpCounter.Copy().Process(Data),
            NError::ErrorCode::SyntaxError
        );
        if (Data[0] != '"' && Data.back() == '"') return Error(
            LpCounter,
            NError::ErrorCode::SyntaxError
        );
        if (Data[0] != '"' && Data.back() != '"') return Error(
            LpCounter,
            NError::ErrorCode::TypeError
        ); 
        return Data.substr(1, Data.size() - 2);
    }

    constexpr auto TJsonValue::AsArray() const -> TExpected<TJsonArray> {
        if (Data.empty()) return Error(
            LpCounter,
            NError::ErrorCode::MissingValueError
        );
        if (Data[0] == '[' && Data.back() != ']') return Error(
            LpCounter.Copy().Process(Data),
            NError::ErrorCode::SyntaxError
        );
        if (Data[0] != '[' && Data.back() == ']') return Error(
            LpCounter,
            NError::ErrorCode::SyntaxError
        );
        if (Data[0] != '[' && Data.back() != ']') return Error(
            LpCounter,
            NError::ErrorCode::TypeError
        );
        return TJsonArray{
            Data.substr(1, Data.size() - 2),
            LpCounter.Copy().Process(Data[0])
        };
    }

    constexpr auto TJsonValue::AsMapping() const -> TExpected<TJsonMapping> {
        if (Data.empty()) return Error(
            LpCounter,
            NError::ErrorCode::MissingValueError
        );
        if (Data[0] == '{' && Data.back() != '}') return Error(
            LpCounter.Copy().Process(Data),
            NError::ErrorCode::SyntaxError
        );
        if (Data[0] != '{' && Data.back() == '}') return Error(
            LpCounter,
            NError::ErrorCode::SyntaxError
        );
        if (Data[0] != '{' && Data.back() != '}') return Error(
            LpCounter,
            NError::ErrorCode::TypeError
        );
        return TJsonMapping{
            Data.substr(1, Data.size() - 2),
            LpCounter.Copy().Process(Data[0])
        };
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
    constexpr auto TExpected<TJsonValue>::AsMapping() const -> TExpected<TJsonMapping> {
        return HasValue() ? Value().AsMapping() : Error();
    }
} // namespace NCompileTimeJsonParser
