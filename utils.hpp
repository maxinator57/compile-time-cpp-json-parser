#pragma once


#include "error.hpp"

#include <array>
#include <sstream>
#include <string_view>


namespace NCompileTimeJsonParser::NUtils {
    template <size_t capacity>
    struct TCompileTimeStack {
    private:
        std::array<char, capacity> Data;
        size_t Size = 0;
    public:
        constexpr auto top() const -> char { return Data[Size - 1]; }  
        constexpr auto empty() const -> bool { return size() == 0; }
        constexpr auto size() const -> size_t { return Size; }
        constexpr auto push(char ch) -> void {
            if (Size == capacity) throw std::runtime_error{(
                std::stringstream{}
                << "Compile-time stack capacity of " << capacity
                << " exceeded"
            ).str()};
            Data[Size] = ch;
            ++Size;
        }
        constexpr auto pop() -> void {
            if (Size == 0) throw std::runtime_error{"Pop from an empty compile-time stack"};
            --Size;
        }
    };

    constexpr auto kSpaces = std::string_view{" \t\n"};
    constexpr auto IsSpace(char ch) -> bool {
        return kSpaces.find(ch) != std::string_view::npos;
    }

    constexpr auto FindFirstOf(
        std::string_view str,
        auto&& predicate,
        std::string_view::size_type startPos = 0
    ) -> std::string_view::size_type {
        auto pos = startPos;
        for (char ch : str.substr(startPos)) {
            if (predicate(ch)) return pos;
            ++pos;
        }
        return std::string_view::npos;
    } 

    constexpr auto FindFirstOfWithZeroBracketBalance(
        std::string_view str, 
        auto predicate,
        std::string_view::size_type pos = 0
    ) -> std::string_view::size_type {
        using namespace NError;
        auto stack = TCompileTimeStack<10>{};
        for (std::string_view::size_type i = pos; i < str.size(); ++i) {
            const auto ch = str[i]; 
            if (ch == '[' || ch == '{') {
                stack.push(str[i]);
            } else if (ch == ']') {
                if (stack.top() != '[') throw TParsingError{
                    "Opening and closing brackets mismatch: expected [, got {"
                };
                stack.pop();
            } else if (ch == '}') {
                if (stack.top() != '{') throw TParsingError{
                    "Opening and closing brackets mismatch: expected {, got ["
                };
                stack.pop();
            };
            const auto balance = stack.size();
            if (balance == 0 && predicate(ch)) return i;
        }
        if (const auto balance = stack.size(); balance != 0) throw TParsingError{
            "Opening and closing brackets mismatch: some opening brackets are unmatched"
        };
        return std::string_view::npos;
    }

    constexpr auto FindNextElementStartPos(
        std::string_view str,
        std::string_view::size_type pos = 0,
        char delimiter = ','
    ) -> std::string_view::size_type {
        pos = FindFirstOfWithZeroBracketBalance(
            str,
            [=](char ch) { return ch == delimiter; },
            pos
        );
        if (pos == std::string_view::npos) return pos;
        pos = FindFirstOf(
            str,
            [](char ch) { return !IsSpace(ch); },
            pos + 1
        );
        return pos;
    }

    constexpr auto FindCurElementEndPos(
        std::string_view str,
        std::string_view::size_type pos = 0,
        char delimiter = ','
    ) -> std::string_view::size_type {
        return FindFirstOfWithZeroBracketBalance(
            str,
            [=](char ch) {
                return ch == delimiter || NUtils::IsSpace(ch);
            },
            pos
        );
    } 
}
