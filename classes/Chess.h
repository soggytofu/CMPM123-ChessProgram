#pragma once
#include "Game.h"
#include "ChessSquare.h"
#include <cstdint>
// #include "magicbitboards.h"

const int pieceSize = 64;

enum ChessPiece {
    NoPiece = 0,
    Pawn = 1,
    Knight = 2,
    Bishop = 3,
    Rook = 4,
    Queen = 5,
    King = 6
};

//
// the main game class
//
class Chess : public Game
{
public:
    Chess();
    ~Chess();

    // set up the board
    void        setUpBoard() override;
    void        loadFromFEN(const std::string &s);

    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void        setStateString(const std::string &s) override;
    // bool        mouseDown(ImVec2 &location, Entity *entity) override;
    bool        actionForEmptyHolder(BitHolder& holder) override;
    bool        canBitMoveFrom(Bit& bit, BitHolder& src) override;
    bool        canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;
    void        bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;

    void        stopGame() override;
    BitHolder& getHolderAt(const int x, const int y) override { return _grid[y][x]; }

    void        updateAI() override;
    bool        gameHasAI() override { return true; }
private:
    Bit *       PieceForPlayer(const int playerNumber, ChessPiece piece);
    const char  bitToPieceNotation(int row, int column) const;

    ChessSquare      _grid[8][8];
    std::vector<uint64_t> possibleMoves = std::vector<uint64_t>(64, 0ULL);
    uint64_t wPieces;
    uint64_t bPieces;
    int     enPessantHolder;
    bool    whiteRookLeft;
    bool    whiteRookRight;
    bool    whiteKing;
    bool    blackRookLeft;
    bool    blackRookRight;
    bool    blackKing;
    void        generateMoveList();
    uint64_t ratt(int sq, uint64_t wPieces, uint64_t bPieces); 
    uint64_t batt(int sq, uint64_t wPieces, uint64_t bPieces); 
    uint64_t katt(int sq, uint64_t wPieces, uint64_t bPieces); 
    uint64_t natt(int sq, uint64_t wPieces, uint64_t bPieces); 
    uint64_t patt(int sq, uint64_t wPieces, uint64_t bPieces); 
    void        highlightMove(uint64_t moveBitboard);
    void        removeHighlight();
    void        updatePieces(uint64_t& pieces, int check);

    //Here is where I should put my bitboards?
    //White pieces only
    //Black pieces only
};