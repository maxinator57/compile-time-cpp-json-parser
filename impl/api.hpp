#pragma once


#include "data_holder.hpp"

#include <cstddef>
#include <string_view>


namespace NCompileTimeJsonParser {
    // A custom type like c++23 `std::expected`
    template <class T> class TExpected; 

    // JSON types:
    using Int = int64_t;
    using Double = double;
    using String = std::string_view;
    class TJsonArray;
    class TJsonMapping;
    // A type for holding an arbitrary json value
    class TJsonValue;


    class TJsonArray : public TDataHolderMixin {
    private:
        constexpr TJsonArray(std::string_view, const TLinePositionCounter&);
        friend class TJsonValue;
    public:
        constexpr auto operator[](size_t idx) const -> TExpected<TJsonValue>;
        constexpr auto size() const -> size_t;

        class Iterator;
        constexpr auto begin() const -> Iterator;
        constexpr auto end() const -> Iterator;
    };


    class TJsonMapping : public TDataHolderMixin {
    private:
        constexpr TJsonMapping(std::string_view, const TLinePositionCounter&);
        friend class TJsonValue;
    public:
        constexpr auto operator[](std::string_view key) const -> TExpected<TJsonValue>;
        constexpr auto size() const -> size_t;

        class Iterator;
        constexpr auto begin() const -> Iterator;
        constexpr auto end() const -> Iterator;
    }; 


    class TJsonValue : public TDataHolderMixin {
    public:
        constexpr TJsonValue(std::string_view, const TLinePositionCounter& = {});

        constexpr auto AsInt() const -> TExpected<Int>;
        constexpr auto AsDouble() const -> TExpected<Double>;
        constexpr auto AsString() const -> TExpected<String>;

        constexpr auto AsArray() const -> TExpected<TJsonArray>;
        constexpr auto operator[](size_t idx) const -> TExpected<TJsonValue>; // same effect as `.AsArray()[idx]`

        constexpr auto AsMapping() const -> TExpected<TJsonMapping>;
        constexpr auto operator[](std::string_view key) const -> TExpected<TJsonValue>; // same effect as `.AsMapping()[idx]`
    }; 
};
