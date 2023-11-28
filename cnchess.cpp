#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <deque>
#include <algorithm>
#include <limits>
#include <cstdio>
#include <cstdint>
#include <cstring>

/*
	Chinese chess board is 10 x 9,
	to speed up rules checking, I added 2 lines for both the top, left, bottom and right sides.
*/
constexpr int32_t BOARD_ROW_LEN = 14;
constexpr int32_t BOARD_COL_LEN = 13;
constexpr int32_t BOARD_ACTUAL_ROW_LEN = 10;
constexpr int32_t BOARD_ACTUAL_COL_LEN = 9;
constexpr int32_t BOARD_ACTUAL_ROW_BEGIN = 2;
constexpr int32_t BOARD_ACTUAL_COL_BEGIN = 2;

// if a pawn has crossed the faced river, then he can move forward, left or right.
constexpr int32_t BOARD_RIVER_UP = BOARD_ACTUAL_ROW_BEGIN + 4;
constexpr int32_t BOARD_RIVER_DOWN = BOARD_ACTUAL_ROW_BEGIN + 5;

// piece: general and advisor must stay within the 9 palace.
constexpr int32_t BOARD_9_PALACE_UP_TOP     = BOARD_ACTUAL_ROW_BEGIN;
constexpr int32_t BOARD_9_PALACE_UP_BOTTOM  = BOARD_ACTUAL_ROW_BEGIN + 2;
constexpr int32_t BOARD_9_PALACE_UP_LEFT    = BOARD_ACTUAL_COL_BEGIN + 3;
constexpr int32_t BOARD_9_PALACE_UP_RIGHT   = BOARD_ACTUAL_COL_BEGIN + 5;

constexpr int32_t BOARD_9_PALACE_DOWN_TOP     = BOARD_ACTUAL_ROW_BEGIN + 7;
constexpr int32_t BOARD_9_PALACE_DOWN_BOTTOM  = BOARD_ACTUAL_ROW_BEGIN + 9;
constexpr int32_t BOARD_9_PALACE_DOWN_LEFT    = BOARD_ACTUAL_COL_BEGIN + 3;
constexpr int32_t BOARD_9_PALACE_DOWN_RIGHT   = BOARD_ACTUAL_COL_BEGIN + 5;

// The max number of steps a player can take in a single turn.
constexpr int32_t MAX_ONE_SIDE_POSSIBLE_MOVES_LEN = 256;

// default AI difficulty.
constexpr uint8_t DEFAULT_AI_SEARCH_DEPTH = 4;

// piece side.
enum PieceSide{
    PS_UP,         // upper side player.
    PS_DOWN,       // down side player.
    PS_EXTRA       // neither upper nor down, for example: empty or out of chess board.
};

// piece type.
enum PieceType{
    PT_PAWN,       // pawn.
    PT_CANNON,     // cannon.
    PT_ROOK,       // rook.
    PT_KNIGHT,     // knight.
    PT_BISHOP,     // bishop.
    PT_ADVISOR,    // advisor.
    PT_GENERAL,    // general.
    PT_EMPTY,      // empty here.
    PT_OUT         // out of chess board.
};

// piece, it is only used as an integer, so there's no need to use enum class(C++11).
enum Piece{
    P_UP,              // upper pawn.
    P_UC,              // upper cannon.
    P_UR,              // upper rook.
    P_UN,              // upper knight.
    P_UB,              // upper bishop.
    P_UA,              // upper advisor.
    P_UG,              // upper general.
    P_DP,              // down pawn.
    P_DC,              // down cannon.
    P_DR,              // down rook.
    P_DN,              // down knight.
    P_DB,              // down bishop.
    P_DA,              // down advisor.
    P_DG,              // down general.
    P_EE,              // empty.
    P_EO,              // out of chess board. used for speeding up rules checking.
    PIECE_TOTAL_LEN    // total number of pieces, you maybe never need this.
};

constexpr char pieceCharMapping[] = {
    // cause #include<> just copy the content, so why not write those things to a txt file rather than writing in code ?
    #include "chessBoardPieceChar.txt"
};

constexpr PieceSide pieceSideMapping[] = {
    PS_UP, PS_UP, PS_UP, PS_UP, PS_UP, PS_UP, PS_UP,
    PS_DOWN, PS_DOWN, PS_DOWN, PS_DOWN, PS_DOWN, PS_DOWN, PS_DOWN,
    PS_EXTRA, PS_EXTRA
};

