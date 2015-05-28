#include "Grid.h"

#include <cmath>
#include <cstdlib>

Grid::Grid(int width, int height, int cellSize) :
    m_width(width),
    m_height(height),
    m_cellSize(cellSize) {
    m_numXCells = (int)ceil((float)m_width / m_cellSize);
    m_numYCells = (int)ceil((float)m_height / m_cellSize);

    // Allocate all the cells
    const int BALLS_TO_RESERVE = 20;
    m_cells.resize(m_numYCells * m_numXCells);
    for (size_t i = 0; i < m_cells.size(); i++) {
        m_cells[i].balls.reserve(BALLS_TO_RESERVE);
    }
}

Grid::~Grid() {
}

void Grid::addBall(Ball* ball) {
    Cell* cell = getCell(ball->position);
    cell->balls.push_back(ball);
    ball->ownerCell = cell;
    ball->cellVectorIndex = cell->balls.size() - 1;
}

void Grid::addBall(Ball* ball, Cell* cell) {
    cell->balls.push_back(ball);
    ball->ownerCell = cell;
    ball->cellVectorIndex = cell->balls.size() - 1;
}

Cell* Grid::getCell(int x, int y) {
    if (x < 0) x = 0;
    if (x >= m_numXCells) x = m_numXCells - 1;
    if (y < 0) y = 0;
    if (y >= m_numYCells) y = m_numYCells - 1;

    return &m_cells[y * m_numXCells + x];
}

Cell* Grid::getCell(const glm::vec2& pos) {
    int cellX = (int)(pos.x / m_cellSize);
    int cellY = (int)(pos.y / m_cellSize);

    return getCell(cellX, cellY);
}

void Grid::removeBallFromCell(Ball* ball) {
    std::vector<Ball*>& balls = ball->ownerCell->balls;
    // Normal vector swap
    balls[ball->cellVectorIndex] = balls.back();
    balls.pop_back();
    // Update vector index
    if ((size_t)ball->cellVectorIndex < balls.size()) {
        balls[ball->cellVectorIndex]->cellVectorIndex = ball->cellVectorIndex;
    }
    // Set the index of ball to -1
    ball->cellVectorIndex = -1;
    ball->ownerCell = nullptr;
}
