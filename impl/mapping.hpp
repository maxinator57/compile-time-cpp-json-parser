#pragma once


#include "api.hpp"
#include "error.hpp"
#include "expected.hpp"
#include "iterator.hpp"
#include "line_position_counter.hpp"

#include <string_view>


namespace NCompileTimeJsonParser {
    constexpr TJsonMapping::TJsonMapping(
        std::string_view data,
        TLinePositionCounter lpCounter
    ) : Data(data), LpCounter(lpCounter) {}

    constexpr auto TJsonMapping::GetData() const -> std::string_view {
        return Data;
    }

    constexpr auto TJsonMapping::GetLpCounter() const -> TLinePositionCounter {
        return LpCounter;
    }

    class TJsonMapping::Iterator {
    private:
        TGenericSerializedSequenceIterator KeyIter;
        TGenericSerializedSequenceIterator ValIter;
        friend class TJsonMapping;
    private:
        constexpr Iterator(
            TGenericSerializedSequenceIterator&& iter
        ) : KeyIter(std::move(iter)), ValIter(KeyIter) {
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
            auto k = (*KeyIter).AsString();
            auto v = *ValIter;
            return {.Key = k, .Value = v};
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

    constexpr auto TJsonMapping::At(std::string_view key) const -> TExpected<TJsonValue> {
        auto it = begin();
        for (; it != end(); ++it) {
            auto [k, v] = *it;
            if (k == key) return v;
        }
        if (it.KeyIter.HasError()) return it.KeyIter.Error();
        if (it.ValIter.HasError()) return it.ValIter.Error();
        return Error(
            LpCounter,
            NError::ErrorCode::MappingKeyNotFound,
            NError::TMappingKeyNotFoundAdditionalInfo{key}
        );
    }

    constexpr auto TExpected<TJsonMapping>::begin() const -> TJsonMapping::Iterator {
        return HasValue() ? Value().begin() : TJsonMapping::Iterator{};
    }

    constexpr auto TExpected<TJsonMapping>::end() const -> TJsonMapping::Iterator {
        return HasValue() ? Value().end() : TJsonMapping::Iterator{};
    }

    constexpr auto TExpected<TJsonMapping>::At(
        std::string_view key
    ) const -> TExpected<TJsonValue> {
        return HasValue() ? Value().At(key) : Error();
    }  
} // namespace NCompileTimeJsonParser
