#include "TicTacToe.hpp"
#include <iostream>
#include <unordered_map>

namespace ttt {
    inline constexpr std::array<glm::ivec2, 4> rays {{
        {-1,  0},
        { 0, -1},
        {-1, -1},
        {-1,  1}
    }};

    bool TicTacToe::setCell(TicTacToe::State& state, const glm::ivec2 cell, const CellState cellState) {
        if (getCell(state, cell) != CellState::Empty) { return false; }
        for (const glm::ivec2& ray : rays) {
            state.cells[cell] = cellState;
            unsigned int dist = raycast(state, cell, ray, cellsToWin);
            unsigned int opisiteDist = raycast(state, cell, -ray, cellsToWin);
            if (dist + opisiteDist + 1 >= cellsToWin) {
                wins.push_back(cell - (ray * static_cast<int>(opisiteDist)));
                wins.push_back(cell + (ray * static_cast<int>(dist)));
            }
        }
        return true;
    }

    CellState TicTacToe::getCell(const TicTacToe::State& state, const glm::ivec2 cell) const {
        auto it = state.cells.find(cell);
        if (it != state.cells.end()) {
            return it->second;
        }

        return CellState::Empty;
    }

    unsigned int TicTacToe::raycast(const TicTacToe::State& state, glm::ivec2 start, const glm::ivec2 dir, const unsigned int maxDist) const {
        unsigned int i = 0;
        const CellState cellState = getCell(state, start);
        while (i < maxDist && cellState == getCell(state, start + dir)) {
            start += dir;
            ++i;
        }
        return i;
    }
    
    glm::ivec2 TicTacToe::findMove(const TicTacToe::State& state, const CellState cellState) const {
        //std::unordered_map<glm::ivec2, int, IVec2Hash> moves;
        int best = -1;
        glm::ivec2 move(0);
        for (const auto& [cell, cellState] : state.cells) {
            for (const glm::ivec2& ray : rays) {
                if (getCell(state, cell + ray) == CellState::Empty) {
                    int score = evaluateMove(state, cell + ray);
                    if (score > best) {
                        best = score;
                        move = cell + ray;
                    }
                }
                if (getCell(state, cell - ray) == CellState::Empty) {
                    int score = evaluateMove(state, cell - ray);
                    if (score > best) {
                        best = score;
                        move = cell - ray;
                    }
                }
            }
        }
        return move;
    }

    int TicTacToe::evaluateMove(const TicTacToe::State& state, const glm::ivec2 move) const {
        int score = 0;
        for (const glm::ivec2& ray : rays) {
            if (getCell(state, move + ray) != CellState::Empty) {
                int rayDist = raycast(state, move + ray, ray, cellsToWin) + 1;
                score += rayDist * rayDist;
            }
            if (getCell(state, move - ray) != CellState::Empty) {
                int rayDist = raycast(state, move - ray, -ray, cellsToWin) + 1;
                score += rayDist * rayDist;
            }
        }
        return score;
    }
}