constexpr PieceType pieceTypeMapping[] = {
    PT_PAWN, PT_CANNON, PT_ROOK, PT_KNIGHT, PT_BISHOP, PT_ADVISOR, PT_GENERAL,
    PT_PAWN, PT_CANNON, PT_ROOK, PT_KNIGHT, PT_BISHOP, PT_ADVISOR, PT_GENERAL,
    PT_EMPTY, PT_OUT
};

constexpr PieceSide pieceSideReverseMapping[] = {
    PS_DOWN,     // upper side reverse is down.
    PS_UP,       // down side reverse is up.
    PS_EXTRA     // extra remains extra.
};

/*
    every piece's value. 
    upper side piece's value is negative, down side is positive. 
*/
constexpr int32_t pieceValueMapping[] = {
    #include "chessBoardPieceValue.txt"
};

/* 
    every piece's position value on the chess board. 
    upper side piece's value is negative, down side is positive. 
*/
constexpr int32_t piecePosValueMapping[][BOARD_ACTUAL_ROW_LEN][BOARD_ACTUAL_COL_LEN] = {
    #include "chessBoardPosValue.txt"
};

constexpr char piece_get_char(Piece p){
    return pieceCharMapping[p];
}

constexpr PieceSide piece_get_side(Piece p){
    return pieceSideMapping[p];
}

constexpr PieceType piece_get_type(Piece p){
    return pieceTypeMapping[p];
}

constexpr PieceSide piece_side_get_reverse(PieceSide side){
    return pieceSideReverseMapping[side];
}

constexpr int32_t piece_get_value(Piece p){
    return pieceValueMapping[p];
}

constexpr int32_t piece_get_pos_value(Piece p, int32_t r, int32_t c){
    return piecePosValueMapping[p][r][c];
}

/* 
    a default chess board, used as a template for new board.
    P_EO is used here for speeding up rules checking.
*/
constexpr Piece CHESS_BOARD_DEFAULT_TEMPLATE[BOARD_ROW_LEN][BOARD_COL_LEN] = {
    { P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO },
    { P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO },
    { P_EO, P_EO, P_UR, P_UN, P_UB, P_UA, P_UG, P_UA, P_UB, P_UN, P_UR, P_EO, P_EO },
    { P_EO, P_EO, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EO, P_EO },
    { P_EO, P_EO, P_EE, P_UC, P_EE, P_EE, P_EE, P_EE, P_EE, P_UC, P_EE, P_EO, P_EO },
    { P_EO, P_EO, P_UP, P_EE, P_UP, P_EE, P_UP, P_EE, P_UP, P_EE, P_UP, P_EO, P_EO },
    { P_EO, P_EO, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EO, P_EO },
    { P_EO, P_EO, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EO, P_EO },
    { P_EO, P_EO, P_DP, P_EE, P_DP, P_EE, P_DP, P_EE, P_DP, P_EE, P_DP, P_EO, P_EO },
    { P_EO, P_EO, P_EE, P_DC, P_EE, P_EE, P_EE, P_EE, P_EE, P_DC, P_EE, P_EO, P_EO },
    { P_EO, P_EO, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EE, P_EO, P_EO },
    { P_EO, P_EO, P_DR, P_DN, P_DB, P_DA, P_DG, P_DA, P_DB, P_DN, P_DR, P_EO, P_EO },
    { P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO },
    { P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO, P_EO },
};

// move node, reprensent a move.
struct MoveNode{
    int32_t beginRow;
    int32_t beginCol;
    int32_t endRow;
    int32_t endCol;

    MoveNode()
        : beginRow{ 0 }, beginCol{ 0 }, endRow{ 0 }, endCol{ 0 }
    {}

    MoveNode(int32_t beginRow, int32_t beginCol, int32_t endRow, int32_t endCol)
        : beginRow{ beginRow }, beginCol{ beginCol }, endRow{ endRow }, endCol{ endCol }
    {}

    bool operator==(const MoveNode& other) const noexcept {
        return beginRow == other.beginRow &&
                beginCol == other.beginCol &&
                endRow == other.endRow &&
                endCol == other.endCol;
    }

    bool operator!=(const MoveNode& other) const noexcept {
        return !(*this == other);
    }
};

// history node, used for undo the previous move.
struct HistoryNode{
    MoveNode move;
    Piece beginPiece;
    Piece endPiece;

    HistoryNode(const MoveNode& moveNode, Piece beginPiece, Piece endPiece)
        : move{ moveNode }, beginPiece{ beginPiece }, endPiece{ endPiece }
    {}
};

// chess board.
class ChessBoard{
    std::array<std::array<Piece, BOARD_COL_LEN>, BOARD_ROW_LEN> data;
    std::deque<HistoryNode> history;
public:
    ChessBoard(){
        memcpy(&(data[0]), &CHESS_BOARD_DEFAULT_TEMPLATE, BOARD_ROW_LEN * BOARD_COL_LEN * sizeof(Piece));
    }

