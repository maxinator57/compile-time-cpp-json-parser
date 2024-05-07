#pragma once


#include "api.hpp"
#include "error.hpp"
#include "iterator.hpp"


namespace NJsonParser {
    constexpr TJsonArray::TJsonArray(std::string_view data, const TLinePositionCounter& lpCounter) noexcept
        : TDataHolderMixin(data, lpCounter) {}

    class TJsonArray::Iterator {
    private:
        TGenericSerializedSequenceIterator Iter;
        friend class TJsonArray;
        friend struct TExpected<TJsonArray>;
    private:
        constexpr Iterator(TGenericSerializedSequenceIterator&& iter)
            : Iter(std::move(iter)) {}
    public:
        using difference_type = int;
        using value_type = TExpected<TJsonValue>;
    public:
        constexpr Iterator() : Iterator(TGenericSerializedSequenceIterator::End({}, {})) {};
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

    constexpr auto TJsonArray::begin() const noexcept -> Iterator { 
        return TGenericSerializedSequenceIterator::Begin(
            Data,
            LpCounter.Copy().Process('['),
            ','
        );
    }

    constexpr auto TJsonArray::end() const noexcept -> Iterator {
        return TGenericSerializedSequenceIterator::End(
            Data,
            LpCounter.Copy().Process('[')
        );
    }

    constexpr auto TJsonArray::operator[](size_t idx) const noexcept -> TExpected<TJsonValue> { 
        size_t counter = 0;
        auto it = begin(), finish = end();
        for (; it != finish && !it.Iter.HasError() && counter < idx; ++it, ++counter);
        if (it.Iter.HasError()) return it.Iter.Error();
        if (it == finish) return Error(
            LpCounter,
            NError::ErrorCode::ArrayIndexOutOfRange,
            NError::TArrayIndexOutOfRangeAdditionalInfo{
                .Index = idx,
                .ArrayLen = counter
            }
        );
        return *it;
    }

    constexpr auto TJsonArray::size() const noexcept -> size_t {
        size_t counter = 0;
        for (auto it = begin(); it != end(); ++it, ++counter);
        return counter; 
    }

    constexpr auto TExpected<TJsonArray>::begin() const noexcept -> TJsonArray::Iterator {
        return HasValue() ? Value().begin() : TJsonArray::Iterator{Error()};
    }

    constexpr auto TExpected<TJsonArray>::end() const noexcept -> TJsonArray::Iterator {
        return HasValue() ? Value().end() : TJsonArray::Iterator{Error()};
    } 

    constexpr auto TExpected<TJsonArray>::operator[](size_t idx) const noexcept -> TExpected<TJsonValue> {
        return HasValue() ? Value()[idx] : Error();
    }

    constexpr auto TExpected<TJsonArray>::size() const noexcept -> TExpected<size_t> {
        return HasValue() ? TExpected<size_t>{Value().size()} : Error();
    }
}
