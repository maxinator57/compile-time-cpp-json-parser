#pragma once


#include "api.hpp"
#include "error.hpp"
#include "expected.hpp"
#include "line_position_counter.hpp"
#include "utils.hpp"

#include <optional>


namespace NJsonParser {
    class GenericSerializedSequenceIterator {
    private:
        using Self = GenericSerializedSequenceIterator;
    private:
        std::string_view Data = {};
        std::string_view::size_type CurElemBegPos = {};
        std::string_view::size_type CurElemEndPos = {}; 
        std::optional<NError::Error> ErrorOpt = {};
        LinePositionCounter ElemBegLpCounter = {};
        LinePositionCounter ElemEndLpCounter = {};
    private:
        constexpr auto SetError(const NError::Error& err) -> void {
            CurElemBegPos = std::string_view::npos;
            ErrorOpt.emplace(err);
        }
    public:
        constexpr auto IsEnd() const -> bool {
            return CurElemBegPos == std::string_view::npos;
        }
        constexpr auto HasError() const -> bool {
            return ErrorOpt.has_value();
        }
        constexpr auto Error() const -> const NError::Error& {
            return ErrorOpt.value();
        }
        constexpr auto GetBegLpCounter() const -> LinePositionCounter {
            return ElemBegLpCounter;
        }

        constexpr GenericSerializedSequenceIterator(
            std::string_view data,
            LinePositionCounter lpCounter,
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
                SetError(nextPosOrErr.Error());
                return;
            }
            CurElemEndPos = nextPosOrErr.Value();
        }

        constexpr GenericSerializedSequenceIterator(NError::Error err)
            : CurElemBegPos(std::string_view::npos), ErrorOpt(std::move(err)) {}

        static constexpr auto Begin(
            std::string_view data,
            LinePositionCounter lpCounter,
            char delimiter
        ) -> Self { return {data, lpCounter, 0, delimiter}; }

        static constexpr auto End(
            std::string_view data, 
            LinePositionCounter lpCounter
        ) -> Self { return {data, lpCounter, std::string_view::npos, {}}; }

        constexpr auto StepForward(char firstDelimiter, char secondDelimiter) -> Self& {
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
                    SetError(nextPosOrErr.Error());
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
                    SetError(nextPosOrErr.Error());
                    return *this;
                }
                CurElemEndPos = nextPosOrErr.Value();
            }
            return *this;
        }

        constexpr auto operator*() const -> Expected<JsonValue> {
            if (ErrorOpt) return ErrorOpt.value();
            if (IsEnd()) return NError::MakeError(
                ElemBegLpCounter,
                NError::ErrorCode::EndIteratorDereferenceError
            );
            return JsonValue{
                Data.substr(CurElemBegPos, CurElemEndPos - CurElemBegPos),
                ElemBegLpCounter
            };
        }

        constexpr auto operator==(const Self& other) const -> bool {
            return Data == other.Data && CurElemBegPos == other.CurElemBegPos;
        }
    };
}