    Piece get(int32_t r, int32_t c) const noexcept {
        return data[r][c];
    }

    void set(int32_t r, int32_t c, Piece p) noexcept {
        data[r][c] = p;
    }

    void move(const MoveNode& moveNode){
        Piece beginPiece = get(moveNode.beginRow, moveNode.beginCol);
        Piece endPiece = get(moveNode.endRow, moveNode.endCol);

        // record the history.
        history.emplace_back(moveNode, beginPiece, endPiece);

        // move the pieces.
        set(moveNode.beginRow, moveNode.beginCol, P_EE);
        set(moveNode.endRow, moveNode.endCol, beginPiece);
    }

    void undo(){
        if (!history.empty()){   // if history is not empty, reset pieces and pop back.
            const HistoryNode& node = history.back();

            set(node.move.beginRow, node.move.beginCol, node.beginPiece);
            set(node.move.endRow, node.move.endCol, node.endPiece);

            history.pop_back();
        }
    }
};

using PossibleMoves = std::vector<MoveNode>;

void check_possible_move_and_insert(const ChessBoard& cb, PossibleMoves& pm, int32_t beginRow, int32_t beginCol, int32_t endRow, int32_t endCol){
    Piece beginP = cb.get(beginRow, beginCol);
    Piece endP = cb.get(endRow, endCol);

    if (endP != P_EO && piece_get_side(beginP) != piece_get_side(endP)){   // not out of chess board, and not the same side.
        pm.emplace_back(beginRow, beginCol, endRow, endCol);
    }
}

void gen_possible_moves_for_pawn(const ChessBoard& cb, PossibleMoves pm, int32_t r, int32_t c, PieceSide side){
    if (side == PS_UP){
        check_possible_move_and_insert(cb, pm,  r, c, r + 1, c);

        if (r > BOARD_RIVER_UP){    // cross the river ?
            check_possible_move_and_insert(cb, pm, r, c, r, c - 1);
            check_possible_move_and_insert(cb, pm, r, c, r, c + 1);
        }
    }
    else if (side == PS_DOWN){
        check_possible_move_and_insert(cb, pm, r, c, r - 1, c);

        if (r < BOARD_RIVER_DOWN){
            check_possible_move_and_insert(cb, pm, r, c, r, c - 1);
            check_possible_move_and_insert(cb, pm, r, c, r, c + 1);
        }
    }
}

void gen_possible_moves_for_cannon_one_direction(const ChessBoard& cb, PossibleMoves& pm, int32_t r, int32_t c, int32_t rGap, int32_t cGap, PieceSide side){
    int32_t row, col;
    Piece p;

    for (row = r + rGap, col = c + cGap; ;row += rGap, col += cGap){
        p = cb.get(row, col);

        if (p == P_EE){    // empty piece, then insert it.
            pm.emplace_back(r, c, row, col);
        }
        else {   // upper piece, down piece or out of chess board, break immediately.
            break;
        }
    }

    if (p != P_EO){   // not out of chess board, check if we can add an enemy piece.
        for (row = row + rGap, col = col + cGap; ;row += rGap, col += cGap){
            p = cb.get(row, col);
        
            if (p == P_EE){    // empty, then continue search.
                continue;
            }
            else if (piece_get_side(p) == piece_side_get_reverse(side)){   // enemy piece, then insert it and break.
                pm.emplace_back(r, c, row, col);
                break;
            }
            else {    // self side piece or out of chess board, break.
                break;
            }
        }
    }
}

void gen_possible_moves_for_cannon(const ChessBoard& cb, PossibleMoves& pm, int32_t r, int32_t c, PieceSide side){
    // go up, down, left, right.
    gen_possible_moves_for_cannon_one_direction(cb, pm, r, c, -1, 0, side);
    gen_possible_moves_for_cannon_one_direction(cb, pm, r, c, +1, 0, side);
    gen_possible_moves_for_cannon_one_direction(cb, pm, r, c, 0, -1, side);
    gen_possible_moves_for_cannon_one_direction(cb, pm, r, c, 0, +1, side);
}

