#include "Chess.h"
#include <cstdint>

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
    loadFromFEN("RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr");
    // loadFromFEN("RRRRRRQR/8/8/8/8/8/8/bbbbbbbb");

    startGame();
}


    // helper function that FEN notation to fill board
void Chess::loadFromFEN(const std::string &fen){
    static const std::unordered_map<char, ChessPiece> pieceMap = {  //Creates a map for the notation to the piece
        {'p', Pawn}, {'r', Rook}, {'n', Knight}, {'b', Bishop},
        {'q', Queen}, {'k', King}
    };
    int posCount = 0;  //Board position

    for(char c : fen){
        if(c == ' '){  //End of FEN
            break;
        }
        if(c == '/'){  //NewLine for Fen
            continue;
        }
        else if(isdigit(c)){
            posCount += c -'0';
        }
        else{

            bool isWhite = isupper(c);  //Checks if uppercase letter which = white pieces
            char lowerC = tolower(c);  //If white, sets to lowercase for map finding
            auto piece = pieceMap.find(lowerC);

            if (piece != pieceMap.end()) {  //This will set all the bit and grid information
                Bit* bit = PieceForPlayer(isWhite ? 0 : 1, piece->second); 
                bit->setPosition(_grid[posCount/8][posCount%8].getPosition());  //Sets bit position
                bit->setParent(&_grid[posCount/8][posCount%8]);  //Sets the grid parent, idk really
                bit->setGameTag(isWhite ? piece->second : piece->second + 128);  //What piece is it and color
                _grid[posCount/8][posCount%8].setGameTag(isWhite ? piece->second : piece->second + 128);;  //Grid gets to hold onto piece and color
                _grid[posCount/8][posCount%8].setBit(bit);  //Set the bit in palce too
                (isWhite ? wPieces : bPieces) |= (1ULL << posCount); 
            }
            posCount++;
        }
    }
}


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

bool Chess::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) { // Piece, Position, go to Position
    int pieceTag = (bit.gameTag() > 128) ? bit.gameTag() - 128 : bit.gameTag();  //If black, remove 128 from gameTag
    ChessSquare* srcSquare = dynamic_cast<ChessSquare*>(&src);  //Grab location of start
    ChessSquare* dstSquare = dynamic_cast<ChessSquare*>(&dst);  //Grab location of end
    int x = srcSquare->getColumn();
    int y = srcSquare->getRow();
    int total = (8*y)+x;  
    int x2 = dstSquare->getColumn();
    int y2 = dstSquare->getRow();
    int total2 = (8*y2)+x2;  //Everything above just turns it into x, y, and square space

    int player = getCurrentPlayer()->playerNumber();
    bool isWhite = (bit.gameTag() < 128) ? true : false;
    uint64_t cPieces = player == 0 ? wPieces : bPieces;  //Check pieces for which can be selected 
    bool isPlayable = (cPieces & (1ULL << total)) != 0;  //Is the selected piece apart of the player's pieces

    if((isWhite && player == 0) || (!isWhite && player == 1)){  //Makes sure that you don't mix and match color and turn
        if(pieceTag == Rook && isPlayable){
            uint64_t result = ratt(total, wPieces, bPieces);
            return (result & (1ULL << total2)) != 0;
        }
        if(pieceTag == Bishop && isPlayable){
            uint64_t result = batt(total, wPieces, bPieces);
            return (result & (1ULL << total2)) != 0;
        }
        if(pieceTag == Queen && isPlayable){
            uint64_t result1 = ratt(total, wPieces, bPieces);
            uint64_t result2 = batt(total, wPieces, bPieces);
            result1 = result1 | result2;
            return (result1 & (1ULL << total2)) != 0;
        }
        if(pieceTag == Pawn && isPlayable){
            uint64_t result = patt(total, wPieces, bPieces);
            return (result & (1ULL << total2)) != 0;
        }
        if(pieceTag == Knight && isPlayable){
            uint64_t result = natt(total, wPieces, bPieces);
            return (result & (1ULL << total2)) != 0;
        }
        if(pieceTag == King && isPlayable){
            uint64_t result = katt(total, wPieces, bPieces);
            return (result & (1ULL << total2)) != 0;
        }
    }
    return false;

}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) { // Post Check piece, old position, new position
    int pieceTag = (bit.gameTag() > 128) ? bit.gameTag() - 128 : bit.gameTag();  //If black, remove 128 from gameTag
    ChessSquare* srcSquare = dynamic_cast<ChessSquare*>(&src);
    ChessSquare* dstSquare = dynamic_cast<ChessSquare*>(&dst);
    int x = srcSquare->getColumn();
    int y = srcSquare->getRow();
    int total = (8*y)+x;

    int x2 = dstSquare->getColumn();
    int y2 = dstSquare->getRow();
    int total2 = (8*y2)+x2;

    _grid[y][x].setGameTag(0);  //Removes old piece that grid was holding
    _grid[y2][x2].setGameTag(bit.gameTag());  //Updates the space's tag of what piece its holding

    updatePieces(wPieces, 0);  //Update our bitbaords of white and black pieces
    updatePieces(bPieces, 1);

    //Eventually add more functionality

    endTurn();
}

//
// free all the memory used by the game on the heap
//
void Chess::stopGame()
{
}


uint64_t Chess::ratt(int sq, uint64_t wPieces, uint64_t bPieces) {  //Rook attack
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
    for(r = rk - 1; r >= 0; r--) {  //Same stuff as above, but different for loop
        if(block & (1ULL << (fl + r * 8))) {  
            if(blockPieces & (1ULL << (fl + r * 8))) { 
                result |= (1ULL << (fl + r * 8)); 
            }
            break; 
        }
        result |= (1ULL << (fl + r * 8));  
    }

    //East
    for(f = fl + 1; f <= 7; f++) {  //Same stuff as above, but different for loop
        if(block & (1ULL << (f + rk * 8))) {  
            if(blockPieces & (1ULL << (f + rk * 8))) {  
                result |= (1ULL << (f + rk * 8)); 
            }
            break;  
        }
        result |= (1ULL << (f + rk * 8)); 
    }

    //West
    for(f = fl - 1; f >= 0; f--) {  //Same stuff as above, but different for loop
        if(block & (1ULL << (f + rk * 8))) { 
            if(blockPieces & (1ULL << (f + rk * 8))) { 
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
        if((block & (1ULL << (sq + 9)))){  //Capture right
            if(blockPieces & (1ULL << (sq + 9))) { 
                result |= (1ULL << (sq + 9)); 
            }
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
        if((block & (1ULL << (sq - 9)))){  //Capture right
            if(blockPieces & (1ULL << (sq - 9))){ 
                result |= (1ULL << (sq - 9)); 
            }
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

    for(int move : knightMoves) {
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

    for(int move : kingMoves) {
        int targetSq = sq + move;
        if(targetSq < 0 || targetSq > 63) continue;
        if((sq % 8 == 0 && (move == -1 || move == -9 || move == 7)) ||  //Edge cases if king is left or right
            (sq % 8 == 7 && (move == 1 || move == -7 || move == 9))) {   
            continue;
        }
        uint64_t targetPos = 1ULL << targetSq;
        result |= (blockPieces & targetPos) ? 0ULL : targetPos;
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
