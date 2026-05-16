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
    class TicTacToe {
    public:
        TicTacToe();


        void setCell(const glm::ivec2 cell, const CellState state);
        CellState getCell(const glm::ivec2 cell) const;

        std::unordered_map<glm::ivec2, CellState, IVec2Hash> m_cells;
        std::vector<glm::ivec2> m_wins;
    };
}

#endif