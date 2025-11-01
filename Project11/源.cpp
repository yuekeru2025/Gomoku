#include "graphics.h"
#include <vector>
#define ENABLE_SOUNDS 1
#if ENABLE_SOUNDS
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif
//宏定义文本
#ifdef _MSC_VER
#define TEXT_WINDOW_TITE "五子棋游戏"
#define TEXT_PLAYER_WIN "玩家胜利"
#define TEXT_AI_WIN "AI胜利"
#else


#endif
//定义棋子类型
enum PieceType
{
	EMPTY = 0,
	BLACK_PIECE = 1,
	WHITE_PIECE = 2,
};

enum GameState
{
	PLAYING,
	BLACK_WIN,
	WHITE_WIN,
	DRAW,
};

//定义游戏常量
const int BOARD_SIZE = 15;
const int CELL_SIZE = 32;
const int OFFSET_X = 50;
const int OFFSET_Y = 50;
const int WINDOW_WIDTH = OFFSET_X * 2 + BOARD_SIZE * CELL_SIZE;
const int WINDOW_HEIGHT = OFFSET_Y * 2 + BOARD_SIZE * CELL_SIZE + 100;


//定义类
class Gomoku
{
public:
	//构造函数
	Gomoku()
	{
		initGame(false);
#if ENABLE_SOUNDS
		midiOutOpen(&m_device, m_deviceID, 0, 0, CALLBACK_NULL);
#endif
	}
	//初始化游戏
	void initGame(bool toggleFirst)
	{
		//游戏状态
		gameState = PLAYING;
		//重置落子位置
		m_lastRow = -1;
		m_lastCol = -1;
		//让棋盘为空
		for (int i = 0; i < BOARD_SIZE; i++)
		{
			for (int j = 0; j < BOARD_SIZE; j++)
			{
				m_board[i][j] = EMPTY;
			}
		}
		//AI模式下切换先后手
		//根据游戏状态与先后手判断是否需要切换先后手
		//双人对战模式下，先手总是黑子
	}
	//画棋盘
	void drawBOARD()
	{
		//把背景色设置为纯木色
		setbkcolor_f(EGERGB(0xDE, 0xB8, 0x87));
		cleardevice();
		//设置线条
		setlinecolor(BLACK);
		setlinestyle(SOLID_LINE, 2);
		//画横线
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			line(OFFSET_X, OFFSET_Y + j * CELL_SIZE, OFFSET_X + (BOARD_SIZE - 1) * CELL_SIZE, OFFSET_Y + j * CELL_SIZE);
		}
		//画竖线
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			line(OFFSET_X + j * CELL_SIZE, OFFSET_Y, OFFSET_X + j * CELL_SIZE, OFFSET_Y + (BOARD_SIZE - 1) * CELL_SIZE);
		}
		//绘制天元与星位
		setcolor(BLACK);
		setfillcolor(BLACK);
		int circleX = OFFSET_X + (BOARD_SIZE / 2) * CELL_SIZE;
		int circleY = OFFSET_Y + (BOARD_SIZE / 2) * CELL_SIZE;
		fillcircle(circleX, circleY, 3);//天元比较重要，用3个像素
		int startPoint = 3;
		//左上
		fillcircle(OFFSET_X + startPoint * CELL_SIZE, OFFSET_Y + startPoint * CELL_SIZE, 2);
		//右上
		int ysX = OFFSET_X + (BOARD_SIZE - 1 - startPoint) * CELL_SIZE;
		fillcircle(ysX, OFFSET_Y + startPoint * CELL_SIZE, 2);
		//左下
		int zxY = OFFSET_Y + (BOARD_SIZE - 1 - startPoint) * CELL_SIZE;
		fillcircle(OFFSET_X + startPoint * CELL_SIZE, zxY, 2);
		//右下
		fillcircle(ysX, zxY, 2);

	}
	void handleClickMouse(int msgX, int msgY)
	{
		//判断是否有效，无效返回
		if (gameState != PLAYING)
		{
			return;
		}
		//如果轮到AI,忽略鼠标的点击
		if (m_vsAI)
		{
			PieceType AIpieceType = m_humanFirst ? WHITE_PIECE : BLACK_PIECE;
		}
		MouseToBoard(msgX, msgY);
		return;
	}
	void MouseToBoard(int msgX, int msgY)
	{
		int bX = (msgX - OFFSET_X + CELL_SIZE / 2) / CELL_SIZE;
		int bY = (msgY - OFFSET_Y + CELL_SIZE / 2) / CELL_SIZE;
		if (bX >= 0 && bX < BOARD_SIZE && bY >= 0 && bY < BOARD_SIZE)
		{
			PieceMove(bX, bY, m_currentPlayer);
		}
		return;
	}
	void PieceMove(int bX, int bY, PieceType piece)
	{
		printf("棋子(% d, % d),棋子类型：%s", bX, bY, (piece == BLACK_PIECE) ? "黑子" : "白子");
		if (m_board[bX][bY] != EMPTY)
		{
			return;
		}
		else
		{
			//更新棋盘的状态
			m_board[bX][bY] = piece;
			//更新最后落子
			m_lastRow = bX;
			m_lastCol = bY;
			//drawPiece(bX, bY, piece);
			pdWinner(bX, bY, piece);
		}
		//不断切换当前玩家
		m_currentPlayer = (m_currentPlayer == BLACK_PIECE) ? WHITE_PIECE : BLACK_PIECE;
		PieceType aiPiece = m_humanFirst ? WHITE_PIECE : BLACK_PIECE;
		bool isturnAI = (m_currentPlayer == aiPiece);
		//在AI模式下并且轮到AI的时候，才aiMove()
		if (m_vsAI && isturnAI)
		{
			aiMove();
		}
		return;
	}
	//画棋子
	void drawPiece()
	{
		ege_enable_aa(true);
		for (int i = 0; i < BOARD_SIZE; i++)
		{
			for (int j = 0; j < BOARD_SIZE; j++)
			{
				if (m_board[i][j] != EMPTY)
				{
					int x = j * CELL_SIZE + OFFSET_X;
					int y = i * CELL_SIZE + OFFSET_Y;
					int r = CELL_SIZE / 2 - 2;
					if (m_board[i][j] == BLACK_PIECE)
					{
						setcolor(EGERGB(96, 96, 96));
						setfillcolor(EGERGB(0, 0, 0));
						fillcircle(x, y, r);
					}
					if (m_board[i][j] == WHITE_PIECE)
					{
						setcolor(EGERGB(192, 192, 192));
						setfillcolor(EGERGB(255, 255, 255));
						fillcircle(x, y, r);
					}
				}
			}
		}
		//判断是否为最后一颗落子,是就得画小红十字//不仅仅是m_lastRow与m_lastCol是i,j，还得判断是否赢了最后一颗棋子。
		for (int i = 0; i < BOARD_SIZE; i++)
		{
			for (int j = 0; j < BOARD_SIZE; j++)
			{
				if (m_lastRow == i && m_lastCol == j)
				{
					setlinecolor(RED);
					setlinewidth(2);
					int circleCenterX = OFFSET_X + i * CELL_SIZE;
					int circleCenterY = OFFSET_Y + j * CELL_SIZE;
					line(circleCenterX - (CELL_SIZE / 2 - 2), circleCenterY, circleCenterX + (CELL_SIZE / 2 - 2), circleCenterY);
					line(circleCenterX, circleCenterY - (CELL_SIZE / 2 - 2), circleCenterX, circleCenterY + (CELL_SIZE / 2 - 2));
				}
			}
		}
		ege_enable_aa(false);

		return;
	}

	//简单AI,
	void aiMove()
	{
		if (gameState != PLAYING)
		{
			return;
		}
		//判断是否轮到AI
		int bestScore = -1000;//不知道为什么要搞个1000？//防御性编程技巧。极小值的初始化。防止如果所有得分都是负，没有比-1小的数来返回，导致出错。
		int bestRow = -1;
		int bestCol = -1;

		for (int i = 0; i < BOARD_SIZE; i++)
		{
			for (int j = 0; j < BOARD_SIZE; j++)
			{
				if (m_board[i][j] == EMPTY)
				{
					int score = evaluePosition(i, j);

					if (score > bestScore)
					{
						bestScore = score;
						bestRow = i;
						bestCol = j;
					}
				}
			}
		}
		if (bestRow != -1 && bestCol != -1)
		{

			PieceMove(bestRow, bestCol, m_currentPlayer);
			return;
		}
		/*
		for (int i = 0; i < BOARD_SIZE; i++)
		{
			for (int j = 0; j < BOARD_SIZE; j++)
			{
				if (m_board[i][j] == EMPTY)
				{
					PieceMove(i, j, m_currentPlayer);
					return;
				}
			}
		}*/

	}
	//评估位置得分
	int evaluePosition(int Row, int Col)
	{
		int score = 0;
		if (m_board[Row][Col] == EMPTY)
		{
			int center = BOARD_SIZE / 2;
			score += 1000 - (abs(Row - center) + abs(Col - center));
			PieceType aiPiece = m_humanFirst ? WHITE_PIECE : BLACK_PIECE;
			PieceType humPiece = m_humanFirst ? BLACK_PIECE : WHITE_PIECE;
			score += evalueDirection(Row, Col, aiPiece);
			//攻击得分：评估AI可以连珠的可能性
			score += evalueDirection(Row, Col, humPiece);
			//防守得分：阻止玩家连珠
		}
		return score;
	}
	//评估方向得分

	int evalueDirection(int Row, int Col, PieceType piece)
	{
		int score = 0;
		//定义方向向量，水平、垂直、主对角线、副对角线
		int direction[4][2] = { {0, 1},{1, 0},{1, 1},{1, -1} };
		for (int a = 0; a < 4; a++)
		{
			bool blockRight = false, blockLeft = false;
			int count = 1;//本身这个位置也可以算作一个
			int rowC = direction[a][0];
			int colC = direction[a][1];
			int rowFb = Row;
			int colFb = Col;
			//正向的
			while (rowFb >= 0 && rowFb < BOARD_SIZE && colFb >= 0 && colFb < BOARD_SIZE)
			{
				rowFb += rowC;
				colFb += colC;
				if (m_board[rowFb][colFb] == piece)
				{
					count++;
				}
				else
				{
					if (m_board[rowFb][colFb] != EMPTY)
					{
						blockRight = true;
					}
					break;
				}
			}
			//反方向的
			while (rowFb >= 0 && rowFb <= BOARD_SIZE && colFb >= 0 && colFb <= BOARD_SIZE)
			{
				rowFb -= rowC;
				colFb -= colC;
				if (m_board[rowFb][colFb] == piece)
				{
					count++;
				}
				else
				{
					if (m_board[rowFb][colFb] != EMPTY)
					{
						blockLeft = true;
					}
					break;
				}
			}
			score += evalueShape(count, blockLeft, blockRight);
		}
		//根据连续性与是否被阻挡来判断分数

		return score;
	}

	int evalueShape(int count, bool blockLeft, bool blockRight)
	{
		int score = 0;
		int isblock = (blockLeft ? 1 : 0) + (blockRight ? 1 : 0);
		switch (count) {
		case 5: return 100000;
		case 4: if (isblock == 0) return 10000;
			if (isblock == 1) return 1000;
			return 0;
		case 3: if (isblock == 0) return 500;
			if (isblock == 1) return 100;
			return 0;
		case 2: if (isblock == 0) return 50;
			if (isblock == 1) return 10;
			return 0;
		case 1: if (isblock == 0) return 5;
		default:
			return 0;
		}
		return score;
	}
	void pdWinner(int bX, int bY, PieceType piece)
	{
		int direction[4][2] = { {1, 0}, {0, 1}, {1, 1}, {-1, 1} };
		for (int a = 0; a < 4; a++)
		{
			int rowC = direction[a][0];
			int colC = direction[a][1];
			int count = 1;
			int r = bX;
			int c = bY;
			//正方向统计
			while (r < BOARD_SIZE && c < BOARD_SIZE)
			{
				r += rowC;
				c += colC;
				if (m_board[r][c] == piece)
				{
					count++;
				}
				else
				{
					break;
				}
			}
			//反向统计
			r = bX;
			c = bY;
			while (r >= 0 && c >= 0)
			{
				r -= rowC;
				c -= colC;
				if (m_board[r][c] == piece)
				{
					count++;
				}
				else
				{
					break;
				}
			}
			if (count >= 5)
			{
				m_winner = piece;
				gameState = (piece == BLACK_PIECE) ? BLACK_WIN : WHITE_WIN;//更新游戏状态
				const char* winnerInfo = (piece == BLACK_PIECE) ? TEXT_PLAYER_WIN : TEXT_AI_WIN;
				putWinner(winnerInfo);
			}
		}
		return;
	}
	//输出获胜者信息
	void putWinner(const char* winnerInfo)
	{
		int width = 400;
		int height = 300;
		m_endImage = newimage(width, height);
		settarget(m_endImage);
		// 绘制背景矩形
		setcolor(BLACK);
		setfillcolor(WHITE);
		setlinewidth(3);
		fillrect(0, 0, width, height);
		rectangle(0, 0, width, height);
		//输出字
		setfont(60, 0, "宋体");
		setcolor(BLACK);
		int textWidth = textwidth(winnerInfo);
		int textHeight = textheight(winnerInfo);
		int textX = (width - textWidth) / 2;
		int textY = (height - textHeight) / 2;
		setbkmode(TRANSPARENT);
		outtextxy(textX, textY, winnerInfo);
		settarget(nullptr);
		int winX = (WINDOW_WIDTH - width) / 2;
		int winY = (WINDOW_HEIGHT - height) / 2;
		ege_setalpha(200, m_endImage);
		putimage_withalpha(NULL, m_endImage, winX, winY);
		return;
	}
	


	GameState getGameState() const { return gameState; }
	PieceType getcurrentPlayer() const { return m_currentPlayer; }



