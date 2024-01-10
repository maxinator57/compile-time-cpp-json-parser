#pragma once

#include "error.hpp"
#include "line_position_counter.hpp"

#include <string_view>


namespace NCompileTimeJsonParser {  
    class TJsonValue;

    template <class T> struct TExpected;

    using Int = int64_t;
    using Double = double;
    using String = std::string_view;

    class TJsonArray {
    private:
        std::string_view Data;
        TLinePositionCounter LpCounter;
        constexpr TJsonArray(std::string_view, TLinePositionCounter);
        friend class TJsonValue;
    public:
        constexpr auto GetData() const -> std::string_view;
        constexpr auto At(size_t i) const -> TExpected<TJsonValue>;
        struct Iterator;
        constexpr auto begin() const -> Iterator;
        constexpr auto end() const -> Iterator;
    };

    class TJsonMapping {
    private:
        std::string_view Data;
        TLinePositionCounter LpCounter;
        constexpr TJsonMapping(std::string_view, TLinePositionCounter);
        friend class TJsonValue;
    public:
        constexpr auto GetData() const -> std::string_view;
        constexpr auto GetLpCounter() const -> TLinePositionCounter;
        constexpr auto At(std::string_view key) const -> TExpected<TJsonValue>;
        struct Iterator;
        constexpr auto begin() const -> Iterator;
        constexpr auto end() const -> Iterator;
    }; 

    class TJsonValue {
    private:
        std::string_view Data;
        TLinePositionCounter LpCounter;
    public:
        constexpr TJsonValue(std::string_view, TLinePositionCounter = {});
        constexpr auto GetData() const -> std::string_view;
        constexpr auto GetLpCounter() const -> TLinePositionCounter;
        constexpr auto AsInt() const -> TExpected<Int>;
        constexpr auto AsDouble() const -> TExpected<Double>; 
        constexpr auto AsString() const -> TExpected<String>;
        constexpr auto AsArray() const -> TExpected<TJsonArray>;
        constexpr auto AsMapping() const -> TExpected<TJsonMapping>;
    }; 
};