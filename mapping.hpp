#pragma once


#include "api.hpp"
#include "error.hpp"
#include "expected.hpp"
#include "iterator.hpp"
#include "line_position_counter.hpp"
#include "utils.hpp"

#include <optional>
#include <sstream>
#include <stdexcept>
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

    class TJsonMapping::Iterator : public TIteratorMixin<TJsonMapping::Iterator> {
        using TBase = TIteratorMixin<TJsonMapping::Iterator>;
    private:
        // MainIndex == CurKeyStartPos
        std::string_view::size_type CurKeyEndPos = 0;
        std::string_view::size_type CurValueStartPos = 0;
        std::string_view::size_type CurValueEndPos = 0;

        friend class TJsonMapping;
        friend class TIteratorMixin<TJsonMapping::Iterator>;
    private:
        constexpr auto StepForwardRemainingImpl() -> void {
            {
                auto posOrErr = NUtils::FindCurElementEndPos(Data, LpCounter, MainIndex, ':');
                if (posOrErr.HasError()) {
                    SetError(std::move(posOrErr.Error()));
                    return;
                }
                CurKeyEndPos = posOrErr.Value();
                if (CurKeyEndPos == std::string_view::npos) {
                    SetError(Error(LpCounter, NError::ErrorCode::MissingValueError));
                    return;
                }
            }
            {
                auto posOrErr = NUtils::FindNextElementStartPos(Data, LpCounter, CurKeyEndPos, ':');
                if (posOrErr.HasError()) {
                    SetError(std::move(posOrErr.Error()));
                    return;
                }
                CurValueStartPos = posOrErr.Value();
                if (CurValueStartPos == std::string_view::npos) {
                    SetError(Error(LpCounter, NError::ErrorCode::MissingValueError));
                    return;
                }
            }
            {
                auto posOrErr = NUtils::FindCurElementEndPos(Data, LpCounter, CurValueStartPos);
                if (posOrErr.HasError()) {
                    SetError(std::move(posOrErr.Error()));
                    return;
                }
                CurValueEndPos = posOrErr.Value();
            }
        }
        constexpr auto GetStartPos() const -> std::string_view::size_type {
            return CurValueEndPos;
        }
        constexpr Iterator(
            std::string_view data,
            TLinePositionCounter lpCounter,
            std::string_view::size_type startPos
        ) : TBase(data, lpCounter, startPos) { Init(); }
    public:
        using difference_type = int;
        struct value_type {
            TExpected<String> Key;
            TExpected<TJsonValue> Value;
        };
    public:
        constexpr Iterator() : Iterator{{}, {}, std::string_view::npos} {};
        constexpr auto operator*() const -> value_type {
            if (ErrorOpt) {
                if (ErrorOpt->Code == NError::ErrorCode::MissingValueError) { 
                    return {
                        // TODO: calculate key position in text
                        .Key = TJsonValue{Data.substr(MainIndex, CurKeyEndPos - MainIndex)}.AsString(),
                        .Value = ErrorOpt.value()
                    };
                }
                return {.Key = "", .Value = ErrorOpt.value()};
            }
            if (IsEnd()) return {
                .Key = "",
                .Value = Error(LpCounter, NError::ErrorCode::IteratorDereferenceError)
            };
            return {
                .Key = TJsonValue{Data.substr(MainIndex, CurKeyEndPos - MainIndex)}.AsString(),
                .Value = TJsonValue{Data.substr(MainIndex, CurValueEndPos - CurValueStartPos)},
            };
        } 
    }; 

    constexpr auto TJsonMapping::begin() const -> Iterator {
        return Iterator::Begin(Data, LpCounter); 
    }

    constexpr auto TJsonMapping::end() const -> Iterator {
        return Iterator::End(Data, LpCounter); 
    }

    constexpr auto TJsonMapping::At(std::string_view key) const -> TExpected<TJsonValue> {
        auto finish = end();
        for (auto [k, v] : *this) if (key == k) return v;
        return Error(LpCounter, NError::ErrorCode::MappingKeyNotFound);
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
