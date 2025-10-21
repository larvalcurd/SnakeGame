#include <iostream>
#include <deque>
#include <vector>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

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
constexpr const char* CLEAR_SCREEN_CMD = "\x1b[H";

// --------------------- Types ---------------------
using Point = std::pair<int, int>;
enum Direction { UP, DOWN, LEFT, RIGHT };

struct GameState
{
    int width;
    int height;
    std::deque<Point> snake;
    Point apple;
    Direction dir;
    bool alive;
    std::vector<std::vector<char>> field;
};

// --------------------- Utility ---------------------
bool IsHeadOutside(const GameState& state)
{
    const Point& head = state.snake.front();
    return head.first <= 0 || head.second <= 0 ||
        head.first >= state.width - 1 || head.second >= state.height - 1;
}

bool IsFree(const GameState& state, int x, int y)
{
    if (x <= 0 || y <= 0 || x >= state.width - 1 || y >= state.height - 1)
        return false;
    return state.field[y][x] == EMPTY_CHAR;
}

// --------------------- Apple ---------------------
Point GenerateApple(const GameState& state)
{
    while (true)
    {
        int x = rand() % (state.width - 2) + 1;
        int y = rand() % (state.height - 2) + 1;
        if (IsFree(state, x, y))
            return { x, y };
    }
}

// --------------------- Initialization ---------------------
void InitializeField(GameState& state, int width, int height)
{
    state.width = width;
    state.height = height;
    state.field = std::vector<std::vector<char>>(height, std::vector<char>(width, EMPTY_CHAR));
}

void InitializeSnake(GameState& state)
{
    int midX = state.width / 2;
    int midY = state.height / 2;
    for (int i = 0; i < INITIAL_SNAKE_LENGTH; ++i)
        state.snake.push_back({ midX - i, midY });
}

GameState InitGame(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT)
{
    GameState state;
    InitializeField(state, width, height);
    InitializeSnake(state);

    int midX = width / 2;
    int midY = height / 2;
    state.apple = { midX + INITIAL_APPLE_OFFSET, midY };
    state.dir = RIGHT;
    state.alive = true;

    return state;
}

// --------------------- Input ---------------------
Direction HandleInput(Direction currentDirection)
{
    if (_kbhit()) {
        char key = _getch();
        switch (key) {
            case KEY_UP:    if (currentDirection != DOWN)  return UP;    break;
            case KEY_DOWN:  if (currentDirection != UP)    return DOWN;  break;
            case KEY_LEFT:  if (currentDirection != RIGHT) return LEFT;  break;
            case KEY_RIGHT: if (currentDirection != LEFT)  return RIGHT; break;
        }
    }
    return currentDirection;
}

// --------------------- Snake Movement ---------------------
Point GetNextHeadPosition(const GameState& state)
{
    Point head = state.snake.front();
    switch (state.dir) {
        case UP:    head.second--; break;
        case DOWN:  head.second++; break;
        case LEFT:  head.first--;  break;
        case RIGHT: head.first++;  break;
    }
    return head;
}

void MoveSnake(GameState& state)
{
    Point newHead = GetNextHeadPosition(state);
    state.snake.push_front(newHead);

    if (newHead.first == state.apple.first && newHead.second == state.apple.second)
        state.apple = GenerateApple(state);
    else
        state.snake.pop_back();
}

// --------------------- Collisions ---------------------
bool IsSelfCollision(const GameState& state)
{
    const Point& head = state.snake.front();
    for (size_t i = 1; i < state.snake.size(); ++i)
        if (head == state.snake[i])
            return true;
    return false;
}

bool IsWallCollision(const GameState& state)
{
    return IsHeadOutside(state);
}

bool IsWin(const GameState& state)
{
    return state.snake.size() >= (state.width - 2) * (state.height - 2);
}

// --------------------- Game Logic ---------------------
void UpdateGameState(GameState& state)
{
    MoveSnake(state);
    if (IsWallCollision(state) || IsSelfCollision(state) || IsWin(state))
        state.alive = false;
}

// --------------------- Rendering ---------------------
void ClearField(GameState& state)
{
    for (int y = 0; y < state.height; ++y)
        std::fill(state.field[y].begin(), state.field[y].end(), EMPTY_CHAR);
}

void DrawWalls(GameState& state)
{
    for (int x = 0; x < state.width; ++x) {
        state.field[0][x] = WALL_CHAR;
        state.field[state.height - 1][x] = WALL_CHAR;
    }
    for (int y = 1; y < state.height - 1; ++y) {
        state.field[y][0] = WALL_CHAR;
        state.field[y][state.width - 1] = WALL_CHAR;
    }
}

void DrawSnake(GameState& state)
{
    for (const auto& segment : state.snake)
        state.field[segment.second][segment.first] = SNAKE_CHAR;
}

void DrawApple(GameState& state)
{
    state.field[state.apple.second][state.apple.first] = APPLE_CHAR;
}

void RenderFrame(GameState& state)
{
    ClearField(state);
    DrawWalls(state);
    DrawSnake(state);
    DrawApple(state);
    std::cout << CLEAR_SCREEN_CMD;
    for (int y = 0; y < state.height; ++y) {
        for (int x = 0; x < state.width; ++x)
            std::cout << state.field[y][x];
        std::cout << "\n";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(TICK_DELAY_MS));
}

// --------------------- Game Loop ---------------------
void RunGameLoop(GameState& game)
{
    int tick = 0;
    while (game.alive)
    {
        game.dir = HandleInput(game.dir);
        if (++tick % TICKS_PER_MOVE == 0)
            UpdateGameState(game);
        RenderFrame(game);
    }
    std::cout << (IsWin(game) ? WIN_MESSAGE : LOSE_MESSAGE) << "\n";
}

// --------------------- Entry ---------------------
int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    GameState game = InitGame();
    RunGameLoop(game);
    return 0;
}
