#ifndef PF_STOCKFISH_H
#define PF_STOCKFISH_H

int stockfish_init();

void stockfish_quit();

int stockfish_bestmove(const char* fen, int movetime_ms, char* out_move);
int bestmove_parser(const char* mv, int& sri, int& sci, int& dri, int& dci, char& PawnPromoteTo);
#endif