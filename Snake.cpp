#include <iostream>
#include <vector>
#include <conio.h>   // For _kbhit() and _getch() - keyboard input
#include <cstdlib>   // For rand() and srand()
#include <ctime>     // For time()
#include <thread>    // For std::this_thread::sleep_for
#include <chrono>    // For std::chrono::milliseconds

// --------------------- Constants ---------------------
// Game configuration constants for easy tuning
constexpr int DEFAULT_WIDTH = 20;
constexpr int DEFAULT_HEIGHT = 20;
constexpr int INITIAL_SNAKE_LENGTH = 2;
constexpr int INITIAL_APPLE_OFFSET = 3;
constexpr int TICK_DELAY_MS = 40;      // Delay between frames in milliseconds
constexpr int TICKS_PER_MOVE = 5;      // Number of frames before each move

// Control keys (WASD)
constexpr char KEY_UP = 'w';
constexpr char KEY_DOWN = 's';
constexpr char KEY_LEFT = 'a';
constexpr char KEY_RIGHT = 'd';

// Display characters
constexpr char WALL_CHAR = '#';
constexpr char SNAKE_CHAR = 'O';
constexpr char APPLE_CHAR = '@';
constexpr char EMPTY_CHAR = ' ';

// End-game messages
constexpr char WIN_MESSAGE[] = "You Win!";
constexpr char LOSE_MESSAGE[] = "Game Over!";

constexpr const char* CLEAR_SCREEN_CMD = "\x1b[H";

// --------------------- Data Structures ---------------------

// Represents a point (x, y) on the game board
struct Point
{
    int x, y;
};

// Enum for possible snake directions
enum Direction { UP, DOWN, LEFT, RIGHT };

// Holds all the current game data (state)
struct GameState
{
    int width;                    // Width of the game board
    int height;                   // Height of the game board
    std::vector<Point> snake;     // Vector storing all snake segments
    Point apple;                  // Position of the apple
    Direction dir;                // Current direction of movement
    bool alive;                   // Whether the game is still running
    std::vector<std::vector<char>> field; // The actual game field [y][x]
};

// --------------------- Utility Functions ---------------------

// Returns true if the snake's head is outside the valid playing area
bool IsHeadOutside(const GameState& state)
{
    Point head = state.snake.front();
    return head.x <= 0 || head.y <= 0 || head.x >= state.width - 1 || head.y >= state.height - 1;
}

// Returns true if the cell (x, y) is free (not wall, not snake)
bool IsFree(const GameState& state, int x, int y)
{
    if (x <= 0 || y <= 0 || x >= state.width - 1 || y >= state.height - 1)
        return false;

    return state.field[y][x] == EMPTY_CHAR;
}

// --------------------- Core Game Logic ---------------------

// Randomly generates a new apple position that is not on the snake
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

// Initializes a new game with default or custom size
GameState InitGame(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT)
{
    GameState state;
    state.width = width;
    state.height = height;

    // Initialize the game field as a vector of vectors
    state.field = std::vector<std::vector<char>>(height, std::vector<char>(width, EMPTY_CHAR));

    int midX = width / 2;
    int midY = height / 2;

    // Create initial snake in the middle of the field
    for (int i = 0; i < INITIAL_SNAKE_LENGTH; ++i)
        state.snake.push_back({ midX - i, midY });

    state.apple = { midX + INITIAL_APPLE_OFFSET, midY };  // Place the first apple near the snake
    state.dir = RIGHT;                                    // Start moving to the right
    state.alive = true;
    return state;
}

// Reads keyboard input and updates direction if valid
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
    return currentDirection; // No input or invalid move — keep current direction
}

// Moves the snake one step forward in the current direction
GameState MoveSnake(const GameState& state)
{
    GameState nextState = state;
    Point head = state.snake.front();

    // Move the head depending on current direction
    switch (state.dir) {
    case UP:    head.y--; break;
    case DOWN:  head.y++; break;
    case LEFT:  head.x--; break;
    case RIGHT: head.x++; break;
    }

    // Insert the new head at the front
    nextState.snake.insert(nextState.snake.begin(), head);

    // If the snake eats the apple — grow; otherwise, remove the tail
    if (head.x == state.apple.x && head.y == state.apple.y)
        nextState.apple = GenerateApple(nextState);
    else
        nextState.snake.pop_back();

    return nextState;
}

// Returns true if the snake's head collides with its own body
bool IsSelfCollision(const GameState& state)
{
    Point head = state.snake.front();
    for (size_t i = 1; i < state.snake.size(); ++i)
        if (head.x == state.snake[i].x && head.y == state.snake[i].y)
            return true;
    return false;
}

// Returns true if the snake's head touches the wall
bool IsWallCollision(const GameState& state)
{
    return IsHeadOutside(state);
}

// Returns true if the snake filled the whole field (win condition)
bool IsWin(const GameState& state)
{
    return state.snake.size() >= (state.width - 2) * (state.height - 2);
}

// Performs one logical game tick (movement + collision checks)
GameState UpdateTick(const GameState& currentState)
{
    GameState nextState = MoveSnake(currentState);
    nextState.alive = !(IsWallCollision(nextState) || IsSelfCollision(nextState));

    // Stop the game if player wins
    if (IsWin(nextState))
        nextState.alive = false;

    return nextState;
}

// --------------------- Rendering Functions ---------------------

// Clears the entire field, filling it with EMPTY_CHAR
void ClearField(GameState& state)
{
    for (int y = 0; y < state.height; ++y)
        std::fill(state.field[y].begin(), state.field[y].end(), EMPTY_CHAR);
}

// Draws the border walls (#) around the playing field
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

// Draws the snake body (O) inside the field
void DrawSnake(GameState& state)
{
    for (const auto& segment : state.snake)
        state.field[segment.y][segment.x] = SNAKE_CHAR;
}

// Draws the apple (@) at its current position
void DrawApple(GameState& state)
{
    state.field[state.apple.y][state.apple.x] = APPLE_CHAR;
}

// Renders the complete game frame to the console
void Draw(GameState& state)
{
    std::cout << CLEAR_SCREEN_CMD; // Move cursor to top-left (to avoid flicker)
    for (int y = 0; y < state.height; ++y) {
        for (int x = 0; x < state.width; ++x)
            std::cout << state.field[y][x];
        std::cout << "\n";
    }
}

// Prints the appropriate message after the game ends
void PrintGameOverMessage(const GameState& state)
{
    if (IsWin(state))
        std::cout << WIN_MESSAGE << "\n";
    else
        std::cout << LOSE_MESSAGE << "\n";
}

// --------------------- Timing ---------------------

// Pauses the program between frames to control game speed
void WaitNextFrame()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TICK_DELAY_MS));
}

// Runs the main game loop with tick-based updates and frame delay
void RunGameLoop(GameState& game)
{
    int tick = 0;

    while (game.alive)
    {
        // Handle player input
        game.dir = HandleInput(game.dir);

        // Move the snake every few ticks
        tick++;
        if (tick % TICKS_PER_MOVE == 0)
            game = UpdateTick(game);

        // Clear the field and draw everything
        ClearField(game);
        DrawWalls(game);
        DrawSnake(game);
        DrawApple(game);
        Draw(game);

        // Wait for next frame
        WaitNextFrame();
    }

    PrintGameOverMessage(game);
}

// --------------------- Entry Point ---------------------
int main()
{
    srand(static_cast<unsigned int>(time(nullptr))); // Initialize random seed

    GameState game = InitGame(); // Create initial game state
    RunGameLoop(game);           // Run the game
}
