#pragma once
#include <memory>
#include <optional>
#include <functional>
#include <random>
#include <algorithm>
#include <array>
#include <vector>
#include <iostream>

#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"

//-----------
//TO DO:
//	FIX OR CHANGE THE LIGHT TYPE
//	MAKE SURE TO REDUCE THE RADIUS TO SOMETHING AROUND 2(or increase the row size)
//  Rotate the piece to always look "outside", otherwise it looks weird
//  Maaaaaybe exaggerate the Projection matrix or in the shader to make the pieces look a bit curved
//-----------

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------
constexpr int BOARD_WIDTH  = 10;
constexpr int BOARD_HEIGHT = 22;   // 20 visible + 2 hidden spawn rows
constexpr int VISIBLE_HEIGHT = 20;

// -----------------------------------------------------------------------------
// Mino / Color
// -----------------------------------------------------------------------------
enum class Mino { Empty, I, O, T, S, Z, J, L, Ghost, Locked };

struct Color {
    unsigned char r, g, b;
};
constexpr std::array<Color, 9> MinoColors{{
    {0,0,0},       // Empty
    {0,255,255},   // I cyan
    {255,255,0},   // O yellow
    {255,0,255},   // T purple
    {0,255,0},     // S green
    {255,0,0},     // Z red
    {0,0,255},     // J blue
    {255,165,0},   // L orange
    {100,100,100}, // Ghost
}};

// -----------------------------------------------------------------------------
// Tetromino definitions (SRS)
// -----------------------------------------------------------------------------
using MinoRow = std::array<Mino, 4>;
using MinoMatrix = std::array<MinoRow, 4>;

struct TetrominoShape {
    std::array<MinoMatrix, 4> rotations;
};

