#pragma once


#include "api.hpp"
#include "error.hpp"
#include "expected.hpp"
#include "line_position_counter.hpp"

#include <array>
#include <stack>
#include <string_view>
#include <type_traits>


namespace NCompileTimeJsonParser::NUtils { 
    struct TCompileTimeStack {
    private:
        static constexpr auto kCapacity = []() -> size_t {
            #ifdef JSON_PARSER_COMPILE_TIME_STACK_CAPACITY
                return JSON_PARSER_COMPILE_TIME_STACK_CAPACITY;
            #else
                return 10; // default value
            #endif
        }();
        std::array<char, kCapacity> Data;
        size_t Size = 0;
    public:
        constexpr auto top() const -> char { return Data[Size - 1]; }  
        constexpr auto empty() const -> bool { return size() == 0; }
        constexpr auto size() const -> size_t { return Size; }
        constexpr auto push(char ch) -> void {
            if (Size == kCapacity) {
                // TODO: handle this error in some sensible way
                if (std::is_constant_evaluated()) throw "aaaaa"; 
            }
            Data[Size] = ch;
            ++Size;
        }
        constexpr auto pop() -> void { --Size; }
    }; 

    constexpr auto kSpaces = std::string_view{" \t\n"};
    constexpr auto IsSpace(char ch) -> bool {
        return kSpaces.find(ch) != std::string_view::npos;
    }
    constexpr auto StripSpaces(std::string_view str) -> std::string_view {
        auto start = str.find_first_not_of(kSpaces);
        if (start == std::string_view::npos) return {};
        auto end = str.find_last_not_of(kSpaces);
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
        auto prevLpCounter = lpCounter.Copy();
        for (char ch : str.substr(startPos)) {
            prevLpCounter = lpCounter;
            lpCounter.Process(ch);
            if (predicate(ch)) {
                lpCounter = prevLpCounter;
                return pos;
            }
            ++pos;
        }
        lpCounter = prevLpCounter;
        return std::string_view::npos;
    }

    template <class Stack>
    requires std::same_as<Stack, TCompileTimeStack>
          || std::same_as<Stack, std::stack<char>>  
    constexpr auto FindFirstOfWithZeroBracketBalanceImpl( 
        Stack&& stack,
        std::string_view str,
        TLinePositionCounter& lpCounter,
        std::invocable<char> auto&& predicate,
        std::string_view::size_type pos = 0
    ) -> TExpected<std::string_view::size_type> {
        if (str.size() <= pos) return std::string_view::npos;
        auto prevLpCounter = lpCounter;
        for (char ch : str.substr(pos)) {
            prevLpCounter = lpCounter;
            lpCounter.Process(ch);
            if (ch == '[' || ch == '{') {
                stack.push(ch);
            } else if (ch == ']') {
                if (stack.top() != '[') {
                    lpCounter = prevLpCounter;
                    return Error(
                        lpCounter,
                        NError::ErrorCode::SyntaxError
                    );
                }
                stack.pop();
            } else if (ch == '}') {
                // TODO: distinguish between these two errors
                if (stack.top() != '{') {
                    lpCounter = prevLpCounter;
                    return Error(
                        lpCounter,
                        NError::ErrorCode::SyntaxError
                    );
                }
                stack.pop();
            };
            const auto balance = stack.size();
            if (balance == 0 && predicate(ch)) {
                lpCounter = prevLpCounter;
                return pos;
            }
            ++pos;
        }
        lpCounter = prevLpCounter;
        if (const auto balance = stack.size(); balance != 0) return Error(
            lpCounter,
            NError::ErrorCode::SyntaxError
        );
        return std::string_view::npos;
    }

    template <std::invocable<char> Predicate>
    constexpr auto FindFirstOfWithZeroBracketBalance(
        std::string_view str,
        TLinePositionCounter& lpCounter,
        Predicate&& predicate,
        std::string_view::size_type pos = 0
    ) {
        // use TCompileTimeStack at compile time
        if (std::is_constant_evaluated()) return FindFirstOfWithZeroBracketBalanceImpl(
            TCompileTimeStack{}, str, lpCounter, std::forward<Predicate>(predicate), pos
        );
        // and std::stack<char> at runtime
        else return FindFirstOfWithZeroBracketBalanceImpl(
            std::stack<char>{}, str, lpCounter, std::forward<Predicate>(predicate), pos
        );
    }

    constexpr auto FindNextElementStartPos(
        std::string_view str,
        TLinePositionCounter& lpCounter,
        std::string_view::size_type pos = 0,
        char delimiter = ','
    ) -> TExpected<std::string_view::size_type> {
        if (pos == std::string_view::npos) return pos;
        {
            const auto result = FindFirstOfWithZeroBracketBalance(
                str,
                lpCounter,
                [=](char ch) { return ch == delimiter; },
                pos
            );
            if (result.HasError()) return result;
            pos = result.Value();
        }
        if (pos == std::string_view::npos) return pos;
        lpCounter.Process(str[pos]);
        return FindFirstOf(
            str,
            lpCounter,
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
            str,
            lpCounter,
            [=](char ch) {
                return ch == delimiter || NUtils::IsSpace(ch);
            },
            pos
        );
    } 
}