void gen_possible_moves_for_rook_one_direction(const ChessBoard& cb, PossibleMoves& pm, int32_t r, int32_t c, int32_t rGap, int32_t cGap, PieceSide side){
    int32_t row, col;
    Piece p;

    for (row = r + rGap, col = c + cGap; ;row += rGap, col += cGap){
        p = cb.get(row, col);

        if (p == P_EE){    // empty piece, then insert it.
            pm.emplace_back(r, c, row, col);
        }
        else {   // upper piece, down piece or out of chess board, break immediately.
            break;
        }
    }

    if (piece_get_side(p) == piece_side_get_reverse(side)){   // enemy piece, then insert it.
        pm.emplace_back(r, c, row, col);
    }
}

void gen_possible_moves_for_rook(const ChessBoard& cb, PossibleMoves& pm, int32_t r, int32_t c, PieceSide side){
    // go up, down, left, right.
    gen_possible_moves_for_rook_one_direction(cb, pm, r, c, -1, 0, side);
    gen_possible_moves_for_rook_one_direction(cb, pm, r, c, +1, 0, side);
    gen_possible_moves_for_rook_one_direction(cb, pm, r, c, 0, -1, side);
    gen_possible_moves_for_rook_one_direction(cb, pm, r, c, 0, +1, side);
}

void gen_possible_moves_for_knight(const ChessBoard& cb, PossibleMoves& pm, int32_t r, int32_t c, PieceSide side){
    Piece p;
    if ((p = cb.get(r + 1, c)) == P_EE){    // if not lame horse leg ?
        check_possible_move_and_insert(cb, pm, r, c, r + 2, c + 1);
        check_possible_move_and_insert(cb, pm, r, c, r + 2, c - 1);
    }

    if ((p = cb.get(r - 1, c)) == P_EE){
        check_possible_move_and_insert(cb, pm, r, c, r - 2, c + 1);
        check_possible_move_and_insert(cb, pm, r, c, r - 2, c - 1);
    }

    if ((p = cb.get(r, c + 1)) == P_EE){
        check_possible_move_and_insert(cb, pm, r, c, r + 1, c + 2);
        check_possible_move_and_insert(cb, pm, r, c, r - 1, c + 2);
    }

    if ((p = cb.get(r, c - 1)) == P_EE){
        check_possible_move_and_insert(cb, pm, r, c, r + 1, c - 2);
        check_possible_move_and_insert(cb, pm, r, c, r - 1, c - 2);
    }
}

void gen_possible_moves_for_bishop(const ChessBoard& cb, PossibleMoves& pm, int32_t r, int32_t c, PieceSide side){
    Piece p;
    if (side == PS_UP){
        if (r + 2 <= BOARD_RIVER_UP){       // bishop can't cross river.
            if ((p = cb.get(r + 1, c + 1)) == P_EE){    // bishop can move only if Xiang Yan is empty.
                check_possible_move_and_insert(cb, pm, r, c, r + 2, c + 2);
            }

            if ((p = cb.get(r + 1, c - 1)) == P_EE){
                check_possible_move_and_insert(cb, pm, r, c, r + 2, c - 2);
            }
        }

        if ((p = cb.get(r - 1, c + 1)) == P_EE){
            check_possible_move_and_insert(cb, pm, r, c, r - 2, c + 2);
        }

        if ((p = cb.get(r - 1, c - 1)) == P_EE){
            check_possible_move_and_insert(cb, pm, r, c, r - 2, c - 2);
        }
    }
    else if (side == PS_DOWN){
        if (r - 2 >= BOARD_RIVER_DOWN){
            if ((p = cb.get(r - 1, c + 1)) == P_EE){
                check_possible_move_and_insert(cb, pm, r, c, r - 2, c + 2);
            }

            if ((p = cb.get(r - 1, c - 1)) == P_EE){
                check_possible_move_and_insert(cb, pm, r, c, r - 2, c - 2);
            }
        }

        if ((p = cb.get(r + 1, c + 1)) == P_EE){
            check_possible_move_and_insert(cb, pm, r, c, r + 2, c + 2);
        }

        if ((p = cb.get(r + 1, c - 1)) == P_EE){
            check_possible_move_and_insert(cb, pm, r, c, r + 2, c - 2);
        }
    }
}

