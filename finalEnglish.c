//Final Version
//My name is Xuedi Liu,you can call me lxd.My number is 1752985
//My English no good hohoho

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define START "START"
#define PLACE "PLACE"
#define TURN  "TURN"
#define END   "END"

// board information
#define BOARD_SIZE 12
#define EMPTY 0
#define BLACK 1
#define WHITE 2

// option
typedef int OPTION;
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define UP_LEFT 4
#define UP_RIGHT 5
#define DOWN_LEFT 6
#define DOWN_RIGHT 7

// bool
typedef int BOOL;
#define TRUE 1
#define FALSE 0


int meFlag, otherFlag;
int X, Y, K;
int step;
clock_t start, finish;

int board[12][12] = { 0 };
int BoardStatus[12][12] = { 0 };
int NumberOfChess[3] = { 0,8,8 };
const int DIR[8][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };
const int interventionDir[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };// Pick dir
const int custodianDir[8][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };// Clamp dir
int shallowScore[120] = { 0 };

struct Command {
	int x;
	int y;
	OPTION option;
};
struct SortScore {
	int x;
	int y;
	OPTION option;
	int score;
};
struct SortScore stepScore[5][120] = { 0,0,0,0 };//x,y,option,score

BOOL IsInBound(int x, int y) {
	return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

BOOL IsDifferent(int flag, int x, int y) {
	return IsInBound(x, y) && board[x][y] == 3 - flag;
}

BOOL IsSame(int flag, int x, int y) {
	return IsInBound(x, y) && board[x][y] == flag;
}

//Print Board
void PrintBoard() {
	printf("  ");
	for (int i = 0; i < 12; i++)
		printf("%2d", i);
	printf("\n");
	for (int i = 0; i < 12; i++) {
		printf("%2d", i);
		for (int j = 0; j < 12; j++) {
			switch (board[i][j]) {
			case 0: {
				printf("  ");
				break;
			}
			case 1: {
				printf("¡ð");
				break;
			}
			case 2: {
				printf("¡ñ");
				break;
			}
			}
		}
		printf("\n");
	}
}

//Pick (new can pick
void PickNumber(int x, int y, int tempFlag) {
	for (int i = 0; i < 4; i++) {
		int x1 = x + interventionDir[i][0];
		int y1 = y + interventionDir[i][1];
		int x2 = x - interventionDir[i][0];
		int y2 = y - interventionDir[i][1];
		if (IsInBound(x1, y1) && IsInBound(x2, y2) && board[x1][y1] == 3 - tempFlag && board[x2][y2] == 3 - tempFlag) {
			BoardStatus[x1][y1] = 1;
			BoardStatus[x2][y2] = 1;
		}
	}
}

//Clamp (new can clamp
void ClampNumber(int x, int y, int tempFlag) {
	for (int i = 0; i < 8; i++) {
		int x1 = x + custodianDir[i][0];
		int y1 = y + custodianDir[i][1];
		int x2 = x + custodianDir[i][0] * 2;
		int y2 = y + custodianDir[i][1] * 2;
		if (IsInBound(x1, y1) && IsInBound(x2, y2) && board[x2][y2] == tempFlag && board[x1][y1] == 3 - tempFlag) {
			BoardStatus[x1][y1] = 1;
		}
	}
}

//number of new can eat
int EatNumber(int x, int y, int flag) {
	int eatNum = 0;
	for (int i = 0; i < 8; i++) {
		if (IsInBound(x + DIR[i][0], y + DIR[i][1]) && BoardStatus[x + DIR[i][0]][y + DIR[i][1]] == 1) {
			eatNum++;
			board[x + DIR[i][0]][y + DIR[i][1]] = flag;//change color
		}
	}
	memset(BoardStatus, 0, sizeof(BoardStatus));
	return eatNum;
}

//move chess and change the Number
int Place(int flag, int x, int y, int k) {
	//step ++;
	int eatNum = 0;
	board[x][y] = 0;
	int newX = x + DIR[k][0];
	int newY = y + DIR[k][1];
	board[newX][newY] = flag;
	PickNumber(newX, newY, flag);
	ClampNumber(newX, newY, flag);
	eatNum = EatNumber(newX, newY, flag);//count eat number and change color
	NumberOfChess[flag] += eatNum;
	NumberOfChess[3 - flag] -= eatNum;
	return eatNum;
}


//Value Function
//When using value function 
//your board is the status after 4 times move

//Find the number of dangerous chess
//larger worse
int FindPickClamp(int i, int j, int flag) {
	int dangerNum = 0;
	for (int k = 0; k < 8; k++) {
		int newX = i + DIR[k][0], newY = j + DIR[k][1];//EMPTY
		int newX1 = i + 2 * DIR[k][0], newY1 = j + 2 * DIR[k][1];//pick place
		int newX2 = i - DIR[k][0], newY2 = j - DIR[k][1];//clamp place
		//pick dangerous
		if (IsSame(flag, newX1, newY1) && board[newX][newY] == 0) {
			for (int k = 0; k < 8; k++) {
				if (IsDifferent(flag, newX + DIR[k][0], newY + DIR[k][1])) {
					dangerNum++;
					break;
				}
			}
			break;
		}//clamp dangerous
		else if (IsDifferent(flag, newX2, newY2) && board[newX][newY] == 0) {
			for (int k = 0; k < 8; k++) {
				if (IsDifferent(flag, newX + DIR[k][0], newY + DIR[k][1])) {
					dangerNum++;
					break;
				}
			}
			break;
		}
	}
	return dangerNum;
}

//get together to BLACK£¨6,5£©WHITE£¨6,7£©
//larger worse
int Together(int i, int j, int flag) {
	int score = 0;
	int deltaX = 0, deltaY = 0;
	if (flag == 1) {
		deltaX = abs(i - 6);
		deltaY = abs(j - 5);
	}
	else if (flag == 2) {
		deltaX = abs(i - 6);
		deltaY = abs(j - 7);
	}
	score = (deltaX + 1)*(deltaY + 1);
	return score;
}

//ues for the last 2 steps:
//if now your newx newy can eat
//return the number of now can eat
int FindNowEat(int x, int y, int flag) {
	int eatNum = 0;
	PickNumber(x, y, flag);
	ClampNumber(x, y, flag);
	for (int i = 0; i < 8; i++) {
		if (IsInBound(x + DIR[i][0], y + DIR[i][1]) && BoardStatus[x + DIR[i][0]][y + DIR[i][1]] == 1) {
			eatNum++;
		}
	}
	memset(BoardStatus, 0, sizeof(BoardStatus));
	return eatNum;
}

//Value return to Alphabeta
int Value(int x, int y, int flag) {
	int score = 0;
	int pickClamp = 0;
	int togetherScore = 0;
	int souroundNum = 0;
	int nowEat = FindNowEat(x, y, flag);//the new step can eat
	int meNum = NumberOfChess[flag], otherNum = NumberOfChess[3 - flag];
	for (int i = 0; i < 12; i++) {
		for (int j = 0; j < 12; j++) {
			if (board[i][j] == flag) {
				pickClamp += FindPickClamp(i, j, flag);
				togetherScore += Together(i, j, flag);
			}
			else if (board[i][j] == 3 - flag) {
				pickClamp -= 5 * FindPickClamp(i, j, 3 - flag);
			}
		}
	}
	if (step >= 118) {
		score = 1000 * meNum - 500 * otherNum + 300 * nowEat;
	}
	else {
		score = 1000 * meNum - 500 * otherNum - togetherScore - 120 * pickClamp + 400 * nowEat;
	}
	return score;
}

//before search, simply calculate the score of eatch option
//sort these option from high to low in order to speed up alphabeta
int ShallowSort(int flag, int depth) {
	int newx, newy;
	int n = 0;
	int eat = 0;
	int boardCopy[12][12];
	memcpy(boardCopy, board, sizeof(board));//copy board
	//save the avalible options and score
	for (int k = 7; k >= 0; k--) {
		for (int i = 0; i < 12; i++) {
			for (int j = 0; j < 12; j++) {
				if (board[i][j] == flag) {
					newx = i + DIR[k][0], newy = j + DIR[k][1];
					if (IsInBound(newx, newy) && board[newx][newy] == 0) {
						stepScore[depth][n].x = i;
						stepScore[depth][n].y = j;
						stepScore[depth][n].option = k;
						if (depth == 0) {//only score when depth == 0
							eat = Place(meFlag, i, j, k);
							stepScore[depth][n].score = Value(newx, newy, flag);
							NumberOfChess[meFlag] -= eat;
							NumberOfChess[otherFlag] += eat;
						}
						n++;
						memcpy(board, boardCopy, sizeof(boardCopy));
					}
				}
			}
		}
	}
	if (depth == 0) {
		struct SortScore tempSort = { 0,0,0,0 };
		for (int i = 0; i < n - 1; i++) {
			for (int j = n; j > i; j--) {
				if (stepScore[depth][j - 1].score < stepScore[depth][j].score) {
					tempSort = stepScore[depth][j];
					stepScore[depth][j] = stepScore[depth][j - 1];
					stepScore[depth][j - 1] = tempSort;
				}
			}
		}
	}
	return n;
}

//alphabeta search with shallow sort
int AlphaBeta(int depth, int flag, int alpha, int beta) {
	int newx = 0, newy = 0;
	finish = clock();
	if (depth == 5) {
		return Value(newx, newy, meFlag);
	}
	int able = 0;
	int boardCopy[12][12], score, eat;
	int i, j, k, n = 0;
	memcpy(boardCopy, board, sizeof(board));
	if (flag == meFlag) {
		n = ShallowSort(meFlag, depth);//all able step
		for (int m = 0; m < n; m++) {
			i = stepScore[depth][m].x;
			j = stepScore[depth][m].y;
			k = stepScore[depth][m].option;
			newx = stepScore[depth][m].x + DIR[k][0], newy = stepScore[depth][m].y + DIR[k][1];
			if (IsInBound(newx, newy) && board[newx][newy] == 0) {
				able = 1;
				eat = Place(meFlag, i, j, k);
				score = AlphaBeta(depth + 1, otherFlag, alpha, beta);
				if (score > alpha) {
					alpha = score;
					if (depth == 0) {
						X = i;
						Y = j;
						K = k;
					}
				}
				memcpy(board, boardCopy, sizeof(boardCopy));
				NumberOfChess[meFlag] -= eat;
				NumberOfChess[otherFlag] += eat;
				if (alpha >= beta) {
					return alpha;
				}
			}
		}
		if (able == 0) {
			return -100000;
		}
		return alpha;
	}
	else {
		n = ShallowSort(otherFlag, depth);
		for (int m = 0; m < n; m++) {
			i = stepScore[depth][m].x;
			j = stepScore[depth][m].y;
			k = stepScore[depth][m].option;
			newx = stepScore[depth][m].x + DIR[k][0], newy = stepScore[depth][m].y + DIR[k][1];
			if (IsInBound(newx, newy) && board[newx][newy] == 0) {
				able = 1;
				eat = Place(otherFlag, i, j, k);
				score = AlphaBeta(depth + 1, meFlag, alpha, beta);
				if (score < beta)
					beta = score;
				memcpy(board, boardCopy, sizeof(boardCopy));
				NumberOfChess[otherFlag] -= eat;
				NumberOfChess[meFlag] += eat;
				if (beta <= alpha)
					return beta;
			}
		}
		if (able == 0) {
			return 100000;
		}
		return beta;
	}
}

//Fixed Start
BOOL HumanHash() {
	if (step == 0) {
		X = 9;
		Y = 7;
		K = 4;
		Place(meFlag, X, Y, K);
		printf("%d %d %d\n", X, Y, K);
		fflush(stdout);
		return 1;
	}
	else if (step == 2 && board[7][7] == 2 && board[6][8] == 0) {
		X = 9;
		Y = 8;
		K = 5;
		Place(meFlag, X, Y, K);
		printf("%d %d %d\n", X, Y, K);
		fflush(stdout);
		return 1;
	}
	else
		return 0;
}

void Start(int flag) {
	memset(board, 0, sizeof(board));
	step = 0;
	for (int i = 0; i < 3; i++) {
		board[2][2 + i] = 2;
		board[6][6 + i] = 2;
		board[5][3 + i] = 1;
		board[9][7 + i] = 1;
	}

	for (int i = 0; i < 2; i++) {
		board[8 + i][2] = 2;
		board[2 + i][9] = 1;
	}
}

void Turn(int flag) {
	if (HumanHash())
		return;
	else {
		AlphaBeta(0, flag, -100000, 100000);
		Place(meFlag, X, Y, K);
		printf("%d %d %d\n", X, Y, K);
	}
	fflush(stdout);
}

void End(int x) {

}

void loop() {
	char buffer[100];
	while (TRUE) {
		memset(buffer, 0, sizeof(buffer));
		gets(buffer);
		if (strstr(buffer, START)) { //START
			char tmp[100] = { 0 };
			sscanf(buffer, "%s %d", tmp, &meFlag);
			otherFlag = 3 - meFlag;
			Start(meFlag);
			printf("OK\n");
			fflush(stdout);
		}
		else if (strstr(buffer, PLACE)) {
			char tmp[100] = { 0 };
			int x, y;
			int option;
			sscanf(buffer, "%s %d %d %d", tmp, &x, &y, &option);
			memset(stepScore, 0, sizeof(stepScore));
			Place(otherFlag, x, y, option);
			step++;
		}
		else if (strstr(buffer, TURN)) {
			start = clock();
			memset(stepScore, 0, sizeof(stepScore));
			Turn(meFlag);
			step++;
		}
		else if (strstr(buffer, END)) {
			char tmp[100] = { 0 };
			int x;
			sscanf(buffer, "%s %d", tmp, &x);
			End(x);
		}
		//printf("%d\n", step);
		printf("%d\n", finish - start);
		//PrintBoard();
	}
}

int main(int argc, char *argv[]) {
	loop();
	return 0;
}