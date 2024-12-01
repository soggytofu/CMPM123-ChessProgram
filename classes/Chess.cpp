#include "Chess.h"
#include <cstdint>
#include <cmath>

const int AI_PLAYER = 1;
const int HUMAN_PLAYER = -1;

Chess::Chess()
{
}

Chess::~Chess()
{
}

//
// make a chess piece for the player
//
Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    // depending on playerNumber load the "x.png" or the "o.png" graphic
    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("chess/") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;
    //
    // we want white to be at the bottom of the screen so we need to reverse the board
    //
    char piece[2];
    piece[1] = 0;
    for(int y = 0; y < _gameOptions.rowY; y++) {
        for(int x = 0; x < _gameOptions.rowX; x++) {
            ImVec2 position((float)(pieceSize * x + pieceSize), (float)(pieceSize * (_gameOptions.rowY - y) + pieceSize));
            _grid[y][x].initHolder(position, "boardsquare.png", x, y);
            piece[0] = bitToPieceNotation(y,x);
            _grid[y][x].setNotation(piece);
        }
    }
    wPieces = 0;
    bPieces = 0;

    // loadFromFEN("RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr");
    // loadFromFEN("K7/PPPPPP2/8/8/8/8/2pppppp/7k");
    loadFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    // loadFromFEN("3k4/8/8/8/8/8/8/R3K3");

    startGame();
    generateMoveList();
}


    // helper function that FEN notation to fill board
void Chess::loadFromFEN(const std::string &fen){
    static const std::unordered_map<char, ChessPiece> pieceMap = {
        {'p', Pawn}, {'r', Rook}, {'n', Knight}, {'b', Bishop},
        {'q', Queen}, {'k', King}
    };
    int posCount = 0;  // Tracks position on the board (0 to 63)
    for (char c : fen) {
        if (c == '/') {
            continue;  // Skip rank separators
        } else if (isdigit(c)) {
            posCount += c - '0';  // Empty squares
        } else {
            bool isWhite = isupper(c);  // Determine piece color
            char lowerC = tolower(c);  // Normalize to lowercase for map lookup
            auto piece = pieceMap.find(lowerC);

            if (piece != pieceMap.end()) {
                int row = 7 - (posCount / 8);  // Convert to top-left starting board position
                int col = posCount % 8;

                Bit* bit = PieceForPlayer(isWhite ? 0 : 1, piece->second);
                bit->setPosition(_grid[row][col].getPosition());
                bit->setParent(&_grid[row][col]);
                bit->setGameTag(isWhite ? piece->second : piece->second + 128);
                _grid[row][col].setGameTag(isWhite ? piece->second : piece->second + 128);
                _grid[row][col].setBit(bit);
                (isWhite ? wPieces : bPieces) |= (1ULL << (row * 8 + col));
            }
            posCount++;
        }
    }
    whiteRookLeft = true;
    whiteRookRight = true;
    whiteKing = true;
    blackRookLeft = true;
    blackRookRight = true;
    blackKing = true;
}


//For future use if i need to yoink in the last part of a fen string
// void Chess::loadFromFEN(const std::string &fen) {
//     static const std::unordered_map<char, ChessPiece> pieceMap = {
//         {'p', Pawn}, {'r', Rook}, {'n', Knight}, {'b', Bishop},
//         {'q', Queen}, {'k', King}
//     };

//     // Split the FEN string into parts
//     std::istringstream fenStream(fen);
//     std::string boardPart, turnPart, castlingPart, enPassantPart;
//     int halfmoveClock, fullmoveNumber;

//     // Read the FEN parts
//     fenStream >> boardPart >> turnPart >> castlingPart >> enPassantPart >> halfmoveClock >> fullmoveNumber;

//     // Clear the board before parsing
//     // clearBoard();

