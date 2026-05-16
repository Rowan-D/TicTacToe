#include "TicTacToe.hpp"

namespace ttt {
    TicTacToe::TicTacToe() {
    }

    void TicTacToe::setCell(const glm::ivec2 cell, const CellState state) {
        if (getCell(cell) != CellState::Empty) { return; }
        m_cells[cell] = state;
        // right
        for (int i = 1; i < cellsToWin; ++i) {
            if (getCell(cell + glm::ivec2(i, 0)) != state) { break; }
            if (i == cellsToWin - 1)
            {
                m_wins.push_back(cell);
                m_wins.push_back(cell + glm::ivec2(i, 0));
            }
        }
        // down
        for (int i = 1; i < cellsToWin; ++i) {
            if (getCell(cell + glm::ivec2(0, i)) != state) { break; }
            if (i == cellsToWin - 1)
            {
                m_wins.push_back(cell);
                m_wins.push_back(cell + glm::ivec2(0, i));
            }
        }
        // down right
        for (int i = 1; i < cellsToWin; ++i) {
            if (getCell(cell + glm::ivec2(i, i)) != state) { break; }
            if (i == cellsToWin - 1)
            {
                m_wins.push_back(cell);
                m_wins.push_back(cell + glm::ivec2(i, i));
            }
        }
        // down left
        for (int i = 1; i < cellsToWin; ++i) {
            if (getCell(cell + glm::ivec2(-i, i)) != state) { break; }
            if (i == cellsToWin - 1)
            {
                m_wins.push_back(cell);
                m_wins.push_back(cell + glm::ivec2(-i, i));
            }
        }
        // left
        for (int i = 1; i < cellsToWin; ++i) {
            if (getCell(cell + glm::ivec2(-i, 0)) != state) { break; }
            if (i == cellsToWin - 1)
            {
                m_wins.push_back(cell);
                m_wins.push_back(cell + glm::ivec2(-i, 0));
            }
        }
        // up
        for (int i = 1; i < cellsToWin; ++i) {
            if (getCell(cell + glm::ivec2(0, -i)) != state) { break; }
            if (i == cellsToWin - 1)
            {
                m_wins.push_back(cell);
                m_wins.push_back(cell + glm::ivec2(0, -i));
            }
        }
        // up right
        for (int i = 1; i < cellsToWin; ++i) {
            if (getCell(cell + glm::ivec2(i, -i)) != state) { break; }
            if (i == cellsToWin - 1)
            {
                m_wins.push_back(cell);
                m_wins.push_back(cell + glm::ivec2(i, -i));
            }
        }
        // up left
        for (int i = 1; i < cellsToWin; ++i) {
            if (getCell(cell + glm::ivec2(-i, -i)) != state) { break; }
            if (i == cellsToWin - 1)
            {
                m_wins.push_back(cell);
                m_wins.push_back(cell + glm::ivec2(-i, -i));
            }
        }
    }

    CellState TicTacToe::getCell(const glm::ivec2 cell) const {
        auto it = m_cells.find(cell);
        if (it != m_cells.end()) {
            return it->second;
        }

        return CellState::Empty;
    }
}