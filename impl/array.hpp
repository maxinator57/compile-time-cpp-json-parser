#pragma once


#include "api.hpp"
#include "error.hpp"
#include "iterator.hpp"

#include <cstddef>
#include <string_view>


namespace NCompileTimeJsonParser {
    constexpr TJsonArray::TJsonArray(std::string_view data, const TLinePositionCounter& lpCounter)
        : TDataHolderMixin(data, lpCounter) {}

    class TJsonArray::Iterator {
    private:
        TGenericSerializedSequenceIterator Iter;
        friend class TJsonArray;
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

    constexpr auto TJsonArray::begin() const -> Iterator { 
        return TGenericSerializedSequenceIterator::Begin(Data, LpCounter, ',');
    }

    constexpr auto TJsonArray::end() const -> Iterator {
        return TGenericSerializedSequenceIterator::End(Data, LpCounter);
    }

    constexpr auto TJsonArray::operator[](size_t idx) const -> TExpected<TJsonValue> {
        auto it = begin(), finish = end();
        auto counter = size_t{0};
        for (; it != finish && counter < idx; ++it, ++counter);
        if (it.Iter.HasError()) return it.Iter.Error();
        return it == finish
            ? Error(
                LpCounter.Copy().StepBack(),
                NError::ErrorCode::ArrayIndexOutOfRange,
                NError::TArrayIndexOutOfRangeAdditionalInfo{
                    .Index = idx,
                    .ArrayLen = counter
                })
            : *it;
    }

    constexpr auto TJsonArray::size() const -> size_t {
        auto counter = size_t{0};
        for (auto it = begin(); it != end(); ++it, ++counter);
        return counter; 
    }

    constexpr auto TExpected<TJsonArray>::begin() const -> TJsonArray::Iterator {
        // TODO: add error forwarding
        return HasValue() ? Value().begin() : TJsonArray::Iterator{};
    }

    constexpr auto TExpected<TJsonArray>::end() const -> TJsonArray::Iterator {
        // TODO: add error forwarding
        return HasValue() ? Value().end() : TJsonArray::Iterator{};
    } 

    constexpr auto TExpected<TJsonArray>::operator[](size_t idx) const -> TExpected<TJsonValue> {
        return HasValue() ? Value()[idx] : Error();
    }

    constexpr auto TExpected<TJsonArray>::size() const -> TExpected<size_t> {
        return HasValue() ? TExpected<size_t>{Value().size()} : Error();
    }
}