//     // Parse the board part
//     int posCount = 0;  // Tracks position on the board (0 to 63)
//     for (char c : boardPart) {
//         if (c == '/') {
//             continue;  // Skip rank separators
//         } else if (isdigit(c)) {
//             posCount += c - '0';  // Empty squares
//         } else {
//             bool isWhite = isupper(c);  // Determine piece color
//             char lowerC = tolower(c);  // Normalize to lowercase for map lookup
//             auto piece = pieceMap.find(lowerC);

//             if (piece != pieceMap.end()) {
//                 int row = 7 - (posCount / 8);  // Convert to top-left starting board position
//                 int col = posCount % 8;

//                 Bit* bit = PieceForPlayer(isWhite ? 0 : 1, piece->second);
//                 bit->setPosition(_grid[row][col].getPosition());
//                 bit->setParent(&_grid[row][col]);
//                 bit->setGameTag(isWhite ? piece->second : piece->second + 128);
//                 _grid[row][col].setGameTag(isWhite ? piece->second : piece->second + 128);
//                 _grid[row][col].setBit(bit);
//                 (isWhite ? wPieces : bPieces) |= (1ULL << (row * 8 + col));
//             }
//             posCount++;
//         }
//     }

//     // // Set starting player
//     // // startingPlayer(turnPart == "w" ? 0 : 1);

//     // whiteRookRight = castlingPart.find('K') != std::string::npos;
//     // whiteRookLeft = castlingPart.find('Q') != std::string::npos;
//     // whiteKing = (whiteRookRight || whiteRookLeft) ? true : false;

//     // blackRookRight = castlingPart.find('k') != std::string::npos;
//     // blackRookLeft = castlingPart.find('q') != std::string::npos;
//     // blackKing = (blackRookRight || blackRookLeft) ? true : false;

//     // // Parse en passant
//     // if (enPassantPart != "-") {
//     //     int file = enPassantPart[0] - 'a';  // File (column) from 'a' to 'h'
//     //     int rank = enPassantPart[1] - '1';  // Rank (row) from '1' to '8'
//     //     enPessantHolder = (7 - rank) * 8 + file;  // Convert to board index (top-left to bottom-left)
//     // } else {
//     //     enPessantHolder = -1;  // No en passant square
//     // }

//     // // Store halfmove clock and fullmove number
//     // // this->halfmoveClock = halfmoveClock;
//     // // this->fullmoveNumber = fullmoveNumber;
// }

//
// about the only thing we need to actually fill out for tic-tac-toe
//
bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    //Check if in check
    return true;
}

// bool Chess::mouseDown(ImVec2 &location, Entity *entity){
    
// }

bool Chess::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) { // Piece, Position, go to Position
    ChessSquare* srcSquare = dynamic_cast<ChessSquare*>(&src);  //Grab location of start
    ChessSquare* dstSquare = dynamic_cast<ChessSquare*>(&dst);  //Grab location of end
    int total = srcSquare->getSquareIndex(); 
    int total2 = dstSquare->getSquareIndex();  //Everything above just turns it into x, y, and square space

    int player = getCurrentPlayer()->playerNumber();
    uint64_t cPieces = player == 0 ? wPieces : bPieces;  //Check pieces for which can be selected 
    bool isPlayable = (cPieces & (1ULL << total)) != 0;  //Is the selected piece apart of the player's pieces

    if(possibleMoves[total] != 0 && isPlayable){
        highlightMove(possibleMoves[total]);
        return (possibleMoves[total] & (1ULL << total2)) != 0;
    }
    return false;

}

void Chess::highlightMove(uint64_t moveBitboard){
    for (int i = 0; i < 64; i++){
        if (moveBitboard & (1ULL << i)) {
            int rk = i / 8, fl = i % 8;
            _grid[rk][fl].setMoveHighlighted(true);
        }
    }
}

void Chess::removeHighlight(){
    for (int i = 0; i < 64; i++){
        int rk = i / 8, fl = i % 8;
        _grid[rk][fl].setMoveHighlighted(false);
    }
}

