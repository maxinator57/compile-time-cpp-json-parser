#pragma once


#include "api.hpp"
#include "error.hpp"


namespace NJsonParser {
    // A mixin class template which provides some convenient operations
    // like those of c++23 `std::expected<T, E>`, but with E = `NError::Error`.
    template <class T>
    struct ExpectedMixin : private std::variant<T, NError::Error> {
        template <class... Args>
        constexpr ExpectedMixin(Args&&... args)
            : std::variant<T, NError::Error>(std::forward<Args>(args)...) {}

        // Common methods for (simplified)
        // c++23 `std::expected`-like types:

        constexpr auto HasValue() const noexcept -> bool {
            return std::holds_alternative<T>(*this);
        }
        constexpr auto HasError() const noexcept -> bool {
            return !HasValue();
        }
        // Provides only const version of the `Value()` method on purpose
        constexpr auto Value() const -> const T& {
            return std::get<T>(*this);
        }
        // Provides only const version of the `Error()` method on purpose
        constexpr auto Error() const -> const NError::Error& {
            return std::get<NError::Error>(*this);
        } 

        constexpr auto operator==(const ExpectedMixin<T>& other) const noexcept(
            noexcept(std::declval<T>() == std::declval<T>())
        ) -> bool {
            static_assert(noexcept(std::declval<NError::Error>() == std::declval<NError::Error>()));
            if (HasValue() && other.HasValue()) return Value() == other.Value();
            if (HasError() && other.HasError()) return Error() == other.Error();
            return false;
        }
        // An `operator==` for efficient comparison with instances of `T`.
        // The comparison happens without the invocation of `ExpectedMixin` constructor
        template <std::equality_comparable_with<T> U>
        constexpr auto operator==(const U& otherValue) const -> bool {
            return HasValue() && Value() == otherValue;
        }
        // An `operator==` for efficient comparison with instances of `NError::Error`.
        // The comparison happens without the invocation of `ExpectedMixin` constructor
        template <std::same_as<NError::Error> U>
        constexpr auto operator==(const U& otherError) const noexcept -> bool {
            return HasError() && Error() == otherError;
        }
    };

    // The general `Expected` template
    template <class T>
    struct Expected : public ExpectedMixin<T> {
        // Bring constructor from mixin to class scope:
        using ExpectedMixin<T>::ExpectedMixin;
    };

    // The specialization of `Expected` class template for `Array`
    template <>
    struct Expected<Array> : public ExpectedMixin<Array> {
        // Bring constructor from mixin to class scope:
        using ExpectedMixin<Array>::ExpectedMixin;
        // Monadic methods specific to `Expected<Array>`:
        constexpr auto operator[](size_t) const noexcept -> Expected<JsonValue>;
        constexpr auto size() const noexcept -> Expected<size_t>;
        constexpr auto begin() const noexcept -> Array::Iterator;
        constexpr auto end() const noexcept -> Array::Iterator;
    };

    // The specialization of `Expected` class template for `Mapping`
    template <>
    struct Expected<Mapping> : public ExpectedMixin<Mapping> {
        // Bring constructor from mixin to class scope:
        using ExpectedMixin<Mapping>::ExpectedMixin;
        // Monadic methods specific to `Expected<Mapping>`:
        constexpr auto operator[](std::string_view) const noexcept -> Expected<JsonValue>;
        constexpr auto size() const noexcept -> Expected<size_t>;
        constexpr auto begin() const noexcept -> Mapping::Iterator;
        constexpr auto end() const noexcept -> Mapping::Iterator;
    };

    // The specialization of `Expected` class template for `JsonValue`
    template <>
    struct Expected<JsonValue> : public ExpectedMixin<JsonValue> {
        // Bring constructor from mixin to class scope:
        using ExpectedMixin<JsonValue>::ExpectedMixin;
        // Monadic methods specific to `Expected<JsonValue>`:
        template <CJsonType T> constexpr auto As() const -> Expected<T>;
        // Same effect as `.As<Array>()[idx]`
        constexpr auto operator[](size_t idx) const -> Expected<JsonValue>;
        // Same effect as `.As<Mapping>()[key]`
        constexpr auto operator[](std::string_view key) const -> Expected<JsonValue>;
    };
}