void gen_possible_moves_for_advisor(const ChessBoard& cb, PossibleMoves& pm, int32_t r, int32_t c, PieceSide side){
    if (side == PS_UP){
        if (r + 1 <= BOARD_9_PALACE_UP_BOTTOM && c + 1 <= BOARD_9_PALACE_UP_RIGHT) {   // walk diagonal lines.
            check_possible_move_and_insert(cb, pm, r, c, r + 1, c + 1);
        }

        if (r + 1 <= BOARD_9_PALACE_UP_BOTTOM && c - 1 >= BOARD_9_PALACE_UP_LEFT) {
            check_possible_move_and_insert(cb, pm, r, c, r + 1, c - 1);
        }

        if (r - 1 >= BOARD_9_PALACE_UP_TOP && c + 1 <= BOARD_9_PALACE_UP_RIGHT) {
            check_possible_move_and_insert(cb, pm, r, c, r - 1, c + 1);
        }

        if (r - 1 >= BOARD_9_PALACE_UP_TOP && c - 1 >= BOARD_9_PALACE_UP_LEFT) {
            check_possible_move_and_insert(cb, pm, r, c, r - 1, c - 1);
        }
    }
    else if (side == PS_DOWN){
        if (r + 1 <= BOARD_9_PALACE_DOWN_BOTTOM && c + 1 <= BOARD_9_PALACE_DOWN_RIGHT) {
            check_possible_move_and_insert(cb, pm, r, c, r + 1, c + 1);
        }

        if (r + 1 <= BOARD_9_PALACE_DOWN_BOTTOM && c - 1 >= BOARD_9_PALACE_DOWN_LEFT) {
            check_possible_move_and_insert(cb, pm, r, c, r + 1, c - 1);
        }

        if (r - 1 >= BOARD_9_PALACE_DOWN_TOP && c + 1 <= BOARD_9_PALACE_DOWN_RIGHT) {
            check_possible_move_and_insert(cb, pm, r, c, r - 1, c + 1);
        }

        if (r - 1 >= BOARD_9_PALACE_DOWN_TOP && c - 1 >= BOARD_9_PALACE_DOWN_LEFT) {
            check_possible_move_and_insert(cb, pm, r, c, r - 1, c - 1);
        }
    }
}

void gen_possible_moves_for_general(const ChessBoard& cb, PossibleMoves& pm, int32_t r, int32_t c, PieceSide side){
    Piece p;
    int32_t row;

    if (side == PS_UP){
        if (r + 1 <= BOARD_9_PALACE_UP_BOTTOM){   // walk horizontal or vertical.
            check_possible_move_and_insert(cb, pm, r, c, r + 1, c);
        }

        if (r - 1 >= BOARD_9_PALACE_UP_TOP){
            check_possible_move_and_insert(cb, pm, r, c, r - 1, c);
        }

        if (c + 1 <= BOARD_9_PALACE_UP_RIGHT){
            check_possible_move_and_insert(cb, pm, r, c, r, c + 1);
        }

        if (c - 1 >= BOARD_9_PALACE_UP_LEFT){
            check_possible_move_and_insert(cb, pm, r, c, r, c - 1);
        }

        // check if both generals faced each other directly.
        for (row = r + 1; row < BOARD_ACTUAL_ROW_BEGIN + BOARD_ACTUAL_ROW_LEN ;++row){
            p = cb.get(row, c);

            if (p == P_EE){
                continue;
            }
            else if (p == P_DG){
                pm.emplace_back(r, c, row, c);
                break;
            }
            else {
                break;
            }
        }
    }
    else if (side == PS_DOWN){
        if (r + 1 <= BOARD_9_PALACE_DOWN_BOTTOM){
            check_possible_move_and_insert(cb, pm, r, c, r + 1, c);
        }

        if (r - 1 >= BOARD_9_PALACE_DOWN_TOP){
            check_possible_move_and_insert(cb, pm, r, c, r - 1, c);
        }

        if (c + 1 <= BOARD_9_PALACE_DOWN_RIGHT){
            check_possible_move_and_insert(cb, pm, r, c, r, c + 1);
        }

        if (c - 1 >= BOARD_9_PALACE_DOWN_LEFT){
            check_possible_move_and_insert(cb, pm, r, c, r, c - 1);
        }

        for (row = r - 1; row >= BOARD_ACTUAL_ROW_BEGIN ;--row){
            p = cb.get(row, c);

            if (p == P_EE){
                continue;
            }
            else if (p == P_UG){
                pm.emplace_back(r, c, row, c);
                break;
            }
            else {
                break;
            }
        }
    }
}