/*
Instead of current code, create a moves list using all the moves,
then, go throughout the list isntead of checking one by one for more efficency
and to highlight

Each space needs to have this:
Given square
-every piece it can move
-check if it check


Use this void ChessSquare::setMoveHighlighted(bool highlighted)
If you click off, turn off highlight

Make a game state cpp file from 11/8, proffessor just posted it
also create a bitboard.h from the progessors code

So if you have a game list, then when making a new move, you 
can check for enpessant

put the moves into gamestate.cpp i think


*/

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) { // Post Check piece, old position, new position
    bool isBlack = (bit.gameTag() > 128) ? true : false;
    int pieceTag = (bit.gameTag() > 128) ? bit.gameTag() - 128 : bit.gameTag();  //If black, remove 128 from gameTag
    ChessSquare* srcSquare = dynamic_cast<ChessSquare*>(&src);
    ChessSquare* dstSquare = dynamic_cast<ChessSquare*>(&dst);
    int x = srcSquare->getColumn();
    int y = srcSquare->getRow();
    int xy = srcSquare->getSquareIndex();
    int x2 = dstSquare->getColumn();
    int y2 = dstSquare->getRow();
    int xy2 = dstSquare->getSquareIndex();

    _grid[y][x].setGameTag(0);  //Removes old piece that grid was holding
    _grid[y2][x2].setGameTag(bit.gameTag());  //Updates the space's tag of what piece its holding

    if(pieceTag != Pawn){
        enPessantHolder = -1;
    }
    if(pieceTag == Pawn){
        if(xy2 == enPessantHolder + 8){  //White En Pessant
            _grid[y2-1][x2].destroyBit();
            _grid[y2-1][x2].setGameTag(0);
        }
        if(xy2 == enPessantHolder - 8){  //Black En Pessant
            _grid[y2+1][x2].destroyBit();
            _grid[y2+1][x2].setGameTag(0);
        }
        int epCheck = std::abs(y - y2);
        if(epCheck == 2){
            enPessantHolder = xy2;
        }
        if(y2 == 7){  //White pawn promotion to queen automatically
            Bit* bit = PieceForPlayer(0, Queen); 
            bit->setPosition(_grid[y2][x2].getPosition());  //Sets bit position
            bit->setParent(&_grid[y2][x2]);  //Sets the grid parent, idk really
            bit->setGameTag(5);  //What piece is it and color
            _grid[y2][x2].setGameTag(5);  //Grid gets to hold onto piece and color
            _grid[y2][x2].setBit(bit);  //Set the bit in palce too
        }
        if(y2 == 0){  //Black pawn promotion to queen automatically
            Bit* bit2 = PieceForPlayer(1, Queen); 
            bit2->setPosition(_grid[y2][x2].getPosition());  //Sets bit position
            bit2->setParent(&_grid[y2][x2]);  //Sets the grid parent, idk really
            bit2->setGameTag(133);  //What piece is it and color
            _grid[y2][x2].setGameTag(133);  //Grid gets to hold onto piece and color
            _grid[y2][x2].setBit(bit2);  //Set the bit in palce too
        }
    }
    if(pieceTag == Rook){
        whiteRookLeft = (xy == 0) ? false : whiteRookLeft;
        whiteRookRight = (xy == 7) ? false : whiteRookRight;
        blackRookLeft = (xy == 56) ? false : blackRookLeft;
        blackRookRight = (xy == 63) ? false : blackRookRight;
    }
    if(pieceTag == King){
        if(xy2 == 2 && whiteKing){
            whiteRookLeft = false;
            whiteRookRight = false;
            _grid[0][0].destroyBit();
            _grid[0][0].setGameTag(0);
            Bit* bit = PieceForPlayer(0, Rook);  //Me lazy, me copy paste old code to delete rook and make new one XD
            bit->setPosition(_grid[0][3].getPosition()); 
            bit->setParent(&_grid[0][3]); 
            bit->setGameTag(4); 
            _grid[0][3].setGameTag(4);  
            _grid[0][3].setBit(bit);  
        }
        if(xy2 == 6 && whiteKing){
            whiteRookLeft = false;
            whiteRookRight = false;
            _grid[0][7].destroyBit();
            _grid[0][7].setGameTag(0);
            Bit* bit = PieceForPlayer(0, Rook);  //Me lazy, me copy paste old code to delete rook and make new one XD
            bit->setPosition(_grid[0][5].getPosition()); 
            bit->setParent(&_grid[0][5]); 
            bit->setGameTag(2); 
            _grid[0][5].setGameTag(2);  
            _grid[0][5].setBit(bit);  
        }
        if(xy2 == 58 && blackKing){
            blackRookLeft = false;
            blackRookRight = false;
            _grid[7][0].destroyBit();
            _grid[7][0].setGameTag(0);
            Bit* bit = PieceForPlayer(1, Rook);  //Me lazy, me copy paste old code to delete rook and make new one XD
            bit->setPosition(_grid[7][3].getPosition()); 
            bit->setParent(&_grid[7][3]); 
            bit->setGameTag(130); 
            _grid[7][3].setGameTag(132);  
            _grid[7][3].setBit(bit);  
        }
        if(xy2 == 62 && blackKing){
            blackRookLeft = false;
            blackRookRight = false;
            _grid[7][7].destroyBit();
            _grid[7][7].setGameTag(0);
            Bit* bit = PieceForPlayer(1, Rook);  //Me lazy, me copy paste old code to delete rook and make new one XD
            bit->setPosition(_grid[7][5].getPosition()); 
            bit->setParent(&_grid[7][5]); 
            bit->setGameTag(130); 
            _grid[7][5].setGameTag(132);  
            _grid[7][5].setBit(bit);  
        }
        isBlack ? blackKing = false : whiteKing = false;
    }
    
    removeHighlight();
    updatePieces(wPieces, 0);  //Update our bitbaords of white and black pieces
    updatePieces(bPieces, 1);
    generateMoveList();

    //Eventually add more functionality

    endTurn();
}

