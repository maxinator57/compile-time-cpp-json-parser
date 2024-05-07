#pragma once


#include "error.hpp"
#include "expected.hpp"
#include "line_position_counter.hpp"

#include <string>


namespace NJsonParser::NUtils {
    constexpr auto kSpaces = std::string_view{" \t\n"};
    constexpr auto IsSpace(char ch) -> bool {
        return kSpaces.find(ch) != std::string_view::npos;
    }
    constexpr auto StripSpaces(std::string_view str) -> std::string_view {
        const auto start = str.find_first_not_of(kSpaces);
        if (start == std::string_view::npos) return {};
        const auto end = str.find_last_not_of(kSpaces);
        return str.substr(start, end - start + 1);
    }

    constexpr auto FindFirstOf(
        std::string_view str,
        auto&& lpCounter,
        std::invocable<char> auto&& predicate,
        std::string_view::size_type startPos = 0
    ) -> std::string_view::size_type {
        if (startPos == std::string_view::npos) return startPos;
        auto pos = startPos;
        for (char ch : str.substr(startPos)) {
            if (predicate(ch)) return pos;
            lpCounter.Process(ch);
            ++pos;
        }
        return std::string_view::npos;
    }

    template <std::invocable<char> Predicate>
    constexpr auto FindFirstOfWithZeroBracketBalance(
        std::string_view str,
        TLinePositionCounter& lpCounter,
        Predicate&& predicate,
        std::string_view::size_type pos = 0
    ) -> TExpected<std::string_view::size_type> { 
        if (str.size() <= pos) return std::string_view::npos;
        auto stack = std::string{};
        auto prevLpCounter = lpCounter;
        for (char ch : str.substr(pos)) {
            switch (ch) {
                case '[':
                case '{':
                    stack.push_back(ch); break;
                case ']':
                    if (stack.empty() || stack.back() != '[') return Error(
                        lpCounter,
                        NError::ErrorCode::SyntaxError,
                        "brackets mismatch: encounterlpCountered an excess ']'"
                    );
                    stack.pop_back(); break;
                case '}':
                    if (stack.empty() || stack.back() != '{') return Error(
                        lpCounter,
                        NError::ErrorCode::SyntaxError,
                        "brackets mismatch: encountered an excess '}'"
                    );
                    stack.pop_back(); break;
            } 
            const auto balance = stack.size();
            if (balance == 0 && predicate(ch)) return pos;
            prevLpCounter = lpCounter;
            lpCounter.Process(ch);
            ++pos;
        }
        if (const auto balance = stack.size(); balance != 0) return Error(
            prevLpCounter,
            NError::ErrorCode::SyntaxError,
            "brackets mismatch: encountered some unmatched opening brackets"
        );
        return std::string_view::npos;
    }

    constexpr auto FindNextElementStartPos(
        std::string_view str,
        TLinePositionCounter& lpCounter,
        std::string_view::size_type pos = 0,
        char delimiter = ','
    ) -> TExpected<std::string_view::size_type> {
        if (pos == std::string_view::npos) return pos;
        const auto result = FindFirstOfWithZeroBracketBalance(
            str, lpCounter,
            [delimiter](char ch) { return ch == delimiter; },
            pos
        ); if (result.HasError()) return result;
        pos = result.Value(); if (pos == std::string_view::npos) return pos;
        return FindFirstOf(
            str, lpCounter.Process(str[pos]),
            [](char ch) { return !IsSpace(ch); },
            pos + 1
        );
    }

    constexpr auto FindCurElementEndPos(
        std::string_view str,
        TLinePositionCounter& lpCounter,
        std::string_view::size_type pos = 0,
        char delimiter = ','
    ) -> TExpected<std::string_view::size_type> {
        return FindFirstOfWithZeroBracketBalance(
            str, lpCounter,
            [delimiter](char ch) {
                return ch == delimiter || NUtils::IsSpace(ch);
            },
            pos
        );
    } 
}
