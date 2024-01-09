#pragma once


#include "api.hpp"
#include "error.hpp"
#include "utils.hpp"

#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>


namespace NCompileTimeJsonParser {
    constexpr TJsonMapping::TJsonMapping(std::string_view data) : Data(data) {}

    constexpr auto TJsonMapping::GetData() const -> std::string_view {
        return Data;
    }

    class TJsonMapping::Iterator {
    private:
        std::string_view MappingData;
        std::string_view::size_type CurKeyStartPos = std::string_view::npos;
        std::string_view::size_type CurKeyEndPos = std::string_view::npos;
        std::string_view::size_type CurValueStartPos = std::string_view::npos;
        std::string_view::size_type CurValueEndPos = std::string_view::npos;
    private: 
        friend class TJsonMapping;
        friend class TExpected<TJsonMapping>;
        constexpr Iterator(
            std::string_view mappingData,
            std::string_view::size_type startPos = std::string_view::npos
        )
            : MappingData(mappingData)
            , CurKeyStartPos(startPos)
            , CurKeyEndPos(NUtils::FindCurElementEndPos(MappingData, CurKeyStartPos, ':'))
            , CurValueStartPos(NUtils::FindNextElementStartPos(MappingData, CurKeyStartPos, ':'))
            , CurValueEndPos(NUtils::FindCurElementEndPos(MappingData, CurValueStartPos))
        {
        }
    public:
        using difference_type = int;
        using value_type = std::pair<std::string_view, TJsonValue>;
    public:
        constexpr Iterator() = default;
        constexpr auto operator*() const -> value_type {
            if (CurKeyStartPos == std::string_view::npos) throw NError::IteratorDereferenceError{
                "operator* called on end() json mapping iterator of mapping ("
            };
            const auto keyData = MappingData.substr(
                CurKeyStartPos,
                CurKeyEndPos - CurKeyStartPos
            );
            const auto key = TJsonValue{keyData}.AsString();
            if (!key.has_value()) throw NError::ParseError{(
                std::stringstream{}
                << "Error when parsing json mapping: encountered a non-string key ("
                << keyData << ")"
            ).str()};
            return {
                key.value(),
                MappingData.substr(CurValueStartPos, CurValueEndPos - CurValueStartPos)
            };
        } 
        constexpr auto operator++() -> Iterator& {
            if (CurKeyStartPos == std::string_view::npos) throw NError::IteratorIncrementError{
                "operator++ called on end() json mapping iterator"
            };

            CurKeyStartPos = NUtils::FindNextElementStartPos(MappingData, CurValueEndPos);
            CurKeyEndPos = NUtils::FindCurElementEndPos(MappingData, CurKeyStartPos, ':'); 
            CurValueStartPos = NUtils::FindNextElementStartPos(MappingData, CurKeyStartPos, ':');
            if (CurKeyStartPos != std::string_view::npos
                && CurValueStartPos == std::string_view::npos
            ) throw NError::ParseError{(
                std::stringstream{}
                << "Error when parsing json mapping: encountered a key without value ("
                << MappingData.substr(CurKeyStartPos, CurKeyEndPos - CurKeyStartPos) << ")"
            ).str()};
            CurValueEndPos = NUtils::FindCurElementEndPos(MappingData, CurValueStartPos); 
            return *this;
        }
        constexpr auto operator++(int) -> Iterator {
            auto copy = *this;
            ++(*this);
            return copy;
        }
        constexpr auto operator==(const Iterator&) const -> bool = default;
    };

    constexpr auto TJsonMapping::begin() const -> Iterator {
        return Iterator(Data, NUtils::FindFirstOf(
            Data,
            [](char ch) { return !NUtils::IsSpace(ch); },
            0
        ));
    }

    constexpr auto TJsonMapping::end() const -> Iterator {
        return Iterator(Data, std::string_view::npos);
    }

    constexpr auto TJsonMapping::At(std::string_view key) const -> TMonadicOptional<TJsonValue> {
        auto finish = end();
        for (auto [k, v] : *this) if (key == k) return v;
        return std::nullopt;
    }

    constexpr auto TMonadicOptional<TJsonMapping>::begin() const -> TJsonMapping::Iterator {
        return has_value() ? value().begin() : TJsonMapping::Iterator{};
    }

    constexpr auto TMonadicOptional<TJsonMapping>::end() const -> TJsonMapping::Iterator {
        return has_value() ? value().end() : TJsonMapping::Iterator{};
    }

    constexpr auto TMonadicOptional<TJsonMapping>::At(
        std::string_view key
    ) const -> TMonadicOptional<TJsonValue> {
        return has_value() ? value().At(key) : std::nullopt;
    }  
} // namespace NCompileTimeJsonParser
