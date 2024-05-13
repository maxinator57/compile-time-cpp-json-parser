#pragma once


#include "error.hpp"
#include "expected.hpp"
#include "line_position_counter.hpp"

#include <iostream>
#include <string>


namespace NJsonParser::NUtils {
    constexpr auto kSpaces = std::string_view{" \t\n"};
    // Have to write an implementation of `IsSpace` by hand, because
    // in c++20 and even 23 `std::isspace` is not constexpr
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

    constexpr auto FindFirstOfWithZeroBracketBalance(
        std::string_view str,
        LinePositionCounter& lpCounter,
        std::invocable<char> auto&& predicate,
        std::string_view::size_type pos = 0
    ) -> Expected<std::string_view::size_type> { 
        if (str.size() <= pos) return std::string_view::npos;
        auto stack = std::string{};
        auto prevLpCounter = lpCounter;
        // indicates whether we are currently parsing a string literal
        bool insideStringLiteral = false;
        for (char ch : str.substr(pos)) {
            if (ch == '"') insideStringLiteral = !insideStringLiteral;
            if (!insideStringLiteral) {
                switch (ch) {
                    case '[':
                    case '{':
                        stack.push_back(ch); break;
                    case ']':
                        if (stack.empty() || stack.back() != '[') return MakeError(
                            lpCounter,
                            NError::ErrorCode::SyntaxError,
                            "brackets mismatch: encountered an excess ']'"
                        );
                        stack.pop_back(); break;
                    case '}':
                        if (stack.empty() || stack.back() != '{') return MakeError(
                            lpCounter,
                            NError::ErrorCode::SyntaxError,
                            "brackets mismatch: encountered an excess '}'"
                        );
                        stack.pop_back(); break;
                }
                const auto balance = stack.size();
                if (balance == 0 && predicate(ch)) return pos;
            }
            prevLpCounter = lpCounter;
            lpCounter.Process(ch);
            ++pos;
        }
        if (const auto balance = stack.size(); balance != 0) return MakeError(
            prevLpCounter,
            NError::ErrorCode::SyntaxError,
            "brackets mismatch: encountered some unmatched opening brackets"
        );
        if (insideStringLiteral) return MakeError(
            prevLpCounter,
            NError::ErrorCode::SyntaxError,
            "a double quote (\") is probably missing "
            "at the end of a string"
        );
        return std::string_view::npos;
    }

    constexpr auto FindNextElementStartPos(
        std::string_view str,
        LinePositionCounter& lpCounter,
        std::string_view::size_type pos = 0,
        char delimiter = ','
    ) -> Expected<std::string_view::size_type> {
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
        LinePositionCounter& lpCounter,
        std::string_view::size_type pos = 0,
        char delimiter = ','
    ) -> Expected<std::string_view::size_type> {
        return FindFirstOfWithZeroBracketBalance(
            str, lpCounter,
            [delimiter](char ch) {
                return ch == delimiter || IsSpace(ch);
            },
            pos
        );
    } 
}
