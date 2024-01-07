#pragma once


#include "api.hpp"
#include "array.hpp"
#include "mapping.hpp"

#include <concepts>
#include <optional>
#include <string_view>


namespace NCompileTimeJsonParser {
    constexpr TJsonValue::TJsonValue(std::string_view data) : Data(data) {}

    constexpr auto TJsonValue::GetData() const -> std::string_view {
        return Data;
    }

    constexpr auto TJsonValue::AsInt64() const -> std::optional<int64_t> {
        if (Data.empty()) return std::nullopt;

        constexpr auto isDigit = [](char ch) {
            return '0' <= ch && ch <= '9';
        };

        auto isNegative = (Data[0] == '-');
        auto result = int64_t{0};
        for (auto ch : Data.substr(isNegative ? 1 : 0)) {
            if (!isDigit(ch)) return std::nullopt;
            result = result * 10 + ch - '0';
        }
        return isNegative ? -result : result;
    }


    constexpr auto TJsonValue::AsDouble() const -> std::optional<double> {
        const auto dotPosition = Data.find_first_of('.');
        if (dotPosition == std::string_view::npos || dotPosition == Data.size() - 1) {
            auto intResult = TJsonValue{Data.substr(0, dotPosition)}.AsInt64();
            if (intResult.has_value()) return static_cast<double>(*intResult);
            return std::nullopt;
        }

        const auto [intPart, intPartOk] = [&]() -> std::pair<int64_t, bool> {
            auto maybeIntPart = TJsonValue{Data.substr(0, dotPosition)}.AsInt64();
            if (maybeIntPart.has_value()) return {*maybeIntPart, true};
            return {0, false};
        }(); if (!intPartOk) return std::nullopt;

        const auto fracPartLen = std::min(
            Data.size() - dotPosition - 1,
            std::string_view::size_type{10}
        );

        const auto [fracPart, fracPartOk] = [&]() -> std::pair<int64_t, bool> {
            auto maybeFracPart = TJsonValue{Data.substr(
                dotPosition + 1,
                fracPartLen
            )}.AsInt64();
            if (maybeFracPart.has_value()) return {*maybeFracPart, true};
            return {0, false};
        }(); if (!fracPartOk || fracPart < 0) return std::nullopt;

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


    constexpr auto TJsonValue::AsString() const -> std::optional<std::string_view> {
        if (Data.size() < 2 || Data[0] != '"' || Data.back() != '"') {
            return std::nullopt;
        }
        return Data.substr(1, Data.size() - 2);
    }


    constexpr auto TJsonValue::AsArray() const -> TMonadicOptional<TJsonArray> {
        if (Data.empty() || Data[0] != '[' || Data.back() != ']') {
            return std::nullopt;
        }
        return TJsonArray(Data.substr(1, Data.size() - 2));
    }

    constexpr auto TJsonValue::AsMapping() const -> TMonadicOptional<TJsonMapping> {
        if (Data.empty() || Data[0] != '{' || Data.back() != '}') {
            return std::nullopt;
        }
        return TJsonMapping(Data.substr(1, Data.size() - 2));
    }

    constexpr auto TMonadicOptional<TJsonValue>::AsInt64() const -> std::optional<int64_t> {
        return has_value() ? value().AsInt64() : std::nullopt;
    }

    constexpr auto TMonadicOptional<TJsonValue>::AsDouble() const -> std::optional<double> {
        return has_value() ? value().AsDouble() : std::nullopt;
    }

    constexpr auto TMonadicOptional<TJsonValue>::AsString() const -> std::optional<std::string_view> {
        return has_value() ? value().AsString() : std::nullopt;
    }

    constexpr auto TMonadicOptional<TJsonValue>::AsArray() const -> TMonadicOptional<TJsonArray> {
        return has_value() ? value().AsArray() : std::nullopt;
    }

    constexpr auto TMonadicOptional<TJsonValue>::AsMapping() const -> TMonadicOptional<TJsonMapping> {
        return has_value() ? value().AsMapping() : std::nullopt;
    }
} // namespace NCompileTimeJsonParser
