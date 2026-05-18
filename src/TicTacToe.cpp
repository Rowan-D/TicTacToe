#include "TicTacToe.hpp"
#include <iostream>

namespace ttt {
    TicTacToe::TicTacToe()
        : m_cells(), m_wins() {
    }

    inline constexpr std::array<glm::ivec2, 4> rays {{
        {-1,  0},
        { 0, -1},
        {-1, -1},
        {-1,  1}
    }};

    bool TicTacToe::setCell(const glm::ivec2 cell, const CellState state) {
        if (getCell(cell) != CellState::Empty) { return false; }
        for (const glm::ivec2& ray : rays) {
            m_cells[cell] = state;
            unsigned int dist = raycast(cell, ray, cellsToWin);
            unsigned int opisiteDist = raycast(cell, -ray, cellsToWin);
            if (dist + opisiteDist + 1 >= cellsToWin) {
                m_wins.push_back(cell - (ray * static_cast<int>(opisiteDist)));
                m_wins.push_back(cell + (ray * static_cast<int>(dist)));
            }
        }
        return true;
    }

    CellState TicTacToe::getCell(const glm::ivec2 cell) const {
        auto it = m_cells.find(cell);
        if (it != m_cells.end()) {
            return it->second;
        }

        return CellState::Empty;
    }

    unsigned int TicTacToe::raycast(glm::ivec2 start, const glm::ivec2 dir, const unsigned int maxDist) const {
        unsigned int i = 0;
        const CellState state = getCell(start);
        while (i < maxDist && state == getCell(start + dir)) {
            start += dir;
            ++i;
        }
        return i;
    }
}