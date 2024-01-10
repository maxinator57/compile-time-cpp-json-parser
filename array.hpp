#pragma once


#include "api.hpp"
#include "error.hpp"
#include "expected.hpp"
#include "iterator.hpp"
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

    class TJsonArray::Iterator : public TIteratorMixin<TJsonArray::Iterator> {
        using TBase = TIteratorMixin<TJsonArray::Iterator>;
    private:
        // MainIndex == CurElemStartPos
        std::string_view::size_type CurElemEndPos = 0;
        friend class TJsonArray;
        friend class TIteratorMixin<TJsonArray::Iterator>;
    private:
        constexpr auto StepForwardRemainingImpl() -> void {
            auto posOrErr = NUtils::FindCurElementEndPos(Data, LpCounter, MainIndex);
            if (posOrErr.HasError()) {
                SetError(std::move(posOrErr.Error()));
                return;
            }
            CurElemEndPos = posOrErr.Value();
        }
        constexpr auto GetStartPos() const -> std::string_view::size_type {
            return CurElemEndPos;
        }
        constexpr Iterator(
            std::string_view data,
            TLinePositionCounter lpCounter,
            std::string_view::size_type startPos
        ) : TBase(data, lpCounter, startPos) { Init(); }
    public:
        using difference_type = int;
        using value_type = TExpected<TJsonValue>;
    public:
        constexpr Iterator() : Iterator{{}, {}, std::string_view::npos} {};
        constexpr auto operator*() const -> value_type {
            if (ErrorOpt) return ErrorOpt.value();
            if (IsEnd()) return Error(LpCounter, NError::ErrorCode::IteratorDereferenceError);
            return TJsonValue{
                Data.substr(MainIndex, CurElemEndPos - MainIndex),
                LpCounter
            };
        } 
    };

    constexpr auto TJsonArray::begin() const -> Iterator { 
        return Iterator::Begin(Data, LpCounter);
    }

    constexpr auto TJsonArray::end() const -> Iterator {
        return Iterator::End(Data, LpCounter);
    }

    constexpr auto TJsonArray::At(size_t idx) const -> TExpected<TJsonValue> {
        auto it = begin(), finish = end();
        auto counter = size_t{0};
        for (; it != finish && counter < idx; ++it, ++counter);
        if (it.ErrorOpt) return it.ErrorOpt.value();
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
