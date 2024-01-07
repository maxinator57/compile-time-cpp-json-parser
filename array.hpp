#pragma once


#include "api.hpp"
#include "utils.hpp"

#include <optional>
#include <stdexcept>
#include <string_view>
#include <ranges>


namespace NCompileTimeJsonParser {
    constexpr TJsonArray::TJsonArray(std::string_view data) : Data(data) {}

    constexpr auto TJsonArray::GetData() const -> std::string_view {
        return Data;
    }

    class TJsonArray::Iterator {
    private:
        std::string_view ArrayData;
        std::string_view::size_type CurElemStartPos = std::string_view::npos;
        std::string_view::size_type CurElemEndPos = std::string_view::npos;
    private: 
        friend class TJsonArray;
        friend class TMonadicOptional<TJsonArray>;
        constexpr Iterator(
            std::string_view arrayData,
            std::string_view::size_type startPos = std::string_view::npos
        )
            : ArrayData(arrayData)
            , CurElemStartPos(startPos)
            , CurElemEndPos(NUtils::FindCurElementEndPos(ArrayData, startPos))
        {
        }
    public:
        using difference_type = int;
        using value_type = TJsonValue;
    public:
        constexpr Iterator() = default;
        constexpr auto operator*() const -> value_type {
            if (CurElemStartPos == std::string_view::npos) throw std::runtime_error{
                "operator* called on end() json array iterator"
            };
            return ArrayData.substr(CurElemStartPos, CurElemEndPos - CurElemStartPos);
        } 
        constexpr auto operator++() -> Iterator& {
            if (CurElemStartPos == std::string_view::npos) throw std::runtime_error{
                "operator++ called on end() json array iterator"
            };
            CurElemStartPos = NUtils::FindNextElementStartPos(ArrayData, CurElemEndPos);
            CurElemEndPos = NUtils::FindCurElementEndPos(ArrayData, CurElemStartPos);
            return *this;
        }
        constexpr auto operator++(int) -> Iterator {
            auto copy = *this;
            ++(*this);
            return copy;
        }
        constexpr auto operator==(const Iterator&) const -> bool = default;
    };

    constexpr auto TJsonArray::begin() const -> Iterator {
        return Iterator(Data, NUtils::FindFirstOf(
            Data,
            [](char ch) { return !NUtils::IsSpace(ch); },
            0
        ));
    }

    constexpr auto TJsonArray::end() const -> Iterator {
        return Iterator(Data, std::string_view::npos);
    }

    constexpr auto TJsonArray::At(size_t idx) const -> TMonadicOptional<TJsonValue> {
        auto it = begin(), finish = end();
        auto counter = size_t{0};
        for (; it != finish && counter < idx; ++it, ++counter);
        return it == finish ? TMonadicOptional<TJsonValue>{std::nullopt} : *it;
    }

    constexpr auto TMonadicOptional<TJsonArray>::begin() const -> TJsonArray::Iterator {
        return has_value() ? value().begin() : TJsonArray::Iterator{};
    }

    constexpr auto TMonadicOptional<TJsonArray>::end() const -> TJsonArray::Iterator {
        return has_value() ? value().end() : TJsonArray::Iterator{};
    } 

    constexpr auto TMonadicOptional<TJsonArray>::At(size_t idx) const -> TMonadicOptional<TJsonValue> {
        return has_value() ? value().At(idx) : std::nullopt;
    }
}
