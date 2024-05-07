#pragma once


#include "data_holder.hpp"


namespace NJsonParser {
    // A custom type like c++23 `std::expected`
    template <class T> struct Expected; 

    // Json types:
    using Int = int64_t;
    using Double = double;
    using String = std::string_view;
    class Array;
    class Mapping;
    // A concept that acts as a sum of all json types
    template <class T>
    concept CJsonType = std::same_as<T, Int>
                     || std::same_as<T, Double>
                     || std::same_as<T, String>
                     || std::same_as<T, Array>
                     || std::same_as<T, Mapping>;
    // A type that holds an arbitrary json value
    class JsonValue;


    class Array : public DataHolderMixin {
    private:
        constexpr Array(std::string_view, LinePositionCounter) noexcept;
        friend class JsonValue;
    public:
        constexpr auto operator[](size_t idx) const noexcept -> Expected<JsonValue>;
        constexpr auto size() const noexcept -> size_t;
        class Iterator;
        constexpr auto begin() const noexcept -> Iterator;
        constexpr auto end() const noexcept -> Iterator;
    };


    class Mapping : public DataHolderMixin {
    private:
        constexpr Mapping(std::string_view, LinePositionCounter) noexcept;
        friend class JsonValue;
    public:
        constexpr auto operator[](std::string_view key) const noexcept -> Expected<JsonValue>;
        constexpr auto size() const noexcept -> size_t;
        class Iterator;
        constexpr auto begin() const noexcept -> Iterator;
        constexpr auto end() const noexcept -> Iterator;
    }; 


    class JsonValue : public DataHolderMixin {
    public:
        constexpr JsonValue(std::string_view, LinePositionCounter = {}) noexcept;
        template <CJsonType T> constexpr auto As() const noexcept -> Expected<T>;
        // Same effect as `.As<Array>()[idx]`
        constexpr auto operator[](size_t idx) const noexcept -> Expected<JsonValue>;
        // Same effect as `.As<Mapping>()[key]`
        constexpr auto operator[](std::string_view key) const noexcept -> Expected<JsonValue>;
    }; 
}
