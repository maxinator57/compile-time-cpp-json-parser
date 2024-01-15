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
    // A `std::stack<char>` replacement with static memory storage.
    // It is to be used only at compile-time.
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
                // throws because `TCompileTimeStack` is used only at compile time
                throw Error(
                    TLinePositionCounter{},
                    NError::ErrorCode::CompileTimeStackCapacityExceededError,
                    "try to set `JSON_PARSER_COMPILE_TIME_STACK_CAPACITY` "
                    "preprocessor option to a greater value when compiling "
                    "(by adding \"-D JSON_PARSER_COMPILE_TIME_STACK_CAPACITY=<value>\" compilation flag, "
                    "default value is 10)"
                );
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
        for (char ch : str.substr(pos)) {
            lpCounter.Process(ch);
            if (ch == '[' || ch == '{') {
                stack.push(ch);
            } else if (ch == ']') {
                if (stack.empty() || stack.top() != '[') {
                    lpCounter.StepBack();
                    return Error(
                        lpCounter,
                        NError::ErrorCode::SyntaxError,
                        "brackets mismatch: encountered an excess ']'"
                    );
                }
                stack.pop();
            } else if (ch == '}') {
                // TODO: distinguish between these two errors
                if (stack.empty() || stack.top() != '{') {
                    lpCounter.StepBack();
                    return Error(
                        lpCounter,
                        NError::ErrorCode::SyntaxError,
                        "brackets mismatch: encountered an excess '}'"
                    );
                }
                stack.pop();
            };
            const auto balance = stack.size();
            if (balance == 0 && predicate(ch)) {
                lpCounter.StepBack();
                return pos;
            }
            ++pos;
        }
        if (const auto balance = stack.size(); balance != 0) {
            lpCounter.StepBack();
            return Error(
                lpCounter,
                NError::ErrorCode::SyntaxError,
                "brackets mismatch: encountered some unmatched opening brackets"
            );
        }
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