/* 
    generate possible moves for one side. 
    you should call free() on the returned value later.
*/
PossibleMoves gen_possible_moves(const ChessBoard& cb, PieceSide side){
	PossibleMoves pm;

    int32_t endRow = BOARD_ACTUAL_ROW_BEGIN + BOARD_ACTUAL_ROW_LEN;
    int32_t endCol = BOARD_ACTUAL_COL_BEGIN + BOARD_ACTUAL_COL_LEN;

    Piece p;
    for (int32_t r = BOARD_ACTUAL_ROW_BEGIN; r < endRow; ++r) {
        for (int32_t c = BOARD_ACTUAL_COL_BEGIN; c < endCol; ++c){
            p = cb.get(r, c);

            if (piece_get_side(p) == side){
                switch (piece_get_type(p))
                {
                case PT_PAWN:
                    gen_possible_moves_for_pawn(cb, pm, r, c, side);
                    break;
                case PT_CANNON:
                    gen_possible_moves_for_cannon(cb, pm, r, c, side);
                    break;
                case PT_ROOK:
                    gen_possible_moves_for_rook(cb, pm, r, c, side);
                    break;
                case PT_KNIGHT:
                    gen_possible_moves_for_knight(cb, pm, r, c, side);
                    break;
                case PT_BISHOP:
                    gen_possible_moves_for_bishop(cb, pm, r, c, side);
                    break;
                case PT_ADVISOR:
                    gen_possible_moves_for_advisor(cb, pm, r, c, side);
                    break;
                case PT_GENERAL:
                    gen_possible_moves_for_general(cb, pm, r, c, side);
                    break;
                case PT_EMPTY:
                case PT_OUT:
                default:
                    break;
                }
            }
        }
    }

    return pm;
}

/* 
	calculate a chess board's score. 
	upper side value is negative, down side is positive.
*/
int32_t board_calc_score(const ChessBoard& cb){
    int32_t totalScore = 0;
    int32_t endRow = BOARD_ACTUAL_ROW_BEGIN + BOARD_ACTUAL_ROW_LEN;
    int32_t endCol = BOARD_ACTUAL_COL_BEGIN + BOARD_ACTUAL_COL_LEN;

    Piece p;
    for (int32_t r = BOARD_ACTUAL_ROW_BEGIN; r < endRow; ++r) {
        for (int32_t c = BOARD_ACTUAL_COL_BEGIN; c < endCol; ++c){
            p = cb.get(r, c);

			if (p != P_EE){
				totalScore += piece_get_value(p);
				totalScore += piece_get_pos_value(p, r - BOARD_ACTUAL_ROW_BEGIN, c - BOARD_ACTUAL_COL_BEGIN);
            }
        }
    }

    return totalScore;
}

// min-max algorithm, with alpha-beta pruning.
int32_t min_max(ChessBoard& cb, uint8_t searchDepth, int32_t alpha, int32_t beta, PieceSide side){
    if (searchDepth == 0){
        return board_calc_score(cb);
    }

    int32_t minMaxValue;
    if (side == PS_UP){
        int32_t minValue = std::numeric_limits<int32_t>::max();
        PossibleMoves possibleMoves = gen_possible_moves(cb, PS_UP);

        for (const MoveNode& node : possibleMoves) {
            cb.move(node);
            minMaxValue = min_max(cb, searchDepth - 1, alpha, beta, PS_DOWN);
            minValue = std::min(minValue, minMaxValue);
            cb.undo();

            beta = std::min(beta, minValue);
            if (alpha >= beta){
                break;
            }
        }

        return minValue;
    }
    else if (side == PS_DOWN){
        int32_t maxValue = std::numeric_limits<int32_t>::min();
        PossibleMoves possibleMoves = gen_possible_moves(cb, PS_DOWN);

        for (const MoveNode& node : possibleMoves) {
            cb.move(node);
            minMaxValue = min_max(cb, searchDepth - 1, alpha, beta, PS_UP);
            maxValue = std::max(maxValue, minMaxValue);
            cb.undo();

            alpha = std::max(alpha, maxValue);
            if (alpha >= beta){
                break;
            }
        }

        return maxValue;
    }
    else {   // never need this, just for return value.
        return 0;
    }
}

/* 
    gen best move for one side. 
    searchDepth is used as difficulty rank, the bigger it is, the more time the generation costs.
    give param enum PieceSide: PS_EXTRA to this function is meaningless, you will always get a struct MoveNode object with {0, 0, 0, 0}.
*/
MoveNode gen_best_move(ChessBoard& cb, PieceSide side, uint8_t searchDepth){
    int32_t value;
    int32_t alpha = std::numeric_limits<int32_t>::min();
    int32_t beta = std::numeric_limits<int32_t>::max();

    MoveNode bestMove;

    if (side == PS_UP){
        int32_t minValue = beta;
        PossibleMoves possibleMoves = gen_possible_moves(cb, PS_UP);

        for (const MoveNode& node : possibleMoves){
            cb.move(node);
            value = min_max(cb, searchDepth, alpha, beta, PS_DOWN);
            cb.undo();

            if (value <= minValue){
                minValue = value;
                bestMove = node;
            }
        }
    }
    else if (side == PS_DOWN){
        int32_t maxValue = alpha;
        PossibleMoves possibleMoves = gen_possible_moves(cb, PS_DOWN);

        for (const MoveNode& node : possibleMoves){
            cb.move(node);
            value = min_max(cb, searchDepth, alpha, beta, PS_UP);
            cb.undo();

            if (value >= maxValue){
                maxValue = value;
                bestMove = node;
            }
        }
    }
    
    return bestMove;
}

