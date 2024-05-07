#pragma once


#include "api.hpp"
#include "error.hpp"
#include "expected.hpp"
#include "iterator.hpp"
#include "line_position_counter.hpp"


namespace NJsonParser {
    constexpr TJsonMapping::TJsonMapping(std::string_view data, const TLinePositionCounter& lpCounter)
        : TDataHolderMixin(data, lpCounter) {}

    class TJsonMapping::Iterator {
    private:
        TGenericSerializedSequenceIterator KeyIter;
        TGenericSerializedSequenceIterator ValIter;
        friend class TJsonMapping;
        friend struct TExpected<TJsonMapping>;
    private:
        constexpr Iterator(TGenericSerializedSequenceIterator&& iter)
            : KeyIter(std::move(iter))
            , ValIter(KeyIter)
        {
            ValIter.StepForward(':', ',');
        }

    public:
        using difference_type = int;
        struct value_type {
            TExpected<String> Key;
            TExpected<TJsonValue> Value;
        };
    public:
        constexpr Iterator() : Iterator(TGenericSerializedSequenceIterator::End({}, {})) {};
        constexpr auto operator*() const -> value_type {
            // TODO: improve error handling here (return more specific errors when possible)
            return {
                .Key = (*KeyIter).AsString(),
                .Value = *ValIter,
            };
        }
        constexpr auto operator++() -> Iterator& {
            if (KeyIter.IsEnd()) return *this;
            KeyIter = ValIter.StepForward(',', ':');
            ValIter.StepForward(':', ',');
            return *this;
        }
        constexpr auto operator++(int) -> Iterator {
            auto copy = *this;
            ++(*this);
            return copy;
        }
        constexpr auto operator==(const Iterator& other) const -> bool = default;
    };

    constexpr auto TJsonMapping::begin() const -> Iterator { 
        return TGenericSerializedSequenceIterator::Begin(Data, LpCounter, ':');
    }

    constexpr auto TJsonMapping::end() const -> Iterator {
        return TGenericSerializedSequenceIterator::End(Data, LpCounter);
    }

    constexpr auto TJsonMapping::operator[](std::string_view key) const -> TExpected<TJsonValue> {
        auto it = begin();
        for (; it != end(); ++it) {
            auto [k, v] = *it;
            if (k.HasError()) return k.Error();
            if (v.HasError()) return v.Error();
            if (k == key) return v;
        }
        return Error(
            LpCounter,
            NError::ErrorCode::MappingKeyNotFound,
            NError::TMappingKeyNotFoundAdditionalInfo{key}
        );
    }

    constexpr auto TJsonMapping::size() const -> size_t {
        size_t counter = 0;
        for (auto it = begin(); it != end(); ++it, ++counter);
        return counter;
    }

    constexpr auto TExpected<TJsonMapping>::begin() const noexcept -> TJsonMapping::Iterator {
        return HasValue() ? Value().begin() : TJsonMapping::Iterator{Error()};
    }

    constexpr auto TExpected<TJsonMapping>::end() const noexcept -> TJsonMapping::Iterator {
        return HasValue() ? Value().end() : TJsonMapping::Iterator{Error()};
    }

    constexpr auto TExpected<TJsonMapping>::operator[](std::string_view key) const noexcept
    -> TExpected<TJsonValue> {
        return HasValue() ? Value()[key] : Error();
    }

    constexpr auto TExpected<TJsonMapping>::size() const noexcept -> TExpected<size_t> {
        return HasValue() ? TExpected<size_t>{Value().size()} : Error();
    }
} // namespace NJsonParser
