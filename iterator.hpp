#pragma once


#include "error.hpp"
#include "line_position_counter.hpp"
#include "utils.hpp"

#include <optional>
#include <string_view>


namespace NCompileTimeJsonParser {
    template <class Derived>
    class TIteratorMixin {
    protected:
        std::string_view Data; 
        TLinePositionCounter LpCounter;
        std::optional<NError::TError> ErrorOpt;
        std::string_view::size_type MainIndex;
    protected:
        constexpr auto IsEnd() const -> bool {
            return MainIndex == std::string_view::npos;
        }
        constexpr auto SetError(NError::TError&& err) -> void {
            MainIndex = std::string_view::npos;
            ErrorOpt.emplace(std::move(err));
        }
        constexpr auto StepForward(std::string_view::size_type startPos) -> void {
            auto posOrErr = NUtils::FindNextElementStartPos(Data, LpCounter, startPos);
            if (posOrErr.HasError()) {
                SetError(std::move(posOrErr.Error()));
                return;
            }
            MainIndex = posOrErr.Value();
            if (IsEnd()) return;
            // Note: needs to be implemented by Derived
            static_cast<Derived*>(this)->StepForwardRemainingImpl();
        }
        // To be called from `Derived` constructor after `TIteratorMixin` constructor
        constexpr auto Init() {
            if (IsEnd()) return;
            MainIndex = NUtils::FindFirstOf(
                Data, LpCounter,
                [](char ch) { return !NUtils::IsSpace(ch); },
                MainIndex
            );
            if (IsEnd()) return;
            // Note: needs to be implemented by Derived
            static_cast<Derived*>(this)->StepForwardRemainingImpl();
        }
        constexpr TIteratorMixin<Derived>(
            std::string_view data, 
            TLinePositionCounter lpCounter,
            std::string_view::size_type startPos = 0
        )
            : Data(data)
            , LpCounter(lpCounter)
            , MainIndex(startPos)
        {
        }
        static constexpr auto Begin(
            std::string_view data, 
            TLinePositionCounter lpCounter
        ) -> Derived { return Derived{data, lpCounter, 0}; }
        static constexpr auto End(
            std::string_view data, 
            TLinePositionCounter lpCounter
        ) -> Derived { return Derived{data, lpCounter, std::string_view::npos}; }
    public:
        constexpr auto operator++() -> Derived& {
            if (IsEnd()) return static_cast<Derived&>(*this);
            // Note: needs to be implemented by Derived
            const auto startPos = static_cast<Derived*>(this)->GetStartPos();
            if (startPos == std::string_view::npos) {
                MainIndex = std::string_view::npos;
            } else {
                StepForward(startPos);
            }
            return static_cast<Derived&>(*this);
        }
        constexpr auto operator++(int) -> Derived {
            auto copy = static_cast<Derived&>(*this);
            ++(*this);
            return copy;
        }
        constexpr auto operator==(const TIteratorMixin<Derived>& other) const -> bool {
            return Data == other.Data && MainIndex == other.MainIndex;
        }
    };
}