//
// free all the memory used by the game on the heap
//
void Chess::stopGame()
{
}


//
// Generate a list of all possible moves, in this list it will have this
// (Square, (Space it can move, special clause?))
//
void Chess::generateMoveList(){
    for(int i = 0; i < 64; i++){
        uint64_t result = 0ULL;
        int rk = i / 8, fl = i % 8;
        int whatPiece = _grid[rk][fl].gameTag();
        bool isBlack = (whatPiece > 128) ? true : false;
        uint64_t block = wPieces | bPieces;
        uint64_t blockPieces = isBlack ? wPieces : bPieces; 
        whatPiece = isBlack ? whatPiece - 128 : whatPiece;
        if(whatPiece == 0){
            result = 0ULL;
            continue;
        }
        else if(whatPiece == Rook){
            result = ratt(i, wPieces, bPieces);
        }
        else if(whatPiece == Bishop){
            result = batt(i, wPieces, bPieces);
        }
        else if(whatPiece == Queen ){
            result = ratt(i, wPieces, bPieces);
            result |= batt(i, wPieces, bPieces);
        }
        else if(whatPiece == Pawn){
            result = patt(i, wPieces, bPieces);
        }
        else if(whatPiece == Knight){
            result = natt(i, wPieces, bPieces);
        }
        else if(whatPiece == King){
            result = katt(i, wPieces, bPieces);
        }
        possibleMoves[i] = result;
    }
}


