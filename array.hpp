#pragma once


#include "api.hpp"
#include "error.hpp"
#include "expected.hpp"
#include "line_position_counter.hpp"
#include "utils.hpp"

#include <iostream>
#include <optional>
#include <string_view>


namespace NCompileTimeJsonParser {
    constexpr TJsonArray::TJsonArray(
        std::string_view data,
        TLinePositionCounter lpCounter
    ) : Data(data), LpCounter(lpCounter) {}

    constexpr auto TJsonArray::GetData() const -> std::string_view {
        return Data;
    }

    class TJsonArray::Iterator {
    private:
        std::string_view ArrayData; 
        TLinePositionCounter LpCounter;
        std::string_view::size_type CurElemStartPos = std::string_view::npos;
        std::string_view::size_type CurElemEndPos = std::string_view::npos;
        std::optional<NError::TError> Error;
    private: 
        friend class TJsonArray;
        friend class TExpected<TJsonArray>;
        constexpr auto SetError(NError::TError&& err) -> void {
            // Make this iterator equal to TJsonArray::end()
            CurElemStartPos = std::string_view::npos;
            CurElemEndPos = std::string_view::npos;
            // Move `err` to `Error` field
            Error.emplace(std::move(err));
        }
        constexpr Iterator(
            std::string_view arrayData, 
            TLinePositionCounter lpCounter,
            std::string_view::size_type startPos = std::string_view::npos
        )
            : ArrayData(arrayData)
            , LpCounter(lpCounter)
            , CurElemStartPos(startPos)
            , CurElemEndPos(0)
        { 
            auto endPosOrErr = CurElemStartPos == std::string_view::npos 
                ? std::string_view::npos
                : NUtils::FindCurElementEndPos(ArrayData, lpCounter, CurElemStartPos);

            if (endPosOrErr.HasError())
                SetError(std::move(endPosOrErr.Error()));
            else
                CurElemEndPos = endPosOrErr.Value();
        }
    public:
        using difference_type = int;
        using value_type = TExpected<TJsonValue>;
    public:
        constexpr Iterator() = default;
        constexpr auto operator*() const -> value_type {
            if (Error) return Error.value();
            return TJsonValue{
                ArrayData.substr(CurElemStartPos, CurElemEndPos - CurElemStartPos),
                LpCounter
            };
        } 
        constexpr auto operator++() -> Iterator& {
            if (Error) return *this;
            if (CurElemStartPos == std::string_view::npos) throw NError::IteratorIncrementError{
                "operator++ called on end() json array iterator"
            };
            {
                auto posOrErr = NUtils::FindNextElementStartPos(ArrayData, LpCounter, CurElemEndPos);
                if (posOrErr.HasError()) {
                    SetError(std::move(posOrErr.Error()));
                    return *this;
                }
                CurElemStartPos = posOrErr.Value();
                if (CurElemStartPos == std::string_view::npos) {
                    CurElemEndPos = std::string_view::npos;
                    return *this;
                }
            }
            {
                auto posOrErr = NUtils::FindCurElementEndPos(ArrayData, LpCounter, CurElemStartPos);
                if (posOrErr.HasError()) {
                    SetError(std::move(posOrErr.Error()));
                    return *this;
                }
                CurElemEndPos = posOrErr.Value();
            }
            return *this;
        }
        constexpr auto operator++(int) -> Iterator {
            auto copy = *this;
            ++(*this);
            return copy;
        }
        constexpr auto operator==(const Iterator& other) const -> bool {
            return ArrayData == other.ArrayData
                && CurElemStartPos == other.CurElemStartPos
                && CurElemEndPos == other.CurElemEndPos;
        }
    };

    constexpr auto TJsonArray::begin() const -> Iterator {
        auto lpCounterCopy = LpCounter;
        auto startPos = NUtils::FindFirstOf(
            Data,
            lpCounterCopy,
            [](char ch) { return !NUtils::IsSpace(ch); },
            0
        );
        return {Data, std::move(lpCounterCopy), startPos};
    }

    constexpr auto TJsonArray::end() const -> Iterator {
        return {
            Data,
            LpCounter,
            std::string_view::npos
        };
    }

    constexpr auto TJsonArray::At(size_t idx) const -> TExpected<TJsonValue> {
        auto it = begin(), finish = end();
        auto counter = size_t{0};
        for (; it != finish && counter < idx; ++it, ++counter);
        if (it.Error) return it.Error.value();
        return it == finish
            ? Error(LpCounter, NError::ErrorCode::ArrayIndexOutOfRange)
            : *it;
    }

    constexpr auto TExpected<TJsonArray>::begin() const -> TJsonArray::Iterator {
        // TODO: add error forwarding
        return HasValue() ? Value().begin() : TJsonArray::Iterator{};
    }

    constexpr auto TExpected<TJsonArray>::end() const -> TJsonArray::Iterator {
        // TODO: add error forwarding
        return HasValue() ? Value().end() : TJsonArray::Iterator{};
    } 

    constexpr auto TExpected<TJsonArray>::At(size_t idx) const -> TExpected<TJsonValue> {
        return HasValue() ? Value().At(idx) : Error();
    }
}
