#pragma once


#include "api.hpp"
#include "error.hpp"
#include "expected.hpp"
#include "line_position_counter.hpp"
#include "utils.hpp"

#include <optional>
#include <string_view>


namespace NCompileTimeJsonParser { 
    class TGenericSerializedSequenceIterator {
    private:
        using TSelf = TGenericSerializedSequenceIterator;
    private:
        std::string_view Data; 
        TLinePositionCounter ElemBegLpCounter;
        TLinePositionCounter ElemEndLpCounter;
        std::string_view::size_type CurElemBegPos;
        std::string_view::size_type CurElemEndPos;
        std::optional<NError::TError> ErrorOpt;
    private:
        constexpr auto SetError(NError::TError&& err) -> void {
            CurElemBegPos = std::string_view::npos;
            ErrorOpt.emplace(std::move(err));
        }

    public:
        constexpr auto IsEnd() const -> bool {
            return CurElemBegPos == std::string_view::npos;
        }
        constexpr auto HasError() const -> bool {
            return ErrorOpt.has_value();
        }
        constexpr auto Error() const -> const NError::TError& {
            return ErrorOpt.value();
        }
        constexpr auto GetBegLpCounter() const -> const TLinePositionCounter& {
            return ElemBegLpCounter;
        }

        constexpr TGenericSerializedSequenceIterator(
            std::string_view data,
            TLinePositionCounter lpCounter,
            std::string_view::size_type startingPos,
            char delimiter
        )
            : Data(data)
            , ElemBegLpCounter(lpCounter)
        {
            CurElemBegPos = NUtils::FindFirstOf(
                Data, ElemBegLpCounter,
                [](char ch) { return !NUtils::IsSpace(ch); },
                startingPos
            );
            ElemEndLpCounter = ElemBegLpCounter;
            if (IsEnd()) {
                CurElemEndPos = std::string_view::npos;
                return;
            }
            auto nextPosOrErr = NUtils::FindCurElementEndPos(
                Data,
                ElemEndLpCounter,
                CurElemBegPos,
                delimiter
            );
            if (nextPosOrErr.HasError()) {
                SetError(std::move(nextPosOrErr.Error()));
                return;
            }
            CurElemEndPos = nextPosOrErr.Value();
        }

        static constexpr auto Begin(
            std::string_view data,
            TLinePositionCounter lpCounter,
            char delimiter
        ) -> TSelf { return {data, lpCounter, 0, delimiter}; }

        static constexpr auto End(
            std::string_view data, 
            TLinePositionCounter lpCounter
        ) -> TSelf { return {data, lpCounter, std::string_view::npos, {}}; }

        constexpr auto StepForward(char firstDelimiter, char secondDelimiter) -> TSelf& {
            if (IsEnd()) return *this;
            {
                auto nextPosOrErr = NUtils::FindNextElementStartPos(
                    Data,
                    ElemEndLpCounter,
                    CurElemEndPos,
                    firstDelimiter
                );
                ElemBegLpCounter = ElemEndLpCounter;
                if (nextPosOrErr.HasError()) {
                    SetError(std::move(nextPosOrErr.Error()));
                    return *this;
                }
                CurElemBegPos = nextPosOrErr.Value(); 
            }
            if (IsEnd()) return *this;
            {
                auto nextPosOrErr = NUtils::FindCurElementEndPos(
                    Data,
                    ElemEndLpCounter,
                    CurElemBegPos,
                    secondDelimiter
                );
                if (nextPosOrErr.HasError()) {
                    SetError(std::move(nextPosOrErr.Error()));
                    return *this;
                }
                CurElemEndPos = nextPosOrErr.Value();
            }
            return *this;
        }

        constexpr auto operator*() const -> TExpected<TJsonValue> {
            if (ErrorOpt) return ErrorOpt.value();
            if (IsEnd()) return NError::Error(
                ElemBegLpCounter,
                NError::ErrorCode::IteratorDereferenceError
            );
            return TJsonValue{
                Data.substr(CurElemBegPos, CurElemEndPos - CurElemBegPos),
                ElemBegLpCounter
            };
        }

        constexpr auto operator==(const TSelf& other) const -> bool {
            return Data == other.Data && CurElemBegPos == other.CurElemBegPos;
        }
    };
}