uint64_t Chess::ratt(int sq, uint64_t wPieces, uint64_t bPieces){  //Rook attack
    uint64_t result = 0ULL;
    uint64_t block = wPieces | bPieces;
    int rk = sq / 8, fl = sq % 8, r, f;
    int whatPiece = _grid[rk][fl].gameTag();  //Gets what side and piece
    bool isBlack = (whatPiece > 128) ? true : false;  //Finalizes what side, checks if black
    uint64_t blockPieces = isBlack ? wPieces : bPieces;  //Gets the oposite team to check for captures

    //North
    for(r = rk + 1; r <= 7; r++) {  //For moves along line,
        if(block & (1ULL << (fl + r * 8))) {  //Checks if there is a blocking piece 
            if(blockPieces & (1ULL << (fl + r * 8))) {  //If its on enemy team >
                result |= (1ULL << (fl + r * 8));  //Add capture move
            }
            break;  //Else get blocked and leave
        }
        result |= (1ULL << (fl + r * 8));  //No block, just add
    }

    //South
    for(r = rk - 1; r >= 0; r--){  //Same stuff as above, but different for loop
        if(block & (1ULL << (fl + r * 8))){  
            if(blockPieces & (1ULL << (fl + r * 8))){ 
                result |= (1ULL << (fl + r * 8)); 
            }
            break; 
        }
        result |= (1ULL << (fl + r * 8));  
    }

    //East
    for(f = fl + 1; f <= 7; f++) {  //Same stuff as above, but different for loop
        if(block & (1ULL << (f + rk * 8))){  
            if(blockPieces & (1ULL << (f + rk * 8))){  
                result |= (1ULL << (f + rk * 8)); 
            }
            break;  
        }
        result |= (1ULL << (f + rk * 8)); 
    }

    //West
    for(f = fl - 1; f >= 0; f--){  //Same stuff as above, but different for loop
        if(block & (1ULL << (f + rk * 8))){ 
            if(blockPieces & (1ULL << (f + rk * 8))){ 
                result |= (1ULL << (f + rk * 8)); 
            }
            break; 
        }
        result |= (1ULL << (f + rk * 8)); 
    }

    return result;
}

uint64_t Chess::batt(int sq, uint64_t wPieces, uint64_t bPieces){  //Bishop attack
    uint64_t result = 0ULL;  //This is the same as the rook attack but diagonal
    uint64_t block = wPieces | bPieces;
    int rk = sq / 8, fl = sq % 8, r, f;
    int whatPiece = _grid[rk][fl].gameTag();
    bool isBlack = (whatPiece > 128) ? true : false;
    uint64_t blockPieces = isBlack ? wPieces : bPieces; 

    //Northeast
    for(r = rk + 1, f = fl + 1; r <= 7 && f <= 7; r++, f++){  //Check Rook Comments to understand, same thing
        if(block & (1ULL << (f + r * 8))) {  
            if(blockPieces & (1ULL << (f + r * 8))){  
                result |= (1ULL << (f + r * 8));  
            }
            break;  
        }
        result |= (1ULL << (f + r * 8)); 
    }

    //Southeast
    for(r = rk - 1, f = fl + 1; r >= 0 && f <= 7; r--, f++){  //Check Rook Comments to understand
        if(block & (1ULL << (f + r * 8))) {  
            if(blockPieces & (1ULL << (f + r * 8))){  
                result |= (1ULL << (f + r * 8));  
            }
            break; 
        }
        result |= (1ULL << (f + r * 8));  
    }

    //Southwest
    for(r = rk - 1, f = fl - 1; r >= 0 && f >= 0; r--, f--){  //Check Rook Comments to understand
        if(block & (1ULL << (f + r * 8))) {  
            if(blockPieces & (1ULL << (f + r * 8))){  
                result |= (1ULL << (f + r * 8));  
            }
            break; 
        }
        result |= (1ULL << (f + r * 8)); 
    }

    //Northwest
    for(r = rk + 1, f = fl - 1; r <= 7 && f >= 0; r++, f--){  //Check Rook Comments to understand
        if(block & (1ULL << (f + r * 8))) {  
            if(blockPieces & (1ULL << (f + r * 8))){  
                result |= (1ULL << (f + r * 8)); 
            }
            break;  
        }
        result |= (1ULL << (f + r * 8));  
    }

    return result;
}

