#pragma once


#include "api.hpp"
#include "error.hpp"


namespace NJsonParser {
    // A mixin class template which provides some convenient operations
    // like those of c++23 `std::expected<T, E>`, but with E = `NError::TError`.
    template <class T>
    struct TExpectedMixin : private std::variant<T, NError::TError> {
        template <class... Args>
        constexpr TExpectedMixin(Args&&... args)
            : std::variant<T, NError::TError>(std::forward<Args>(args)...) {}

        // Common methods for (simplified)
        // c++23 std::expected-like types:

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
        constexpr auto Error() const -> const NError::TError& {
            return std::get<NError::TError>(*this);
        } 

        constexpr auto operator==(const TExpectedMixin<T>& other) const noexcept(
            noexcept(std::declval<T>() == std::declval<T>())
         && noexcept(Error() == other.Error())
        ) -> bool {
            static_assert(noexcept(std::declval<NError::TError>() == std::declval<NError::TError>()));
            if (HasValue() && other.HasValue()) return Value() == other.Value();
            if (HasError() && other.HasError()) return Error() == other.Error();
            return false;
        }
        // An `operator==` for efficient comparison with instances of `T`.
        // The comparison happens without the invocation of `TExpectedMixin` constructor
        template <class U>
        requires std::convertible_to<U, T>
        constexpr auto operator==(const U& otherValue) const -> bool {
            return HasValue() && Value() == otherValue;
        }
        // An `operator==` for efficient comparison with instances of `NError::TError`.
        // The comparison happens without the invocation of `TExpectedMixin` constructor
        template <class U>
        requires std::same_as<U, NError::TError>
        constexpr auto operator==(const U& otherError) const -> bool {
            return HasError() && Error() == otherError;
        }
    };

    // The general `TExpected` template
    template <class T>
    struct TExpected : public TExpectedMixin<T> {
        // Bring constructor from mixin to class scope
        using TExpectedMixin<T>::TExpectedMixin;
    };

    // The specialization of `TExpected` class template for `TJsonArray`
    template <>
    struct TExpected<TJsonArray> : public TExpectedMixin<TJsonArray> {
        // Bring constructor from mixin to class scope:
        using TExpectedMixin<TJsonArray>::TExpectedMixin;

        // Monadic methods specific to `TExpected<TJsonArray>`:
        constexpr auto operator[](size_t) const noexcept -> TExpected<TJsonValue>;
        constexpr auto size() const noexcept -> TExpected<size_t>;
        constexpr auto begin() const noexcept -> TJsonArray::Iterator;
        constexpr auto end() const noexcept -> TJsonArray::Iterator;
    };

    // The specialization of `TExpected` class template for `TJsonMapping`
    template <>
    struct TExpected<TJsonMapping> : public TExpectedMixin<TJsonMapping> {
        // Bring constructor from mixin to class scope:
        using TExpectedMixin<TJsonMapping>::TExpectedMixin;

        // Monadic methods specific to `TExpected<TJsonMapping>`:
        constexpr auto operator[](std::string_view) const noexcept -> TExpected<TJsonValue>;
        constexpr auto size() const noexcept -> TExpected<size_t>;
        constexpr auto begin() const noexcept -> TJsonMapping::Iterator;
        constexpr auto end() const noexcept -> TJsonMapping::Iterator;
    };

    // The specialization of `TExpected` class template for `TJsonValue`
    template <>
    struct TExpected<TJsonValue> : public TExpectedMixin<TJsonValue> {
        // Bring constructor from mixin to class scope:
        using TExpectedMixin<TJsonValue>::TExpectedMixin;

        // Monadic methods specific to `TExpected<TJsonValue>`:
        constexpr auto AsInt() const -> TExpected<Int>;
        constexpr auto AsDouble() const -> TExpected<Double>; 
        constexpr auto AsString() const -> TExpected<String>;

        constexpr auto AsArray() const -> TExpected<TJsonArray>;
        // Same effect as `.AsArray()[idx]`
        constexpr auto operator[](size_t idx) const -> TExpected<TJsonValue>;

        constexpr auto AsMapping() const -> TExpected<TJsonMapping>;
        // Same effect as `.AsMapping()[key]`
        constexpr auto operator[](std::string_view key) const -> TExpected<TJsonValue>;
    };
}
