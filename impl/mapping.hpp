#pragma once


#include "api.hpp"
#include "error.hpp"
#include "expected.hpp"
#include "iterator.hpp"
#include "line_position_counter.hpp"


namespace NJsonParser {
    constexpr Mapping::Mapping(std::string_view data, LinePositionCounter lpCounter)
        : DataHolderMixin(data, lpCounter) {}

    class Mapping::Iterator {
    private:
        GenericSerializedSequenceIterator KeyIter;
        GenericSerializedSequenceIterator ValIter;
        friend class Mapping;
        friend struct Expected<Mapping>;
    private:
        constexpr Iterator(GenericSerializedSequenceIterator&& iter)
            : KeyIter(std::move(iter))
            , ValIter(KeyIter)
        {
            ValIter.StepForward(':', ',');
        }

    public:
        using difference_type = int;
        struct value_type {
            Expected<String> Key;
            Expected<JsonValue> Value;
        };
    public:
        constexpr Iterator() : Iterator(GenericSerializedSequenceIterator::End({}, {})) {};
        constexpr auto operator*() const -> value_type {
            // TODO: improve error handling here (return more specific errors when possible)
            return {
                .Key = (*KeyIter).As<String>(),
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

    constexpr auto Mapping::begin() const -> Iterator { 
        return GenericSerializedSequenceIterator::Begin(Data, LpCounter, ':');
    }

    constexpr auto Mapping::end() const -> Iterator {
        return GenericSerializedSequenceIterator::End(Data, LpCounter);
    }

    constexpr auto Mapping::operator[](std::string_view key) const -> Expected<JsonValue> {
        auto it = begin();
        for (; it != end(); ++it) {
            auto [k, v] = *it;
            if (k.HasError()) return k.Error();
            if (v.HasError()) return v.Error();
            if (k == key) return v;
        }
        return MakeError(
            LpCounter,
            NError::ErrorCode::MappingKeyNotFound,
            NError::MappingKeyNotFoundAdditionalInfo{key}
        );
    }

    constexpr auto Mapping::size() const -> size_t {
        size_t counter = 0;
        for (auto it = begin(); it != end(); ++it, ++counter);
        return counter;
    }

    constexpr auto Expected<Mapping>::begin() const noexcept -> Mapping::Iterator {
        return HasValue() ? Value().begin() : Mapping::Iterator{Error()};
    }

    constexpr auto Expected<Mapping>::end() const noexcept -> Mapping::Iterator {
        return HasValue() ? Value().end() : Mapping::Iterator{Error()};
    }

    constexpr auto Expected<Mapping>::operator[](std::string_view key) const noexcept
    -> Expected<JsonValue> {
        return HasValue() ? Value()[key] : Error();
    }

    constexpr auto Expected<Mapping>::size() const noexcept -> Expected<size_t> {
        return HasValue() ? Expected<size_t>{Value().size()} : Error();
    }
} // namespace NJsonParser
