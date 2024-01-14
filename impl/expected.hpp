#pragma once


#include "api.hpp"
#include "error.hpp"

#include <variant>


namespace NCompileTimeJsonParser {
    template <class T>
    struct TExpectedMixin : private std::variant<T, NError::TError> {
        template <class U>
        constexpr TExpectedMixin(U&& value)
            : std::variant<T, NError::TError>(std::forward<U>(value))
        {
        } 

        // Common methods for (simplified)
        // c++23 std::expected-like types:
        constexpr auto HasValue() const noexcept -> bool {
            return std::holds_alternative<T>(*this);
        }
        constexpr auto HasError() const noexcept -> bool {
            return !HasValue();
        }
        constexpr auto Value() const -> const T& {
            return std::get<T>(*this);
        }
        constexpr auto Value() -> T& {
            return std::get<T>(*this);
        }
        constexpr auto Error() const -> const NError::TError& {
            return std::get<NError::TError>(*this);
        }
        constexpr auto Error() -> NError::TError& {
            return std::get<NError::TError>(*this);
        }
        constexpr auto operator==(const TExpectedMixin<T>& other) const -> bool {
            if (HasValue() && other.HasValue()) return Value() == other.Value();
            if (HasError() && other.HasError()) return Error() == other.Error();
            return false;
        }
        // constexpr auto operator==(const T& otherValue) const -> bool {
        //     return HasValue() && Value() == otherValue;
        // }
        // constexpr auto operator==(const NError::TError& otherError) const -> bool {
        //     return HasError() && Error() == otherError;
        // }
    };

    template <class T>
    struct TExpected : public TExpectedMixin<T> {
        // Bring constructor from mixin to class scope:
        using TExpectedMixin<T>::TExpectedMixin;

        // Other mixin methods are available
        // thanks to public inheritance
    };

    template <>
    struct TExpected<TJsonArray> : public TExpectedMixin<TJsonArray> {
        // Bring constructor from mixin to class scope:
        using TExpectedMixin<TJsonArray>::TExpectedMixin;

        // Other mixin methods are available
        // thanks to public inheritance

        // Monadic methods specific to TExpected<TJsonArray>:
        constexpr auto At(size_t i) const -> TExpected<TJsonValue>;
        constexpr auto begin() const -> TJsonArray::Iterator;
        constexpr auto end() const -> TJsonArray::Iterator;
    };

    template <>
    struct TExpected<TJsonMapping> : public TExpectedMixin<TJsonMapping> {
        // Bring constructor from mixin to class scope:
        using TExpectedMixin<TJsonMapping>::TExpectedMixin;

        // Other mixin methods are available
        // thanks to public inheritance

        // Monadic methods specific to TExpected<TJsonMapping>:
        constexpr auto At(std::string_view key) const -> TExpected<TJsonValue>;
        constexpr auto begin() const -> TJsonMapping::Iterator;
        constexpr auto end() const -> TJsonMapping::Iterator;
    };

    template <>
    struct TExpected<TJsonValue> : public TExpectedMixin<TJsonValue> {
        // Bring constructor from mixin to class scope:
        using TExpectedMixin<TJsonValue>::TExpectedMixin;

        // Other mixin methods are available
        // thanks to public inheritance

        // Monadic methods specific to TExpected<TJsonValue>:
        constexpr auto AsInt() const -> TExpected<Int>;
        constexpr auto AsDouble() const -> TExpected<Double>; 
        constexpr auto AsString() const -> TExpected<String>;
        constexpr auto AsArray() const -> TExpected<TJsonArray>;
        constexpr auto AsMapping() const -> TExpected<TJsonMapping>;
    };
}