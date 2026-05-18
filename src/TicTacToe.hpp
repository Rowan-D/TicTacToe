#ifndef TIC_TAC_TOE_TIC_TAC_TOE_HPP
#define TIC_TAC_TOE_TIC_TAC_TOE_HPP

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

constexpr int cellsToWin = 4;


namespace ttt {
    struct IVec2Hash {
        std::size_t operator()(const glm::ivec2& v) const noexcept {
            const auto x = static_cast<std::uint32_t>(v.x);
            const auto y = static_cast<std::uint32_t>(v.y);

            return (static_cast<std::size_t>(x) << 32)
                 ^ static_cast<std::size_t>(y);
        }
    };

    enum class CellState {
        Empty,
        X,
        O
    };

    class TicTacToe
    {
    public:
        TicTacToe() : wins() {}

        struct State {
            State(const CellState turn)
                : cells(), turn(turn) {}

            std::unordered_map<glm::ivec2, CellState, IVec2Hash> cells;
            CellState turn;
        };

        bool setCell(State& state, const glm::ivec2 cell, const CellState cellState);
        CellState getCell(const State& state, const glm::ivec2 cell) const;

        unsigned int raycast(const State& state, glm::ivec2 start, glm::ivec2 dir, unsigned int maxDist) const;

        glm::ivec2 findMove(const State& state, const CellState cellState) const;
        int evaluateMove(const State& state, const glm::ivec2 move) const;

        std::vector<glm::ivec2> wins;
    };
}

#endif