uint64_t Chess::patt(int sq, uint64_t wPieces, uint64_t bPieces) {  //Pawn attack
    uint64_t result = 0ULL;
    int rk = sq / 8, fl = sq % 8;
    int whatPiece = _grid[rk][fl].gameTag();
    bool isBlack = (whatPiece > 128) ? true : false;
    uint64_t block = wPieces | bPieces;
    uint64_t blockPieces = isBlack ? wPieces : bPieces; 

    if(!isBlack){  //For white
        if(rk == 1 && !(block & (1ULL << (sq + 8))) && !(block & (1ULL << (sq + 16)))){  //Still on first space
            result |= (1ULL << (sq + 16));
        }
        if(_grid[rk+1][fl].gameTag() <= 0){  //Regular move forward
            result |= (1ULL << (sq + 8));
        }
        if((block & (1ULL << (sq + 7)))){  //Capture left
            if(blockPieces & (1ULL << (sq + 7))) { 
                result |= (1ULL << (sq + 7)); 
            }
        }
        if(enPessantHolder == (sq - 1)){  //En Pessant left
            result |= (1ULL << (sq + 7)); 
        }
        if((block & (1ULL << (sq + 9)))){  //Capture right
            if(blockPieces & (1ULL << (sq + 9))) { 
                result |= (1ULL << (sq + 9)); 
            }
        }
        if(enPessantHolder == (sq + 1)){  //En Pessant right
            result |= (1ULL << (sq + 9)); 
        }
    }
    if(isBlack){
        if(rk == 6 && !(block & (1ULL << (sq - 8))) && !(block & (1ULL << (sq - 16)))){  //Still on first space
            result |= (1ULL << (sq - 16));
        }
        if(_grid[rk-1][fl].gameTag() <= 0){  //Regular move forward
            result |= (1ULL << (sq - 8));
        }
        if((block & (1ULL << (sq - 7)))){  //Capture left
            if(blockPieces & (1ULL << (sq - 7))){ 
                result |= (1ULL << (sq - 7)); 
            }
        }
        if(enPessantHolder == (sq - 1)){  //En Pessant left
            result |= (1ULL << (sq - 9)); 
        }
        if((block & (1ULL << (sq - 9)))){  //Capture right
            if(blockPieces & (1ULL << (sq - 9))){ 
                result |= (1ULL << (sq - 9)); 
            }
        }
        if(enPessantHolder == (sq + 1)){  //En Pessant right
            result |= (1ULL << (sq - 7)); 
        }
    }


    return result;
    
}

uint64_t Chess::natt(int sq, uint64_t wPieces, uint64_t bPieces) {  //Knight attack
    uint64_t result = 0ULL;
    int whatPiece = _grid[sq/8][sq%8].gameTag();
    bool isBlack = (whatPiece > 128) ? true : false;
    uint64_t blockPieces = isBlack ? bPieces : wPieces;
    int knightMoves[8] = {17, 15, 10, 6, -6, -10, -15, -17};  //Knight moves

    for(int move : knightMoves){
        int targetSq = sq + move;
        if(targetSq < 0 || targetSq > 63) continue;  //Don't overshoot the board

        int sqFile = sq % 8;
        int targetFile = targetSq % 8;
        if(abs(targetFile - sqFile) > 2) continue;  //Ignore wraping moves

        uint64_t targetPos = 1ULL << targetSq;
        result |= (!(blockPieces & targetPos)) ? targetPos : 0ULL;  //Add to result if not blocked
    }
    
    return result;
}