// The 7 pieces in official order: I, O, T, S, Z, J, L
constexpr TetrominoShape Tetrominoes[7] = {
    // ─── I ───
    TetrominoShape{{
        MinoMatrix{{
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::I,     Mino::I,     Mino::I,     Mino::I},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::Empty, Mino::I, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::I, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::I, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::I, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::I,     Mino::I,     Mino::I,     Mino::I},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::I, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::I, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::I, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::I, Mino::Empty, Mino::Empty}
        }}
    }},

    // ─── O ─── (only one rotation)
    TetrominoShape{{
        MinoMatrix{{
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::O,     Mino::O,     Mino::Empty},
            {Mino::Empty, Mino::O,     Mino::O,     Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::O,     Mino::O,     Mino::Empty},
            {Mino::Empty, Mino::O,     Mino::O,     Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::O,     Mino::O,     Mino::Empty},
            {Mino::Empty, Mino::O,     Mino::O,     Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::O,     Mino::O,     Mino::Empty},
            {Mino::Empty, Mino::O,     Mino::O,     Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }}
    }},

    // ─── T ───
    TetrominoShape{{
        MinoMatrix{{
            {Mino::Empty, Mino::T, Mino::Empty, Mino::Empty},
            {Mino::T,     Mino::T, Mino::T,     Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::T, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::T, Mino::T, Mino::Empty},
            {Mino::Empty, Mino::T, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::T,     Mino::T, Mino::T,     Mino::Empty},
            {Mino::Empty, Mino::T, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::T, Mino::Empty, Mino::Empty},
            {Mino::T,     Mino::T, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::T, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }}
    }},

    // ─── S ───
    TetrominoShape{{
        MinoMatrix{{
            {Mino::Empty, Mino::S, Mino::S,     Mino::Empty},
            {Mino::S,     Mino::S, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::S, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::S, Mino::S, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::S, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::S, Mino::S,     Mino::Empty},
            {Mino::S,     Mino::S, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::S,     Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::S,     Mino::S, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::S, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }}
    }},

    // ─── Z ───
    TetrominoShape{{
        MinoMatrix{{
            {Mino::Z, Mino::Z,     Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Z, Mino::Z,     Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::Empty, Mino::Z, Mino::Empty},
            {Mino::Empty, Mino::S, Mino::Z, Mino::Empty},
            {Mino::Empty, Mino::Z, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        // (same pattern as S but mirrored – rest omitted for brevity, full below)
        // ... actually just copy the correct ones:
        MinoMatrix{{
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::Z,     Mino::Z,     Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Z, Mino::Z,     Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::Z, Mino::Empty, Mino::Empty},
            {Mino::Z,     Mino::Z, Mino::Empty, Mino::Empty},
            {Mino::Z,     Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }}
    }},

    // ─── J ───
    TetrominoShape{{
        MinoMatrix{{
            {Mino::J, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::J, Mino::J,     Mino::J,     Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::J, Mino::J, Mino::Empty},
            {Mino::Empty, Mino::J, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::J, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::J,     Mino::J,     Mino::J,     Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::J, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::J, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::J, Mino::Empty, Mino::Empty},
            {Mino::J,     Mino::J, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }}
    }},

    // ─── L ───
    TetrominoShape{{
        MinoMatrix{{
            {Mino::Empty, Mino::Empty, Mino::L, Mino::Empty},
            {Mino::L,     Mino::L,     Mino::L, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::L, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::L, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::L, Mino::L, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::L,     Mino::L,     Mino::L,     Mino::Empty},
            {Mino::L,     Mino::Empty, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }},
        MinoMatrix{{
            {Mino::L,     Mino::L, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::L, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::L, Mino::Empty, Mino::Empty},
            {Mino::Empty, Mino::Empty, Mino::Empty, Mino::Empty}
        }}
    }}
};


// -----------------------------------------------------------------------------
// Pieces class (current falling Pieces)
// -----------------------------------------------------------------------------
class Pieces {
public:
    Mino type;
    int x;          // pivot x (board columns)
    int y;          // pivot y (board rows, 0 = bottom)
    int rotation;   // 0-3

    Pieces(Mino t = Mino::Empty) : type(t), x(0), y(0), rotation(0) {}

    void spawn() {
        x = BOARD_WIDTH / 2 - 2;
        y = BOARD_HEIGHT - 2; // top of visible area
        rotation = 0;
    }

    // Get the 4x4 matrix for current rotation
    enum TetrominoType { I, O, T, S, Z, J, L };

	const MinoMatrix& matrix() const {
		int idx = 0;
		switch(type) {
			case Mino::I: idx = I; break;
			case Mino::O: idx = O; break;
			case Mino::T: idx = T; break;
			case Mino::S: idx = S; break;
			case Mino::Z: idx = Z; break;
			case Mino::J: idx = J; break;
			case Mino::L: idx = L; break;
			default: idx = 0;
		}
		return Tetrominoes[idx].rotations[rotation];
	}

    // Coordinates of the four minos (relative to board)
    std::vector<std::pair<int,int>> minoPositions() const {
        std::vector<std::pair<int,int>> pos;
        const auto& mat = matrix();
        for(int ry=0; ry<4; ++ry)
            for(int rx=0; rx<4; ++rx)
                if(mat[ry][rx] != Mino::Empty)
                    pos.emplace_back(x + rx, y - ry); // y grows upward
        return pos;
    }
};

// -----------------------------------------------------------------------------
// Board
// -----------------------------------------------------------------------------
class Board {
    std::array<std::array<Mino, BOARD_WIDTH>, BOARD_HEIGHT> cells{};

public:
    Board() { clear(); }

    void clear() {
        for(auto& row : cells)
            row.fill(Mino::Empty);
    }

//    Mino at(int x, int y) const {
//        if(x<0 || x>=BOARD_WIDTH || y<0 || y>=BOARD_HEIGHT) return Mino::Locked;
//        return cells[y][x];
//    }//
	Mino at(int x, int y) const {
		x = (x % BOARD_WIDTH + BOARD_WIDTH) % BOARD_WIDTH;  // Wrap X
		if (y < 0 || y >= BOARD_HEIGHT) return Mino::Locked;
		if (x < 0 || x >= BOARD_WIDTH) return Mino::Empty; // shouldn't happen
		return cells[y][x];
	}

    bool canPlace(const Pieces& p) const {
        for(auto [px,py] : p.minoPositions())
            if(at(px, py) != Mino::Empty) return false;
        return true;
    }

    void lockPiece(const Pieces& p) {
        for(auto [px,py] : p.minoPositions())
            if(py >= 0 && py < BOARD_HEIGHT)
                cells[py][px] = p.type;
    }

    int clearLines() {
        int lines = 0;
        for(int row = 0; row < BOARD_HEIGHT; ++row) {
            bool full = true;
            for(int col=0; col<BOARD_WIDTH; ++col)
                if(cells[row][col] == Mino::Empty) { full=false; break; }
            if(full) {
                ++lines;
                // shift down
                for(int r=row; r < BOARD_HEIGHT-1; ++r)
                    cells[r] = cells[r+1];
                cells[BOARD_HEIGHT-1].fill(Mino::Empty);
                --row; // recheck this row
            }
        }
        return lines;
    }

    Mino getCell(int x, int y) const { return (y>=0 && y<BOARD_HEIGHT) ? cells[y][x] : Mino::Empty; }
};

// -----------------------------------------------------------------------------
// Random bag
// -----------------------------------------------------------------------------
class Bag {
    std::vector<Mino> bag;
    std::mt19937 rng{std::random_device{}()};

    void refill() {
        bag = {Mino::I, Mino::O, Mino::T, Mino::S, Mino::Z, Mino::J, Mino::L};
        std::shuffle(bag.begin(), bag.end(), rng);
    }

public:
    Bag() { refill(); refill(); } // two bags to avoid early repeats

    Mino next() {
        if(bag.empty()) refill();
        Mino m = bag.back();
        bag.pop_back();
        return m;
    }
};

// -----------------------------------------------------------------------------
// Game class (core loop ready)
// -----------------------------------------------------------------------------
class TetrisGame {
public:
    Board board;
    Pieces current;
    std::optional<Pieces> hold;
    Mino holdType = Mino::Empty;
    bool canHold = true;
    Bag bag;

    int score = 0;
    int level = 1;
    int lines = 0;

    void newPiece() {
        current.type = bag.next();
        current.spawn();
        if(!board.canPlace(current)) {
            // Game Over
            std::cout << "Game Over!\n";
        }
        canHold = true;
    }

    void start() {
        board.clear();
        score = lines = level = 0;
        hold.reset();
        holdType = Mino::Empty;
        newPiece();
    }

    bool move(int dx, int dy) {
        current.x += dx;
        current.y += dy;
        if(!board.canPlace(current)) {
            current.x -= dx;
            current.y -= dy;
            return false;
        }
        return true;
    }

    bool rotate(bool clockwise = true) {
        int oldRot = current.rotation;
        current.rotation = (current.rotation + (clockwise?1:3)) % 4;

        // Simple wall kick attempts (full SRS table is big, this is "good enough" for a base)
        const std::array<std::pair<int,int>,5> kicks = {{
            {0,0}, {-1,0}, {1,0}, {0,-1}, {-1,-1}
        }};

        for(auto [kx,ky] : kicks) {
            current.x += kx;
            current.y += ky;
            if(board.canPlace(current)) return true;
            current.x -= kx;
            current.y -= ky;
        }

        current.rotation = oldRot; // revert
        return false;
    }

    void hardDrop() {
        while(move(0, -1));
        lockCurrent();
    }

    void lockCurrent() {
        board.lockPiece(current);
        lines += board.clearLines();
        newPiece();
    }

    void holdPiece() {
        if(!canHold) return;
        if(!hold) {
            hold = current;
            hold->type = current.type;
            newPiece();
        } else {
            std::swap(current.type, hold->type);
            current.spawn();
        }
        canHold = false;
    }
};