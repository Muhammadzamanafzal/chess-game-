#include<iostream>
#include<ctime>
#include<cstring>
#include<Windows.h>
#include <crtdbg.h>
#include <fstream>
#include <conio.h>
#include"pf_stockfish.h"
using namespace std;
enum Color { Black, White };
struct POSITION
{
	int row, col;
};
struct Piece
{
	char sym;
	Color C;
	POSITION P;
	int value;
	bool (*isvalidmove)(POSITION, POSITION, Piece* board[8][8]);
	void (*Draw)(int cr, int cc);
};
struct player
{
	char* sym;
	Color Pc;
};
struct replayboard
{
	char sym;
	Color C;
};
struct AImove
{
	POSITION S;
	POSITION D;
};

const Color AIcolor = White;
replayboard  Replay[300][8][8];
int replaysize = 0;
int replyindex = 1;
void color(int k)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, k);
}
void changeturn(Color& turn);
bool isselfcheck(Piece* board[8][8], POSITION S, POSITION D, Color turn);
void MANUbox(int a, int b, int c);
void hideConsoleCursor()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = false;
	SetConsoleCursorInfo(hConsole, &cursorInfo);
}
void gotoRowCol(int rpos, int cpos)
{
	COORD scrn;
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	scrn.X = cpos;
	scrn.Y = rpos;
	SetConsoleCursorPosition(hOutput, scrn);
}
void printline(int p1r, int p1c, int p2r, int p2c, char sym = -37)
{
	for (int i = 0; i <= 20; i += 1)
	{
		double a = 1.0 * i / 20.0;
		double pr = p1r * a + p2r * (1 - a);
		double pc = p1c * a + p2c * (1 - a);
		if (p2r == p1r)
			pr = p1r;
		if (p1c == p2c)
			pc = p1c;
		if (pr < 0 or pc < 0)
			continue;
		gotoRowCol(pr, pc);
		cout << sym;
	}
}
void pawn(int cr, int cc)
{
	cc = cc - 1;
	printline(cr - 3, cc, cr + 2, cc);
	printline(cr - 2, cc + 1, cr + 2, cc + 1);
	printline(cr - 2, cc - 1, cr + 2, cc - 1);
	printline(cr + 2, cc - 2, cr + 2, cc + 2);
}
void rook(int cr, int cc)
{
	cc = cc - 1;
	printline(cr - 3, cc, cr + 3, cc);
	printline(cr - 3, cc + 1, cr + 3, cc + 1);
	printline(cr - 2, cc + 2, cr + 3, cc + 2);
	printline(cr - 2, cc - 1, cr + 3, cc - 1);
	printline(cr + 2, cc - 2, cr + 2, cc + 3);
	printline(cr + 3, cc - 2, cr + 3, cc + 3);
	printline(cr - 2, cc - 2, cr - 2, cc + 3);
	printline(cr - 3, cc + 3, cr - 3, cc + 3);
	printline(cr - 3, cc - 2, cr - 3, cc - 2);

}
void bishop(int cr, int cc)
{
	cc = cc - 1;
	printline(cr - 4, cc, cr + 3, cc);
	printline(cr - 3, cc + 1, cr + 3, cc + 1);
	printline(cr - 3, cc - 1, cr + 3, cc - 1);
	printline(cr - 2, cc - 2, cr - 2, cc + 2);
	printline(cr + 1, cc - 2, cr + 1, cc + 2);
	printline(cr + 2, cc - 2, cr + 2, cc + 2);
	printline(cr + 3, cc - 2, cr + 3, cc + 2);
}
void queen(int cr, int cc)
{
	printline(cr - 2, cc, cr + 2, cc);
	printline(cr - 1, cc + 1, cr + 2, cc + 1);
	printline(cr - 1, cc - 1, cr + 2, cc - 1);
	printline(cr - 2, cc + 2, cr + 2, cc + 2);
	printline(cr - 3, cc + 3, cr, cc + 3);
	printline(cr - 1, cc - 2, cr - 2, cc + 2);
	printline(cr - 1, cc - 2, cr - 1, cc + 2);
	printline(cr, cc - 3, cr, cc + 3);
	printline(cr - 2, cc - 2, cr - 2, cc + 2);
	printline(cr - 1, cc - 2, cr - 1, cc + 2);
	printline(cr, cc - 3, cr, cc + 3);
	printline(cr - 2, cc - 2, cr + 2, cc - 2);
	printline(cr - 3, cc - 3, cr, cc - 3);

}
void knight(int cr, int cc)
{
	printline(cr - 3, cc - 2, cr - 3, cc + 2);
	printline(cr - 2, cc - 3, cr - 2, cc + 2);
	printline(cr - 2, cc + 2, cr + 3, cc - 3);
	printline(cr + 3, cc - 3, cr + 3, cc + 2);
	//	printline(cr + 4, cc - 3, cr + 4, cc + 2);
}
void king(int cr, int cc)
{
	cc = cc - 1;
	printline(cr - 3, cc - 2, cr + 2, cc - 2);
	printline(cr, cc - 2, cr, cc + 3);
	printline(cr, cc, cr - 4, cc);
	printline(cr, cc + 1, cr - 4, cc + 1);

	printline(cr + 2, cc - 2, cr + 2, cc + 3);
	printline(cr + 3, cc - 2, cr + 3, cc + 3);
	//	printline(cr + 4, cc - 2, cr + 4, cc + 3);
	printline(cr + 3, cc + 3, cr - 3, cc + 3);
}
void getRowColbyLeftClick(int& rpos, int& cpos)
{
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	DWORD Events;
	INPUT_RECORD InputRecord;
	SetConsoleMode(hInput, ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT |
		ENABLE_EXTENDED_FLAGS);
	do
	{
		ReadConsoleInput(hInput, &InputRecord, 1, &Events);
		if (InputRecord.Event.MouseEvent.dwButtonState ==

			FROM_LEFT_1ST_BUTTON_PRESSED)

		{
			cpos = InputRecord.Event.MouseEvent.dwMousePosition.X;
			rpos = InputRecord.Event.MouseEvent.dwMousePosition.Y;
			break;
		}
	} while (true);
}
bool isHorizontalMove(POSITION S, POSITION D)
{
	return (S.row == D.row) and (S.col != D.col);
}
bool isverticalMove(POSITION S, POSITION D)
{
	return (S.row != D.row) and (S.col == D.col);
}
bool isdiagonalMove(POSITION S, POSITION D)
{
	return abs(S.row - D.row) == abs(S.col - D.col) and (S.row != D.row and S.col != D.col);
}
bool isHorizontalPathClear(POSITION S, POSITION D, Piece* board[8][8])
{
	int startC, EndC;
	if (S.col > D.col)
	{
		startC = D.col;
		EndC = S.col;
	}
	else
	{
		startC = S.col;
		EndC = D.col;
	}
	for (int i = startC + 1;i < EndC;i++)
	{
		if (board[S.row][i]->sym != '-')
		{
			return false;
		}
	}
	return true;
}
bool isverticalPathClear(POSITION S, POSITION D, Piece* board[8][8])
{
	int startR, EndR;
	if (S.row > D.row)
	{
		startR = D.row;
		EndR = S.row;
	}
	else
	{
		startR = S.row;
		EndR = D.row;
	}
	for (int i = startR + 1;i < EndR;i++)
	{
		if (board[i][S.col]->sym != '-')
		{
			return false;
		}
	}
	return true;
}
bool isDigonalPathClear(POSITION S, POSITION D, Piece* board[8][8])
{
	int dr = (S.row - D.row);
	int dc = (S.col - D.col);
	if (dr < 0 and dc < 0)
	{
		for (int i = 1;i < abs(dr);i++)
		{
			if (board[S.row + i][S.col + i]->sym != '-')
			{
				return false;
			}
		}
		return true;
	}
	if (dr > 0 and dc < 0)
	{
		for (int i = 1;i < abs(dr);i++)
		{
			if (board[S.row - i][S.col + i]->sym != '-')
			{
				return false;
			}
		}
		return true;
	}
	if (dr < 0 and dc > 0)
	{
		for (int i = 1;i < abs(dr);i++)
		{
			if (board[S.row + i][S.col - i]->sym != '-')
			{
				return false;
			}
		}
		return true;
	}
	if (dr > 0 and dc > 0)
	{
		for (int i = 1;i < abs(dr);i++)
		{
			if (board[S.row - i][S.col - i]->sym != '-')
			{
				return false;
			}
		}
		return true;
	}
	return false;

}
bool isLegalPawn(POSITION S, POSITION D, Piece* board[8][8])
{
	Piece* p = board[S.row][S.col];
	int dir = (p->C == White) ? 1 : -1;
	if (D.col == S.col && board[D.row][D.col]->sym == '-')
	{
		if (D.row == S.row + dir)
			return true;
		if ((p->C == White && S.row == 1 && D.row == 3) ||
			(p->C == Black && S.row == 6 && D.row == 4))
		{
			if (board[S.row + dir][S.col]->sym == '-')
				return true;
		}
	}
	if (abs(D.col - S.col) == 1 && D.row == S.row + dir)
	{
		if (board[D.row][D.col]->sym != '-' &&
			board[D.row][D.col]->C != p->C)
			return true;
	}

	return false;
}
bool isLegalRook(POSITION S, POSITION D, Piece* board[8][8])
{
	return (isHorizontalMove(S, D) and isHorizontalPathClear(S, D, board)) or (isverticalMove(S, D) and isverticalPathClear(S, D, board));
}
bool isLegalBishop(POSITION S, POSITION D, Piece* board[8][8])
{
	return isdiagonalMove(S, D) and isDigonalPathClear(S, D, board);
}
bool isLegalQueen(POSITION S, POSITION D, Piece* board[8][8])
{
	return isLegalBishop(S, D, board) or isLegalRook(S, D, board);
}
bool isLegalKing(POSITION S, POSITION D, Piece* board[8][8])
{
	int dr = abs(D.row - S.row);
	int dc = abs(D.col - S.col);
	return dr <= 1 and dc <= 1;
}
bool isLegalKnight(POSITION S, POSITION D, Piece* board[8][8])
{
	int dr = abs(D.row - S.row);
	int dc = abs(D.col - S.col);
	return (dr == 2 and dc == 1) or (dr == 1 and dc == 2);
}
void clearline(int r, int c, int len = 5)
{
	gotoRowCol(r, c);
	for (int i = 0;i < len;i++)
	{
		cout << " ";
	}
}
void turnmsgdisplay(Color turn, player P[])
{
	int c = (turn == Black) ? 4 : 7;
	MANUbox(5, 81, 17);
	color(c);
	gotoRowCol(8, 84);
	cout << "Player is : " << P[turn].sym;
	gotoRowCol(10, 84);
	cout << "Color is (" << (turn == Black ? "Black" : "White") << ")" << endl;
}
void init(Piece* Board[][8], player PS[], Color& T)
{
	T = (Color)(rand() % 2);
	for (int ri = 0; ri < 8; ri++)
	{
		for (int ci = 0; ci < 8; ci++)
		{
			Board[ri][ci] = new Piece{ '-',Black,{ri,ci},0,nullptr,nullptr };
		}
	}
	for (int ci = 0; ci < 8; ci++)
	{
		Board[1][ci] = new Piece{ 'P',White,{1,ci},2,isLegalPawn,pawn };
	}
	for (int ci = 0; ci < 8; ci++)
	{
		Board[6][ci] = new Piece{ 'p',Black,{6,ci},2,isLegalPawn,pawn };
	}
	//upper
	Board[0][0] = new Piece{ 'R',White,{0,0},4 ,isLegalRook,rook };
	Board[0][7] = new Piece{ 'R',White,{0,7},4,isLegalRook,rook };
	Board[0][1] = new Piece{ 'N',White,{0,1},6,isLegalKnight,knight };
	Board[0][6] = new Piece{ 'N',White,{0,6},6,isLegalKnight,knight };
	Board[0][2] = new Piece{ 'B',White,{0,2},8,isLegalBishop,bishop };
	Board[0][5] = new Piece{ 'B',White,{0,5},8,isLegalBishop,bishop };
	Board[0][4] = new Piece{ 'K',White,{0,4},50,isLegalKing,king };
	Board[0][3] = new Piece{ 'Q',White,{0,3},30,isLegalQueen,queen };
	//lower
	Board[7][0] = new Piece{ 'r',Black,{7,0},4 ,isLegalRook,rook };
	Board[7][7] = new Piece{ 'r',Black,{7,7},4 ,isLegalRook,rook };
	Board[7][1] = new Piece{ 'n',Black,{7,1},6,isLegalKnight,knight };
	Board[7][6] = new Piece{ 'n',Black,{7,6},6,isLegalKnight,knight };
	Board[7][2] = new Piece{ 'b',Black,{7,2},8,isLegalBishop,bishop };
	Board[7][5] = new Piece{ 'b',Black,{7,5},8,isLegalBishop,bishop };
	Board[7][4] = new Piece{ 'k',Black,{7,4},50,isLegalKing,king };
	Board[7][3] = new Piece{ 'q',Black,{7,3},30,isLegalQueen,queen };
}
int assignvalue(char p)
{
	switch ((tolower(p)))
	{
		case'p': return 2;
		case'r':return 4;
		case'n':return 6;
		case'b':return 8;
		case'k':return 50;
		case'q':return 30;
		default: return 0;
	}
}
void assignsym(Piece* p)
{
	p->value=assignvalue(p->sym);
	switch (p->sym)
	{
	case 'P':
	case 'p':
	{
		p->isvalidmove = isLegalPawn;
		p->Draw = pawn;
		break;
	}
	case 'K':
	case 'k':
	{
		p->isvalidmove = isLegalKing;
		p->Draw = king;
		break;
	}
	case 'Q':
	case 'q':
	{
		p->isvalidmove = isLegalQueen;
		p->Draw = queen;
		break;
	}
	case 'R':
	case 'r':
	{
		p->isvalidmove = isLegalRook;
		p->Draw = rook;
		break;
	}
	case 'N':
	case 'n':
	{
		p->isvalidmove = isLegalKnight;
		p->Draw = knight;
		break;
	}
	case 'B':
	case 'b':
	{
		p->isvalidmove = isLegalBishop;
		p->Draw = bishop;
		break;
	}
	default:
	{
		p->isvalidmove = nullptr;
		p->Draw = nullptr;
	}

	}
}
void loadgame(Piece* Board[][8], player PS[], Color& T)
{
	for (int i = 0;i < 8;i++)
	{
		for (int j = 0;j < 8;j++)
		{
			Board[i][j] = nullptr;
		}
	}
	ifstream Rdr("loadedfile1.txt");
	if (!Rdr)
		return;
	int turn;
	Rdr >> turn;
	T = (Color)turn;
	char name[50];
	for (int i = 0;i < 2;i++)
	{
		Rdr >> name;
		PS[i].sym = new char[strlen(name) + 1];
		strcpy_s(PS[i].sym, strlen(name) + 1, name);

		PS[i].Pc = Color(i);
	}
	char s;
	for (int i = 0;i < 8;i++)
	{
		for (int j = 0;j < 8;j++)
		{
			Rdr >> s;
			Color C = (s >= 'A' and s <= 'Z') ? White : Black;
			if (Board[i][j] != nullptr)
			{
				delete Board[i][j];
			}
			Board[i][j] = new Piece{ s,C,{i,j},0,nullptr,nullptr };
			if (s == '-')
			{
				Board[i][j]->C = Black;
			}
			assignsym(Board[i][j]);
		}
	}



}
void Selectpiece(POSITION& S)
{

	int r, c;
	getRowColbyLeftClick(r, c);
	S.row = r / 10;
	S.col = c / 10;
}
void selectdestination(POSITION& d)
{
	int r, c;
	getRowColbyLeftClick(r, c);
	d.row = r / 10;
	d.col = c / 10;
}
bool isvalidsource(Piece* Board[][8], POSITION S, Color turn)
{
	if (S.row > 7 or S.row < 0 or S.col>7 or S.col < 0) return false;
	if (Board[S.row][S.col]->sym == '-')
	{
		return false;
	}
	if (Board[S.row][S.col]->C != turn)
		return false;
	return true;

}
bool isvaliddestination(Piece* Board[][8], POSITION d, Color turn)
{
	if (d.row > 7 or d.row < 0 or d.col>7 or d.col < 0) return false;
	
	if (Board[d.row][d.col]->sym == '-')
	{
		return true;
	}
	if (Board[d.row][d.col]->C != turn)
		return true;
	return false;

}
void promotepawn(Piece* P)
{
	char Choice;
	if((P->C == White))
	{
	do
	{
		cout << "Promote Pawn into (Q,K,B,N)";
		cin >> Choice;
		Choice = toupper(Choice);

	} while (Choice != 'Q' and Choice != 'K' and Choice != 'N' and Choice != 'B');
	}
	Color c = P->C;
	if (P->C == White)
	{
	P->sym=Choice;
		assignsym(P);
	}
	else
	{
		P->sym='q';
		assignsym(P);
	}




	/*P->sym = (c == White) ? Choice : choice;
	assignsym(P);*/
}
void updateboard(Piece* Board[][8], POSITION s, POSITION d)
{
	if (Board[d.row][d.col]->sym != '-')
		delete Board[d.row][d.col];
	Board[d.row][d.col] = Board[s.row][s.col];
	Board[d.row][d.col]->P.row = d.row;
	Board[d.row][d.col]->P.col = d.col;
	Board[s.row][s.col] = new Piece{ '-',Black,{s.row,s.col},0,nullptr };
	if (Board[d.row][d.col]->sym == 'P' and d.row == 7)
	{
		promotepawn(Board[d.row][d.col]);
	}
	if (Board[d.row][d.col]->sym == 'p' and d.row == 0)
	{
		promotepawn(Board[d.row][d.col]);
	}
}
void changeturn(Color& turn)
{
	if (turn == Black)
	{
		turn = White;
	}
	else
	{
		turn = Black;
	}
}
void specialbox(int sr, int sc)
{
	sr = sr * 10;
	sc = sc * 10;
	color(4);
	char sym = -37;
	for (int r = 0; r < 10; r++)
	{
		for (int c = 0; c < 10; c++)
		{
			if (r == 0 or c == 0 or r == 9 or c == 9)
			{
				color(41);
				gotoRowCol(sr + r, sc + c);
				cout << sym;
			}
		}
	}
}
void printbox(int sr, int sc, int rows, int cols, Piece* board[][8])
{
	color(6);
	char sym = -37;
	for (int r = 0; r < 10; r++)
	{
		for (int c = 0; c < 10; c++)
		{
			if (r == 0 or c == 0 or r == 9 or c == 9)
			{
				color(6);
				gotoRowCol(sr + r, sc + c);
				cout << sym;
			}
			if (!(r == 0 or c == 0 or r == 9 or c == 9))
			{
				int boxColor = (rows + cols) % 2 == 0 ? 1 : 4;
				color(boxColor);
				gotoRowCol(sr + r, sc + c);
				cout << sym;
			}
		}
	}
	for (int r = 0; r < 10; r++)
	{
		for (int c = 0; c < 10; c++)
		{
			if (r == 4 and c == 4)
			{
				color(board[rows][cols]->C == Black ? 0 : 15);
				gotoRowCol(sr + r, sc + c);
				Piece* p = board[rows][cols];
				if (p->Draw != nullptr)
				{
					p->Draw(sr + 5, sc + 5);
				}
			}
			color(6);
		}
	}
}
void BOARD(Piece* board[][8])
{

	for (int r = 0; r < 8; r++)
	{
		for (int c = 0; c < 8; c++)
		{

			printbox(r * 10, 10 * c, r, c, board);
		}
	}


}
void printboxMB(int row, int col, Piece* board[][8], bool BM[8][8], char sym)
{

	hideConsoleCursor();
	int sr = 10 * row;
	int sc = 10 * col;
	for (int r = 0; r < 10; r++)
	{
		for (int c = 0; c < 10; c++)
		{
			if (BM[row][col])
			{
				color(5);
				if (r == 0 or r == 9 or c == 0 or c == 9)
				{
					gotoRowCol(sr + r, sc + c);
					cout << sym;
				}
			}
			color(6);
		}
	}
}
void BOARDBM(Piece* board[][8], bool BM[8][8], char sym)
{

	for (int r = 0; r < 8; r++)
	{
		for (int c = 0; c < 8; c++)
		{
			printboxMB(r, c, board, BM, sym);
		}
	}

}
void removeprintboxMB(int row, int col, Piece* board[][8], bool BM[8][8], char sym)
{

	hideConsoleCursor();
	int sr = 10 * row;
	int sc = 10 * col;
	for (int r = 0; r < 10; r++)
	{
		for (int c = 0; c < 10; c++)
		{
			if (!BM[row][col])
			{
				color(6);
				if (r == 0 or r == 9 or c == 0 or c == 9)
				{
					gotoRowCol(sr + r, sc + c);
					cout << sym;
				}
			}
			color(6);
		}
	}
}
void removeBOARDBM(Piece* board[][8], bool BM[8][8], char sym)
{

	for (int r = 0; r < 8; r++)
	{
		for (int c = 0; c < 8; c++)
		{
			removeprintboxMB(r, c, board, BM, sym);
		}
	}

}
void removehighlight(bool BM[8][8])
{
	for (int i = 0;i < 8;i++)
	{
		for (int j = 0;j < 8;j++)
		{
			BM[i][j] = false;
		}
	}
}
void Highlightcomp(Piece* board[][8], Color turn, POSITION S, bool BM[8][8])
{
	hideConsoleCursor();
	for (int i = 0;i < 8;i++)
	{
		for (int j = 0;j < 8;j++)
		{
			BM[i][j] = false;
		}
	}
	for (int i = 0;i < 8;i++)
	{
		for (int j = 0;j < 8;j++)
		{
			POSITION TD = { i,j };
			if (isvaliddestination(board, TD, turn) and board[S.row][S.col]->isvalidmove and board[S.row][S.col]->isvalidmove(S, TD, board) and !isselfcheck(board, S, TD, turn))
			{
				BM[i][j] = true;
			}
		}
	}

}
void printRownumber()
{
	for (int i = 0;i < 8;i++)
	{
		color(14);
		gotoRowCol(i * 10 + 4, 80);
		cout << (8 - i);
	}
}
void printcolALpha()
{
	char ch = 'A';
	for (int i = 0;i < 8;i++)
	{
		gotoRowCol(80, i * 10 + 4);
		cout << ch++;
	}
}
POSITION foundking(Piece* board[8][8], Color turn)
{
	char king = (turn == White) ? 'K' : 'k';
	POSITION D;
	D.row = -1;
	D.col = -1;
	for (int i = 0;i < 8;i++)
	{
		for (int j = 0;j < 8;j++)
		{
			if (board[i][j] != nullptr)
			{

				if (turn == board[i][j]->C and board[i][j]->sym == king)
				{
					D.row = i;
					D.col = j;
					return D;
				}
			}
		}
	}
	return D;
}
bool ischeck(Piece* board[8][8], Color turn)
{
	POSITION D = foundking(board, turn);
	if (D.row == -1)
	{
		return true;
	}
	Color enemy = (turn == White) ? Black : White;
	POSITION s;
	for (int i = 0;i < 8;i++)
	{
		for (int j = 0;j < 8;j++)
		{
			s.row = i;
			s.col = j;
			if (isvalidsource(board, s, enemy) and board[i][j]->isvalidmove)
			{
				if ( board[i][j]->isvalidmove and (board[i][j]->isvalidmove(s, D, board) ))
				{
					return true;
				}
			}

		}
	}
	return false;
}
bool isselfcheck(Piece* board[8][8], POSITION S, POSITION D, Color turn)
{
    bool check=false;
	Piece* SI = board[S.row][S.col];
	Piece* DI = board[D.row][D.col];
	board[D.row][D.col] = SI;
	board[S.row][S.col] = new Piece{ '-',Black,{S.row,S.col},0,nullptr,nullptr };
     check = ischeck(board, turn);
	delete board[S.row][S.col];
	board[S.row][S.col] = SI;
	board[D.row][D.col] = DI;
	return check;
}
void playerinfo(player PS[])
{
	char name[50];
	for (int i = 0;i < 2;i++)
	{
		cout << "Enter  " << (Color(i) == 0 ? "Black" : "white") << "  player name : ";
		cin >> name;
		PS[i].sym = new char[strlen(name) + 1];
		strcpy_s(PS[i].sym, strlen(name) + 1, name);

		PS[i].Pc = Color(i);
		cout << endl;
	}
}
void savegame(Piece* board[8][8], player PS[], Color turn)
{
	ofstream wrt("loadedfile1.txt");
	if (!wrt)
		return;
	wrt << turn << endl;
	wrt << PS[White].sym << endl;
	wrt << PS[Black].sym << endl;
	for (int i = 0;i < 8;i++)
	{
		for (int j = 0;j < 8;j++)
		{
			wrt << board[i][j]->sym;
		}
		wrt << endl;
	}
	wrt.close();
}
void MANUbox(int sr, int sc, int end)
{
	color(4);
	char sym = -37;
	for (int r = 0; r < end; r++)
	{
		for (int c = 0; c < end * 2; c++)
		{
			if (r == 0 or c == 0 or r == end - 1 or c == end * 2 - 1)
			{
				color(41);
				gotoRowCol(sr + r, sc + c);
				cout << sym;
			}
		}
	}
	color(4);
}
void manu(Piece* board[8][8], player PS[], Color& turn, bool& load)
{
	POSITION Box = {};
	while (true)
	{
		gotoRowCol(17, 53);
		cout << "Start New Game !";
		MANUbox(13, 50, 10);
		gotoRowCol(30, 53);
		cout << "Start Old Game !";
		MANUbox(26, 50, 10);
		gotoRowCol(45, 53);
		cout << "Quit the Game !";
		MANUbox(40, 50, 10);
		getRowColbyLeftClick(Box.row, Box.col);
		if (Box.row >= 13 and Box.row <= 23 and Box.col >= 50 and Box.col <= 70)
		{
			init(board, PS, turn);
			return;
		}
		else if (Box.row >= 26 and Box.row <= 36 and Box.col >= 50 and Box.col <= 70)
		{
			loadgame(board, PS, turn);
			load = true;
			return;
		}
		else if (Box.row >= 40 and Box.row <= 50 and Box.col >= 50 and Box.col <= 70)
		{
			exit(0);
		}
	}
}
bool isstalementandcheckmate(Piece* board[8][8], Color turn)
{
	POSITION S, D;
	for (int i = 0;i < 8;i++)
	{
		for (int j = 0;j < 8;j++)
		{
			S.row = i, S.col = j;
			if (isvalidsource(board, S, turn))
			{
				for (int k = 0;k < 8;k++)
				{
					for (int l = 0;l < 8;l++)
					{
						D.row = k, D.col = l;
						if ((isvaliddestination(board, D, turn) and board[S.row][S.col]->isvalidmove and board[S.row][S.col]->isvalidmove(S, D, board) and !isselfcheck(board, S, D, turn)))
						{
							return false;
						}
					}
				}
			}
		}
	}
	return true;
}
void redrawbox(Piece* board[][8], int r, int c)
{
	printbox(10 * r, 10 * c, r, c, board);
}
void savereplayboard(Piece* board[][8])
{
	replaysize = replyindex;
	for (int i = 0;i < 8;i++)
	{
		for (int j = 0;j < 8;j++)
		{
			Replay[replaysize][i][j].sym = board[i][j]->sym;
			Replay[replaysize][i][j].C = board[i][j]->C;
		}
	}
	replyindex++;
	replaysize = replyindex;
}
void loadreplayboard(int k, Piece* board[][8])
{
	for (int i = 0;i < 8;i++)
	{
		for (int j = 0;j < 8;j++)
		{
			if (board[i][j] != nullptr)
			{
				delete board[i][j];
			}
			board[i][j] = nullptr;
		}
	}


	for (int i = 0;i < 8;i++)
	{
		for (int j = 0;j < 8;j++)
		{
			char s = Replay[k][i][j].sym;
			Color c = Replay[k][i][j].C;
			board[i][j] = new Piece{ s,c,{i,j},0,nullptr,nullptr };
			assignsym(board[i][j]);
		}
	}

}
void displayreplaycoard(Piece* board[][8])
{
	char sym;
	cout << "Do you want replay of game (Enter R)";
	cin >> sym;
	if (sym == 'R' or sym == 'r')
	{

		for (int i = 0;i < replaysize;i++)
		{
			system("cls");
			loadreplayboard(i, board);
			BOARD(board);
			printcolALpha();
			printRownumber();
			Sleep(1000);
		}
	}
}
void undo(Piece* board[8][8], Color& turn)
{
	replyindex--;
	loadreplayboard(replyindex - 1, board);
	changeturn(turn);
	system("cls");
	BOARD(board);
	printcolALpha();
	printRownumber();
}
void redo(Piece* board[][8], Color& turn)
{
	loadreplayboard(replyindex, board);
	replyindex++;
	changeturn(turn);
	system("cls");
	BOARD(board);
	printcolALpha();
	printRownumber();
}
AImove getAImove(Piece* board[8][8], Color AIcolor)
{
	AImove bestmove ;
	bestmove.S={-1,-1};
	bestmove.D={-1,-1};
	int maxvalue=-1,bestpiecevalue=1000;
	POSITION S;
	POSITION D;
	for (int Sr = 0;Sr < 8;Sr++)
	{
		for (int Sc = 0;Sc < 8;Sc++)
		{
			S={Sr,Sc};
			if (isvalidsource(board, S, AIcolor))
			{
				for (int Dr = 0;Dr < 8;Dr++)
				{
					for (int Dc = 0;Dc < 8;Dc++)
					{
						D = { Dr,Dc };
						if (isvaliddestination(board, D, AIcolor)and board[S.row][S.col]->isvalidmove and board[S.row][S.col]->isvalidmove(S,D,board) and !isselfcheck(board,S,D,AIcolor))
						{
							if (board[D.row][D.col]->sym != '-' and board[D.row][D.col]->C != AIcolor)
							{
								int value=board[D.row][D.col]->value;
								if (value > maxvalue)
								{
									maxvalue=value;
									bestmove.S=S;
									bestmove.D=D;
								}
							}
						}
					}
				}
			}
		}
	}
	if(bestmove.S.row==-1)
	{ 
	for (int Sr = 0;Sr < 8;Sr++)
	{
		for (int Sc = 0;Sc < 8;Sc++)
		{
			S = { Sr,Sc };
			if (isvalidsource(board, S, AIcolor))
			{
				for (int Dr = 0;Dr < 8;Dr++)
				{
					for (int Dc = 0;Dc < 8;Dc++)
					{
						D = { Dr,Dc };
						if (isvaliddestination(board, D, AIcolor) and board[S.row][S.col]->isvalidmove and board[S.row][S.col]->isvalidmove(S, D, board) and !isselfcheck(board, S, D, AIcolor))
						{
							int value2 = board[S.row][S.col]->value;
							if (value2 < bestpiecevalue)
							{
							  if(!isselfcheck(board, S, D, AIcolor))
							    {
								  bestpiecevalue = value2;
								  bestmove.S = S;
								  bestmove.D = D;
							     }
							}
						}
					}
				}
			}
		}
	}
	}
	return bestmove;
}
bool onlyKingsLeft(Piece* board[8][8])
{
	int count = 0;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (tolower(board[i][j]->sym) != '-')
				count++;
		}
	}

	return count == 2; 
}
const char* boardToFEN(Piece* board[8][8], Color turn)
{
	static char fen[200];
	int idx = 0;

	for (int r = 7; r >= 0; r--)
	{
		int empty = 0;
		for (int c = 0; c < 8; c++)
		{
			char s = board[r][c]->sym;

			if (s == '-')
			{
				empty++;
			}
			else
			{
				if (empty > 0)
				{
					fen[idx++] = char('0' + empty);
					empty = 0;
				}
				fen[idx++] = s; 
			}
		}
		if (empty > 0)
			fen[idx++] = char('0' + empty);

		if (r != 0)
			fen[idx++] = '/';
	}

	
	fen[idx++] = ' ';
	fen[idx++] = (turn == White) ? 'w' : 'b';

	
	fen[idx++] = ' ';
	fen[idx++] = 'K';
	fen[idx++] = 'Q';
	fen[idx++] = 'k';
	fen[idx++] = 'q';
	fen[idx++] = ' ';
	fen[idx++] = '-';
	fen[idx++] = ' ';
	fen[idx++] = '0';
	fen[idx++] = ' ';
	fen[idx++] = '1';
	fen[idx] = '\0';
	return fen;
}
int main1()
{
	system("cls");
	hideConsoleCursor();
	Piece* Board[8][8];
	player Ps[2];
	Color turn;
	POSITION s, d;
	bool BM[8][8] = { false };
	char sym = -37;
	bool load = false;
	_getch();
	manu(Board, Ps, turn, load);
	system("cls");
	if (!load)
	{
		playerinfo(Ps);
	}
	system("cls");
	BOARD(Board);
	savereplayboard(Board);
	printcolALpha();
	printRownumber();
	if (!stockfish_init()) {
		cout << "Stockfish failed to start.\n";
		return 0;
	}

	while (true)
	{

		hideConsoleCursor();
		turnmsgdisplay(turn, Ps);
		if (turn == White)
		{
			const char* fen = boardToFEN(Board, turn);

			int sri=0, sci=0, dri=0, dci=0; char PawnPromoteTo='q';
			POSITION S1={}, D1={};
			char move[32];
			if (stockfish_bestmove(fen, 200, move)) {
				bestmove_parser(move, sri, sci, dri, dci, PawnPromoteTo);
				sri = 7 - sri;
				dri = 7 - dri;
			S1.row = sri;
			S1.col = sci;
			D1.row = dri;
			D1.col = dci;
			}
			updateboard(Board, S1, D1);
			redrawbox(Board, S1.row, S1.col);
			redrawbox(Board, D1.row, D1.col);
			savereplayboard(Board);
			printcolALpha();
			printRownumber();
		}
	    else
		{ 
		

			do {
				do {
					do
					{
						color(9);
						Selectpiece(s);
					} while (!isvalidsource(Board, s, turn));
					removehighlight(BM);
					removeBOARDBM(Board, BM, -37);
					Highlightcomp(Board, turn, s, BM);
					BOARDBM(Board, BM, -37);
					clearline(9, 81);
					gotoRowCol(9, 81);
					selectdestination(d);
				} while (!isvaliddestination(Board, d, turn));
			} while (!Board[s.row][s.col]->isvalidmove(s, d, Board) or isselfcheck(Board, s, d, turn));
			updateboard(Board, s, d);
			savegame(Board, Ps, turn);
			removehighlight(BM);
			removeBOARDBM(Board, BM, -37);
			redrawbox(Board, s.row, s.col);
			redrawbox(Board, d.row, d.col);
			savereplayboard(Board);
		}
		Color enemy = (turn == White) ? Black : White;
		if (ischeck(Board, enemy))
		{
			changeturn(turn);
			POSITION K=foundking(Board,turn);
			specialbox(K.row,K.col);
			gotoRowCol(14,84);
			color(6);
			cout<<"Check !"<<endl;
			changeturn(turn);
		}
		if (isstalementandcheckmate(Board, enemy))
		{
			system("cls");
			if (ischeck(Board, enemy))
			{
				cout<<"Checkmate ";
			}
			else
			{
				cout<<"Draw by stalement !";
			}
			break;
		}
		if (onlyKingsLeft(Board))
		{
			system("cls");
			cout<<"Draw ! the game";
			break;
		}
		changeturn(turn);
		turnmsgdisplay(turn,Ps);
		if (_kbhit())
		{
			char ch =_getch();
			if(ch=='U' or ch=='u') 
			   undo(Board,turn);
			if(ch=='R' or ch=='r')
				redo(Board,turn);

		}

	}
	cout << "player" << ((turn == 0) ? White : Black) << "winner\n";
	displayreplaycoard(Board);
	for (int i = 0; i < 8;i++)
		for (int j = 0;j < 8;j++)
			delete Board[i][j];
	for (int i = 0;i < 2;i++)
		delete[] Ps[i].sym;

	return 0;
}
int main2()
{
	system("cls");
	hideConsoleCursor();
	Piece* Board[8][8];
	player Ps[2];
	Color turn;
	POSITION s, d;
	bool BM[8][8] = { false };
	char sym = -37;
	bool load = false;
	_getch();
	manu(Board, Ps, turn, load);
	system("cls");
	if (!load)
	{
		playerinfo(Ps);
	}
	system("cls");
	BOARD(Board);
	savereplayboard(Board);
	printcolALpha();
	printRownumber();
	if (!stockfish_init()) {
		cout << "Stockfish failed to start.\n";
		return 0;
	}

	while (true)
	{

		hideConsoleCursor();
		turnmsgdisplay(turn, Ps);
		if (turn == White)
		{
			AImove M = getAImove(Board, AIcolor);
			if (M.S.row != -1)
			{
				updateboard(Board, M.S, M.D);
				savereplayboard(Board);
				savegame(Board, Ps, turn);
				BOARD(Board);
				if (isstalementandcheckmate(Board, turn))
				{
					if (ischeck(Board, turn))
					{
						system("cls");
						cout << "Checkmate! AI wins\n";
					}
					else
					{
						system("cls");
						cout << "Draw Game \n";
					}
					break;
				}

			}
		}
		else
		{


			do {
				do {
					do
					{
						color(9);
						Selectpiece(s);
					} while (!isvalidsource(Board, s, turn));
					removehighlight(BM);
					removeBOARDBM(Board, BM, -37);
					Highlightcomp(Board, turn, s, BM);
					BOARDBM(Board, BM, -37);
					clearline(9, 81);
					gotoRowCol(9, 81);
					selectdestination(d);
				} while (!isvaliddestination(Board, d, turn));
			} while (!Board[s.row][s.col]->isvalidmove(s, d, Board) or isselfcheck(Board, s, d, turn));
			updateboard(Board, s, d);
			savegame(Board, Ps, turn);
			removehighlight(BM);
			removeBOARDBM(Board, BM, -37);
			redrawbox(Board, s.row, s.col);
			redrawbox(Board, d.row, d.col);
			savereplayboard(Board);
		}
		Color enemy = (turn == White) ? Black : White;
		if (ischeck(Board, enemy))
		{
			changeturn(turn);
			POSITION K = foundking(Board, turn);
			specialbox(K.row, K.col);
			gotoRowCol(14, 84);
			color(6);
			cout << "Check !" << endl;
			changeturn(turn);
		}
		if (isstalementandcheckmate(Board, enemy))
		{
			system("cls");
			if (ischeck(Board, enemy))
			{
				cout << "Checkmate ";
			}
			else
			{
				cout << "Draw by stalement !";
			}
			break;
		}
		if (onlyKingsLeft(Board))
		{
			system("cls");
			cout << "Draw ! the game";
			break;
		}
		changeturn(turn);
		turnmsgdisplay(turn, Ps);
		if (_kbhit())
		{
			char ch = _getch();
			if (ch == 'U' or ch == 'u')
				undo(Board, turn);
			if (ch == 'R' or ch == 'r')
				redo(Board, turn);

		}

	}
	cout << "player" << ((turn == 0) ? White : Black) << "winner\n";
	displayreplaycoard(Board);
	for (int i = 0; i < 8;i++)
		for (int j = 0;j < 8;j++)
			delete Board[i][j];
	for (int i = 0;i < 2;i++)
		delete[] Ps[i].sym;

	return 0;
}
void manugame()
{
	cout<<"Enter 1 for human vs my AI"<<endl;
	cout<<"Enter 2 for human vs Stockfish AI"<<endl;
	cout<<"Enter choice :";
}
int main()
{
	int a;
	manugame();
	cin>>a;
	switch (a)
	{
		case 1:
		main1();
		break;
		case 2:
		main2();
		break;
	}
}