uint64_t Chess::katt(int sq, uint64_t wPieces, uint64_t bPieces){
    uint64_t result = 0ULL;
    uint64_t block = wPieces | bPieces;
    int whatPiece = _grid[sq/8][sq%8].gameTag();
    bool isBlack = (whatPiece > 128) ? ((whatPiece -= 128), true) : false; 
    uint64_t blockPieces = isBlack ? bPieces : wPieces;
    int kingMoves[8] = {7, 8, 9, -1, 1, -9, -8, -7};  //King moves

    for(int move : kingMoves){
        int targetSq = sq + move;
        if(targetSq < 0 || targetSq > 63) continue;
        if((sq % 8 == 0 && (move == -1 || move == -9 || move == 7)) ||  //Edge cases if king is left or right
            (sq % 8 == 7 && (move == 1 || move == -7 || move == 9))) {   
            continue;
        }
        uint64_t targetPos = 1ULL << targetSq;
        result |= (blockPieces & targetPos) ? 0ULL : targetPos;
    }
    if(isBlack && blackKing){  //Castling for black king
        if(blackRookRight && _grid[7][5].gameTag() == 0 && _grid[7][6].gameTag() == 0){ 
            result |= (1ULL << 62);  //King castling
        }
        if(blackRookLeft && _grid[7][1].gameTag() == 0 && _grid[7][2].gameTag() == 0 && _grid[7][3].gameTag() == 0){
            result |= (1ULL << 58);  //Queen castling
        }
    }
    if(!isBlack && whiteKing){  //Castling for white king
        if(whiteRookRight && _grid[0][5].gameTag() == 0 && _grid[0][6].gameTag() == 0){
            result |= (1ULL << 6);  // King castling
        }
        if(whiteRookLeft && _grid[0][1].gameTag() == 0 && _grid[0][2].gameTag() == 0 && _grid[0][3].gameTag() == 0){
            result |= (1ULL << 2);  // Queen castling
        }
    }
    return result;
}

void Chess::updatePieces(uint64_t& pieces, int check){  //0 = white, 1 = black
    pieces = 0;
    for (int y = 0; y < 8; ++y){
        for (int x = 0; x < 8; ++x){
            int gameTag = _grid[y][x].gameTag();
            if (check == 0 && gameTag > 0 && gameTag < 128){  //For white pieces
                int bitPosition = y * 8 + x;
                pieces |= (1ULL << bitPosition);
            }
            else if(check == 1 && gameTag > 128){  //For black pieces
                int bitPosition = y * 8 + x;
                pieces |= (1ULL << bitPosition);
            }
        }
    }
}


Player* Chess::checkForWinner()
{
    // check to see if either player has won
    return nullptr;
}

bool Chess::checkForDraw()
{
    // check to see if the board is full
    return false;
}

//
// add a helper to Square so it returns out FEN chess notation in the form p for white pawn, K for black king, etc.
// this version is used from the top level board to record moves
//
const char Chess::bitToPieceNotation(int row, int column) const {
    if (row < 0 || row >= 8 || column < 0 || column >= 8) {
        return '0';
    }

    const char* wpieces = { "?PNBRQK" };
    const char* bpieces = { "?pnbrqk" };
    unsigned char notation = '0';
    Bit* bit = _grid[row][column].bit();
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag() & 127];
    } else {
        notation = '0';
    }
    return notation;
}

//
// state strings
//
std::string Chess::initialStateString()
{
    return stateString();
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string Chess::stateString()
{
    std::string s;
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            s += bitToPieceNotation(y, x);
        }
    }
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void Chess::setStateString(const std::string &s)
{
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            int index = y * _gameOptions.rowX + x;
            int playerNumber = s[index] - '0';
            if (playerNumber) {
                _grid[y][x].setBit(PieceForPlayer(playerNumber - 1, Pawn));
            } else {
                _grid[y][x].setBit(nullptr);
            }
        }
    }
}


//
// this is the function that will be called by the AI
//
void Chess::updateAI() 
{
}