// given move is fit for rule ? return false if not.
bool check_rule(const ChessBoard& cb, const MoveNode& moveNode){
    Piece p = cb.get(moveNode.beginRow, moveNode.beginCol);
    PossibleMoves pm = gen_possible_moves(cb, piece_get_side(p));

    return std::find(pm.cbegin(), pm.cend(), moveNode) != pm.cend();
}

// user input could represent a move ? return false if can't.
bool check_input_is_a_move(const std::string& input){
    if (input.size() < 4){
        return false;
    }

    return  (input[0] >= 'a' && input[0] <= 'i') &&
            (input[1] >= '0' && input[1] <= '9') &&
            (input[2] >= 'a' && input[2] <= 'i') &&
            (input[3] >= '0' && input[3] <= '9');
}

/*
    convert user input to a struct MoveNode object.
    you should call check_input_is_a_move() before to make sure this converting is valid.
*/
MoveNode convert_input_to_move(const std::string& input){
    MoveNode m;

    m.beginRow = 9 - (static_cast<int32_t>(input[1]) - static_cast<int32_t>('0')) + BOARD_ACTUAL_ROW_BEGIN;
    m.beginCol = static_cast<int32_t>(input[0]) - static_cast<int32_t>('a') + BOARD_ACTUAL_COL_BEGIN;
    m.endRow   = 9 - (static_cast<int32_t>(input[3]) - static_cast<int32_t>('0')) + BOARD_ACTUAL_ROW_BEGIN;
    m.endCol   = static_cast<int32_t>(input[2]) - static_cast<int32_t>('a') + BOARD_ACTUAL_COL_BEGIN;

    return m;
}

// convert a move to string.
std::string convert_move_to_str(const MoveNode& move){
    std::string buf;

    buf += static_cast<char>(move.beginCol - BOARD_ACTUAL_COL_BEGIN + 'a');
    buf += static_cast<char>(9 - (move.beginRow - BOARD_ACTUAL_ROW_BEGIN) + '0');
    buf += static_cast<char>(move.endCol - BOARD_ACTUAL_COL_BEGIN + 'a');
    buf += static_cast<char>(9 - (move.endRow - BOARD_ACTUAL_ROW_BEGIN) + '0');
    return buf;
}

// every one can only move his pieces, not the enemy's.
bool check_is_this_your_piece(const ChessBoard& cb, const MoveNode& move, PieceSide side){
    Piece p = cb.get(move.beginRow, move.beginCol);
    return piece_get_side(p) == side;
}

// if no one wins, return PS_EXTRA.
PieceSide check_winner(const ChessBoard& cb){
    bool upAlive = false;
    bool downAlive = false;

    for (int32_t r = BOARD_9_PALACE_UP_TOP; r <= BOARD_9_PALACE_UP_BOTTOM; ++r) {
        for (int32_t c = BOARD_9_PALACE_UP_LEFT; c <= BOARD_9_PALACE_UP_RIGHT; ++c) {
            if (cb.get(r, c) == P_UG) {
                upAlive = true;
                break;
            }
        }
    }

    for (int32_t r = BOARD_9_PALACE_DOWN_TOP; r <= BOARD_9_PALACE_DOWN_BOTTOM; ++r) {
        for (int32_t c = BOARD_9_PALACE_DOWN_LEFT; c <= BOARD_9_PALACE_DOWN_RIGHT; ++c) {
            if (cb.get(r, c) == P_DG) {
                downAlive = true;
                break;
            }
        }
    }

    if (upAlive && downAlive) {
        return PS_EXTRA;
    }
    else if (upAlive) {
        return PS_UP;
    }
    else {
        return PS_DOWN;
    }
}

