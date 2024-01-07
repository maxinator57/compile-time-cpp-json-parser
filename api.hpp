#pragma once


#include <optional>
#include <string_view>


namespace NCompileTimeJsonParser {  
    class TJsonValue;

    template <class T>
    struct TMonadicOptional;
    
    class TJsonArray {
    private:
        std::string_view Data;
        constexpr TJsonArray(std::string_view);
        friend class TJsonValue;
    public:
        constexpr auto GetData() const -> std::string_view;
        constexpr auto At(size_t i) const -> TMonadicOptional<TJsonValue>;
        struct Iterator;
        constexpr auto begin() const -> Iterator;
        constexpr auto end() const -> Iterator;
    };
    template <>
    struct TMonadicOptional<TJsonArray> : private std::optional<TJsonArray> {
        using std::optional<TJsonArray>::optional;
        using std::optional<TJsonArray>::has_value;
        using std::optional<TJsonArray>::value;
        constexpr auto At(size_t i) const -> TMonadicOptional<TJsonValue>;
        constexpr auto begin() const -> TJsonArray::Iterator;
        constexpr auto end() const -> TJsonArray::Iterator;
    };

    class TJsonMapping {
    private:
        std::string_view Data;
        constexpr TJsonMapping(std::string_view);
        friend class TJsonValue;
    public:
        constexpr auto GetData() const -> std::string_view;
        constexpr auto At(std::string_view key) const -> TMonadicOptional<TJsonValue>;
        struct Iterator;
        constexpr auto begin() const -> Iterator;
        constexpr auto end() const -> Iterator;
    };
    template <>
    struct TMonadicOptional<TJsonMapping> : private std::optional<TJsonMapping> {
        using std::optional<TJsonMapping>::optional;
        using std::optional<TJsonMapping>::has_value;
        using std::optional<TJsonMapping>::value;
        constexpr auto At(std::string_view key) const -> TMonadicOptional<TJsonValue>;
        constexpr auto begin() const -> TJsonMapping::Iterator;
        constexpr auto end() const -> TJsonMapping::Iterator;
    };

    class TJsonValue {
    private:
        std::string_view Data;
    public:
        constexpr TJsonValue(std::string_view data);
        constexpr auto GetData() const -> std::string_view;
        constexpr auto AsInt64() const -> std::optional<int64_t>;
        constexpr auto AsDouble() const -> std::optional<double>; 
        constexpr auto AsString() const -> std::optional<std::string_view>;
        constexpr auto AsArray() const -> TMonadicOptional<TJsonArray>;
        constexpr auto AsMapping() const -> TMonadicOptional<TJsonMapping>;
    };
    template <>
    struct TMonadicOptional<TJsonValue> : private std::optional<TJsonValue> {
        using std::optional<TJsonValue>::optional;
        using std::optional<TJsonValue>::has_value;
        using std::optional<TJsonValue>::value;
        constexpr auto AsInt64() const -> std::optional<int64_t>;
        constexpr auto AsDouble() const -> std::optional<double>; 
        constexpr auto AsString() const -> std::optional<std::string_view>;
        constexpr auto AsArray() const -> TMonadicOptional<TJsonArray>;
        constexpr auto AsMapping() const -> TMonadicOptional<TJsonMapping>;
    };
};