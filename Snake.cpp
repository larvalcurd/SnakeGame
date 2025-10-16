#include <iostream>
#include <vector>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

using Coord = std::pair<int, int>;

// --------------------- Constants ---------------------
constexpr int DEFAULT_WIDTH = 20;
constexpr int DEFAULT_HEIGHT = 20;
constexpr int INITIAL_SNAKE_LENGTH = 2;
constexpr int INITIAL_APPLE_OFFSET = 3;
constexpr int TICK_DELAY_MS = 40;
constexpr int TICKS_PER_MOVE = 5;

constexpr char KEY_UP = 'w';
constexpr char KEY_DOWN = 's';
constexpr char KEY_LEFT = 'a';
constexpr char KEY_RIGHT = 'd';

constexpr char WALL_CHAR = '#';
constexpr char SNAKE_CHAR = 'O';
constexpr char APPLE_CHAR = '@';
constexpr char EMPTY_CHAR = ' ';

constexpr char WIN_MESSAGE[] = "You Win!";
constexpr char LOSE_MESSAGE[] = "Game Over!";

// --------------------- Types ---------------------
enum Direction { UP, DOWN, LEFT, RIGHT };

struct GameState
{
    int width;
    int height;
    std::vector<Coord> snake;
    Coord apple;
    Direction dir;
    bool alive;
    std::vector<std::vector<char>> field;
};

// --------------------- Functional Utilities ---------------------
Coord MoveCoord(Coord p, Direction dir)
{
    switch (dir) {
    case UP:    p.second--; break;
    case DOWN:  p.second++; break;
    case LEFT:  p.first--;  break;
    case RIGHT: p.first++;  break;
    }
    return p;
}

bool IsInsideField(const GameState& state, Coord p)
{
    return p.first > 0 && p.second > 0 && p.first < state.width - 1 && p.second < state.height - 1;
}

bool IsCellFree(const GameState& state, Coord p)
{
    if (!IsInsideField(state, p)) return false;
    for (auto segment : state.snake)
        if (segment == p) return false;
    return true;
}

bool HasEatenApple(Coord head, Coord apple) { return head == apple; }

bool IsCollision(const GameState& state)
{
    Coord head = state.snake.front();
    if (!IsInsideField(state, head)) return true;
    for (size_t i = 1; i < state.snake.size(); ++i)
        if (head == state.snake[i]) return true;
    return false;
}

bool IsWin(const GameState& state)
{
    return state.snake.size() >= (state.width - 2) * (state.height - 2);
}

Coord GenerateApple(const GameState& state)
{
    while (true) {
        Coord p{ rand() % (state.width - 2) + 1, rand() % (state.height - 2) + 1 };
        if (IsCellFree(state, p)) return p;
    }
}

// --------------------- Terminal Utilities ---------------------
void ClearScreen()
{
    std::cout << "\x1b[2J";
}

void MoveCursorBelowGame(const GameState& state)
{
    std::cout << "\x1b[" << state.height + 1 << ";1H";
}

void PrintEndMessage(const GameState& state)
{
    std::cout << (IsWin(state) ? WIN_MESSAGE : LOSE_MESSAGE) << "\n";
}

void DrawCell(Coord c, char ch)
{
    std::cout << "\x1b[" << c.second + 1 << ";" << c.first + 1 << "H" << ch;
}

// --------------------- Game State Updates ---------------------
void InitField(GameState& state)
{
    state.field.assign(state.height, std::vector<char>(state.width, EMPTY_CHAR));
}

void PlaceWalls(GameState& state)
{
    for (int x = 0; x < state.width; x++) {
        state.field[0][x] = WALL_CHAR;
        state.field[state.height - 1][x] = WALL_CHAR;
    }
    for (int y = 1; y < state.height - 1; y++) {
        state.field[y][0] = WALL_CHAR;
        state.field[y][state.width - 1] = WALL_CHAR;
    }
}

void PlaceSnake(GameState& state)
{
    for (auto s : state.snake) state.field[s.second][s.first] = SNAKE_CHAR;
}

void PlaceApple(GameState& state)
{
    state.field[state.apple.second][state.apple.first] = APPLE_CHAR;
}

GameState InitGame(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT)
{
    GameState state;
    state.width = width;
    state.height = height;
    state.alive = true;
    state.dir = RIGHT;

    InitField(state);

    int midX = width / 2, midY = height / 2;
    for (int i = 0; i < INITIAL_SNAKE_LENGTH; i++)
        state.snake.push_back({ midX - i, midY });

    state.apple = { midX + INITIAL_APPLE_OFFSET, midY };

    ClearScreen();
    return state;
}

// --------------------- Snake Operations ---------------------
Coord GetNewHead(const GameState& state) { return MoveCoord(state.snake.front(), state.dir); }
void GrowSnake(GameState& state, Coord head) { state.snake.insert(state.snake.begin(), head); }
void MoveTail(GameState& state) { state.snake.pop_back(); }
void EatApple(GameState& state) { state.apple = GenerateApple(state); }

// --------------------- Game Update ---------------------
void UpdateGame(GameState& state)
{
    Coord newHead = GetNewHead(state);

    GrowSnake(state, newHead);

    if (HasEatenApple(newHead, state.apple))
        EatApple(state);
    else
        MoveTail(state);

    state.alive = !IsCollision(state);
    if (IsWin(state)) state.alive = false;


    DrawCell(state.snake.front(), SNAKE_CHAR);
    if (!HasEatenApple(newHead, state.apple))
        DrawCell(state.snake.back(), EMPTY_CHAR);
    DrawCell(state.apple, APPLE_CHAR);
}

// --------------------- Input ---------------------
Direction HandleInput(Direction current)
{
    if (_kbhit()) {
        char key = _getch();
        switch (key) {
        case KEY_UP:    if (current != DOWN)  return UP; break;
        case KEY_DOWN:  if (current != UP)    return DOWN; break;
        case KEY_LEFT:  if (current != RIGHT) return LEFT; break;
        case KEY_RIGHT: if (current != LEFT)  return RIGHT; break;
        }
    }
    return current;
}

// --------------------- Game Loop ---------------------
void WaitNextFrame() { std::this_thread::sleep_for(std::chrono::milliseconds(TICK_DELAY_MS)); }

void RunGame(GameState& state)
{
    InitField(state);
    PlaceWalls(state);
    PlaceSnake(state);
    PlaceApple(state);
    for (int y = 0; y < state.height; y++) {
        for (int x = 0; x < state.width; x++)
            std::cout << state.field[y][x];
        std::cout << "\n";
    }

    while (state.alive)
    {
        state.dir = HandleInput(state.dir);
        static int tick = 0;
        if (++tick % TICKS_PER_MOVE == 0)
            UpdateGame(state);

        WaitNextFrame();
    }

    MoveCursorBelowGame(state);
    PrintEndMessage(state);
}

// --------------------- Entry Point ---------------------
int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    GameState game = InitGame();
    RunGame(game);
}
