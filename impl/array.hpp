#pragma once


#include "api.hpp"
#include "error.hpp"
#include "iterator.hpp"


namespace NJsonParser {
    constexpr Array::Array(std::string_view data, LinePositionCounter lpCounter) noexcept
        : DataHolderMixin(data, lpCounter) {}

    class Array::Iterator {
    private:
        GenericSerializedSequenceIterator Iter;
        friend class Array;
        friend struct Expected<Array>;
    private:
        constexpr Iterator(GenericSerializedSequenceIterator&& iter)
            : Iter(std::move(iter)) {}
    public:
        using difference_type = int;
        using value_type = Expected<JsonValue>;
    public:
        constexpr Iterator() : Iterator(GenericSerializedSequenceIterator::End({}, {})) {};
        constexpr auto operator*() const -> value_type { return *Iter; }
        constexpr auto operator++() -> Iterator& {
            Iter.StepForward(',', ',');
            return *this;
        }
        constexpr auto operator++(int) -> Iterator {
            auto copy = *this;
            ++(*this);
            return copy;
        }
        constexpr auto operator==(const Iterator& other) const -> bool = default;
    };

    constexpr auto Array::begin() const noexcept -> Iterator { 
        return GenericSerializedSequenceIterator::Begin(
            Data,
            LpCounter.Copy().Process('['),
            ','
        );
    }

    constexpr auto Array::end() const noexcept -> Iterator {
        return GenericSerializedSequenceIterator::End(
            Data,
            LpCounter.Copy().Process('[')
        );
    }

    constexpr auto Array::operator[](size_t idx) const noexcept -> Expected<JsonValue> { 
        size_t counter = 0;
        auto it = begin();
        const auto finish = end();
        for (; it != finish && !it.Iter.HasError() && counter < idx; ++it, ++counter);
        if (it.Iter.HasError()) return it.Iter.Error();
        if (it == finish) return MakeError(
            LpCounter,
            NError::ErrorCode::ArrayIndexOutOfRange,
            NError::ArrayIndexOutOfRangeAdditionalInfo{
                .Index = idx,
                .ArrayLen = counter
            }
        );
        return *it;
    }

    constexpr auto Array::size() const noexcept -> size_t {
        size_t counter = 0;
        const auto finish = end();
        for (auto it = begin(); it != finish; ++it, ++counter);
        return counter; 
    }

    constexpr auto Expected<Array>::begin() const noexcept -> Array::Iterator {
        return HasValue() ? Value().begin() : Array::Iterator{Error()};
    }

    constexpr auto Expected<Array>::end() const noexcept -> Array::Iterator {
        return HasValue() ? Value().end() : Array::Iterator{Error()};
    } 

    constexpr auto Expected<Array>::operator[](size_t idx) const noexcept -> Expected<JsonValue> {
        return HasValue() ? Value()[idx] : Error();
    }

    constexpr auto Expected<Array>::size() const noexcept -> Expected<size_t> {
        return HasValue() ? Expected<size_t>{Value().size()} : Error();
    }
}