private:
	int m_lastRow{};
	int m_lastCol{};
	PieceType m_board[BOARD_SIZE][BOARD_SIZE];
	bool m_vsAI = true;
	PieceType m_currentPlayer = BLACK_PIECE;
	PieceType m_winner = {};
	PIMAGE m_endImage = nullptr;

#if ENABLE_SOUNDS
	HMIDIOUT m_device{};
	UINT m_deviceID = 0;
#endif
	GameState gameState{};
	bool m_humanFirst = true;
};
int main()
{
	//初始化窗口
	initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

	Gomoku game;
	game.drawBOARD();
	
	while (true)
	{
		game.drawPiece();
		while (mousemsg())
		{
			mouse_msg msg = getmouse();
			if (msg.is_left() && msg.is_down())
			{
				printf("鼠标点击位置: (%d, %d)\n", msg.x, msg.y);  // 调试输出
				game.handleClickMouse(msg.x, msg.y);
				
			}
		}
		if (kbhit())
		{
			int key = getchar();
			if (key == 27)
			{
				break;//按esc键才退出
			}
		}
		delay_fps(60);
	}
	closegraph();
}


/*
10_22:
反思了一下我自己打的代码与被给代码之前的区别与优劣。
1.被给代码在使用一个函数的时候，总是先去判断函数是否调用成功，才进行别的。
2.被给代码总是会注意到很多。处理可能的意外情况。比如MakeMove()这个函数中在placePiece()完成之后，会判断棋盘是否满了。mouseToBoard()这个函数
不仅仅是把坐标转换，而且处理意外情况，比如用户点击没有点击的棋盘内部，点击到格子中心，没有点击到对应位置等。

*/
//我就不信怎么还不行？