void print_board_to_console(const ChessBoard& cb){
    int splitLineIndex = BOARD_ACTUAL_ROW_BEGIN + (BOARD_ACTUAL_ROW_LEN / 2);
    int endRow = BOARD_ACTUAL_ROW_BEGIN + BOARD_ACTUAL_ROW_LEN;
    int endCol = BOARD_ACTUAL_COL_BEGIN + BOARD_ACTUAL_COL_LEN;
    int n = BOARD_ACTUAL_ROW_LEN - 1;

    std::cout << "\n    +-------------------+\n";

    int r, c;
    for (r = BOARD_ACTUAL_ROW_BEGIN; r < endRow; ++r) {
        if (r == splitLineIndex){
            std::cout << "    |===================|\n";
            std::cout << "    |===================|\n";
        }

        std::cout << " " << n-- << "  | ";

        for (c = BOARD_ACTUAL_COL_BEGIN; c < endCol; ++c){
            std::cout << piece_get_char(cb.get(r, c)) << " ";
        }

        std::cout << "|\n";
    }

    std::cout << "    +-------------------+\n";
    std::cout << "\n      a b c d e f g h i\n\n";
}

void print_help_page(){
    std::cout << "\n=======================================\n";
    std::cout << "Help Page\n\n";
    std::cout << "    1. help         - this page.\n";
    std::cout << "    2. b2e2         - input like this will be parsed as a move.\n";
    std::cout << "    3. undo         - undo the previous move.\n";
    std::cout << "    4. exit or quit - exit the game.\n";
    std::cout << "    5. remake       - remake the game.\n";
    std::cout << "    6. advice       - give me a best move.\n\n";
    std::cout << "  The characters on the board have the following relationships: \n\n";
    std::cout << "    P -> AI side pawn.\n";
    std::cout << "    C -> AI side cannon.\n";
    std::cout << "    R -> AI side rook.\n";
    std::cout << "    N -> AI side knight.\n";
    std::cout << "    B -> AI side bishop.\n";
    std::cout << "    A -> AI side advisor.\n";
    std::cout << "    G -> AI side general.\n";
    std::cout << "    p -> our pawn.\n";
    std::cout << "    c -> our cannon.\n";
    std::cout << "    r -> our rook.\n";
    std::cout << "    n -> our knight.\n";
    std::cout << "    b -> our bishop.\n";
    std::cout << "    a -> our advisor.\n";
    std::cout << "    g -> our general.\n";
    std::cout << "    . -> no piece here.\n";
    std::cout << "=======================================\n";
    std::cout << "Press any key to continue.\n";

    (void)getchar();
}

int main(){
    PieceSide userSide = PS_DOWN;
    PieceSide aiSide = PS_UP;

    ChessBoard cb;
    std::string userInput;

    print_board_to_console(cb);

    while (1){
        std::cout << "Your move: ";
        std::getline(std::cin, userInput);

        if (userInput == "help"){
            print_help_page();
            print_board_to_console(cb);
        }
        else if (userInput == "undo"){
            cb.undo();
            cb.undo();
            print_board_to_console(cb);
        }
        else if (userInput == "quit"){
            return 0;
        }
        else if (userInput == "exit"){
            return 0;
        }
        else if (userInput == "remake"){
            cb = ChessBoard{};

            std::cout << "New cnchess started.\n";
            print_board_to_console(cb);
            continue;
        }
        else if (userInput == "advice"){
            MoveNode advice = gen_best_move(cb, userSide, DEFAULT_AI_SEARCH_DEPTH);
            std::string adviceStr = convert_move_to_str(advice);
            std::cout << "Maybe you can try: " << adviceStr 
                        << ", piece is " << piece_get_char(cb.get(advice.beginRow, advice.beginCol))
                        << ".\n";
        }
        else{
            if (check_input_is_a_move(userInput)){
                MoveNode userMove = convert_input_to_move(userInput);
                
                if (!check_is_this_your_piece(cb, userMove, userSide)){
                    std::cout << "This piece is not yours, please choose your piece.\n";
                    continue;
                }

                if (check_rule(cb, userMove)){
                    cb.move(userMove);
                    print_board_to_console(cb);

                    if (check_winner(cb) == userSide){
                        std::cout << "Congratulations! You win!\n";
                    }

                    std::cout << "AI thinking...\n";
                    MoveNode aiMove = gen_best_move(cb, aiSide, DEFAULT_AI_SEARCH_DEPTH);
                    std::string aiMoveStr = convert_move_to_str(aiMove);
                    cb.move(aiMove);
                    print_board_to_console(cb);
                    std::cout << "AI move: " << aiMoveStr
                             << ", piece is '" << piece_get_char(cb.get(aiMove.endRow, aiMove.endCol)) 
                             << "'.\n";

                    if (check_winner(cb) == aiSide){
                        std::cout << "Game over! You lose!\n";
                    }
                }
                else {
                    std::cout << "Given move doesn't fit for rules, please re-enter.\n";
                    continue;
                }
            }
            else {
                std::cout << "Input is not a valid move nor instruction, please re-enter(try help ?).\n";
                continue;
            }
        }
    }

    return 0;
}
