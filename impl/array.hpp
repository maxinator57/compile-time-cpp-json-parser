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
        constexpr Iterator(const GenericSerializedSequenceIterator& iter)
            : Iter(iter) {}
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
        size_t i = 0;
        auto it = begin();
        for (; it != end(); ++it, ++i) {
            const auto elem = *it;
            if (i == idx) return elem;
            if (elem.HasError()) return elem;
        }
        if (it.Iter.HasError()) return it.Iter.Error();
        return MakeError(
            LpCounter,
            NError::ErrorCode::ArrayIndexOutOfRange,
            NError::ArrayIndexOutOfRangeAdditionalInfo{
                .Index = idx,
                .ArrayLen = i
            }
        );
    }

    constexpr auto Array::size() const noexcept -> size_t {
        return std::distance(begin(), end());
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
