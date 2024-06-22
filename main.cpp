#include <iostream>
#include <vector>
#include <raylib.h>
#include <omp.h> 
#include <math.h>

int WIDTH = 720;
int HEIGHT = 480;
int CELL_SIZE = 2;

const Color BACKGROUND_COLOR = DARKGRAY;
const Color CELL_COLOR = GREEN;
const Color GRID_COLOR = GRAY;

struct Cell
{
    bool isAlive;
    int neighbour_count;
    bool hasChanged;
};

void IncreaseNeighbourCounts(std::vector<std::vector<Cell>> &grid, int x, int y)
{
    for(int i = -1; i <= 1; i++){
        for(int j = -1; j <= 1; j++){
            if(i == 0 && j == 0){
                continue;
            }
            grid[(i + x + HEIGHT) % HEIGHT][(j + y + WIDTH) % WIDTH].neighbour_count++;
        }
    }
}

void DecreaseNeighbourCounts(std::vector<std::vector<Cell>> &grid, int x, int y)
{
    for(int i = -1; i <= 1; i++){
        for(int j = -1; j <= 1; j++){
            if(i == 0 && j == 0){
                continue;
            }
            grid[(i + x + HEIGHT) % HEIGHT][(j + y + WIDTH) % WIDTH].neighbour_count--;
        }
    }
}

void UpdateGrid(std::vector<std::vector<Cell>> &grid, std::vector<std::vector<Cell>> &new_grid,
                int width, int height)
{
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            new_grid[i][j] = {false, 0, false};
        }
    }

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            if((grid[i][j].isAlive && (grid[i][j].neighbour_count == 2 || grid[i][j].neighbour_count == 3)) ||
                (!grid[i][j].isAlive && grid[i][j].neighbour_count == 3)){
                new_grid[i][j].isAlive = true;
                if(!grid[i][j].isAlive){
                    new_grid[i][j].hasChanged = true;
                }
                IncreaseNeighbourCounts(new_grid, i, j);
            }
            else{
                if(grid[i][j].isAlive){
                    new_grid[i][j].hasChanged = true;
                }
            }
        }
    }
    grid = new_grid;
}

void InitializeGrid(std::vector<std::vector<Cell>> &grid)
{
    for(int i = 0; i < HEIGHT; i++)
    {
        for(int j = 0; j < WIDTH; j++)
        {
            int random = rand() % 2;
            grid[i][j] = {(bool)random, 0, false};
            if(random){
                IncreaseNeighbourCounts(grid, i, j);
            }
        }
    }
}

void DrawArray(const std::vector<std::vector<Cell>> &grid, bool show_grid)
{
    for (int i = 0; i < HEIGHT; i++) {
        int start = -1; // Track the start of a sequence of cells to be drawn
        for (int j = 0; j < WIDTH; j++) {
            if (grid[i][j].hasChanged || grid[i][j].isAlive) {
                if (start == -1) {
                    start = j; // Start of a new sequence
                }
            } else {
                if (start != -1) {
                    // End of a sequence, draw the rectangle
                    int length = j - start;
                    DrawRectangle(start * CELL_SIZE, i * CELL_SIZE, length * CELL_SIZE, CELL_SIZE, CELL_COLOR);
                    start = -1; // Reset the start position
                }
            }
        }
        // Check if there was an ongoing sequence at the end of the row
        if (start != -1) {
            int length = WIDTH - start;
            DrawRectangle(start * CELL_SIZE, i * CELL_SIZE, length * CELL_SIZE, CELL_SIZE, CELL_COLOR);
        }
    }
    
    if (show_grid) {
        float line_thickness = std::min(1.0f, CELL_SIZE * 0.5f); 
        
        // Draw grid lines with specified thickness
        for (int i = 0; i <= HEIGHT; i++) {
            DrawLineEx({0, static_cast<float>(i * CELL_SIZE)}, {static_cast<float>(WIDTH * CELL_SIZE), static_cast<float>(i * CELL_SIZE)}, line_thickness, GRID_COLOR); // Horizontal lines
        }
        for (int j = 0; j <= WIDTH; j++) {
            DrawLineEx({static_cast<float>(j * CELL_SIZE), 0}, {static_cast<float>(j * CELL_SIZE), static_cast<float>(HEIGHT * CELL_SIZE)}, line_thickness, GRID_COLOR); // Vertical lines
        }
    }
}

void DrawCell(std::vector<std::vector<Cell>> &grid, int x, int y)
{
    if(!grid[x][y].isAlive){
        grid[x][y].isAlive = true;
        grid[x][y].hasChanged = true;
        IncreaseNeighbourCounts(grid, x, y);
    }
}

void EraseCell(std::vector<std::vector<Cell>> &grid, int x, int y)
{
    if(grid[x][y].isAlive){
        grid[x][y].isAlive = false;
        grid[x][y].hasChanged = true;
        DecreaseNeighbourCounts(grid, x, y);
    }
}


int main(int argc, char* argv[])
{
    if(argc == 4){
        WIDTH = std::stoi(argv[1]);
        HEIGHT = std::stoi(argv[2]);
        CELL_SIZE = std::stoi(argv[3]);
        std::cout << "Using custom values: width = " << WIDTH << ", height = " << HEIGHT << ", cell_size = " << CELL_SIZE << std::endl;
    }
    else{
        std::cout << "Usage: " << argv[0] << " <width> <height> <cell_size>" << std::endl;
        std::cout << "Using default values: width = " << WIDTH << ", height = " << HEIGHT << ", cell_size = " << CELL_SIZE << std::endl;
    }

    InitWindow(WIDTH, HEIGHT, "Pdaloxd - Game Of Life");
    SetTargetFPS(60);

    std::vector<std::vector<Cell>> grid(HEIGHT, std::vector<Cell>(WIDTH, {false, 0, false}));
    std::vector<std::vector<Cell>> new_grid(HEIGHT, std::vector<Cell>(WIDTH, {false, 0, false}));

    InitializeGrid(grid);

    int screen_width = WIDTH / CELL_SIZE;
    int screen_height = HEIGHT / CELL_SIZE;
    int generation = 0;
    bool isPaused = false;
    bool show_grid = false;
    bool show_info = false;

    while(!WindowShouldClose()){
        if(IsKeyPressed(KEY_SPACE)){
            isPaused = !isPaused;
        }
        if(IsKeyPressed(KEY_S)){
            show_grid = !show_grid;
            show_info = !show_info;
        }

        // Mouse input handling
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) || IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
            Vector2 mousePosition = GetMousePosition();
            int cellX = mousePosition.x / CELL_SIZE;
            int cellY = mousePosition.y / CELL_SIZE;

            if (cellX >= 0 && cellX < screen_width && cellY >= 0 && cellY < screen_height) {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    DrawCell(grid, cellY, cellX);
                }
                if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)){
                    EraseCell(grid, cellY, cellX);
                }
            }
        }

        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        DrawArray(grid, show_grid);
        if(show_info){
            DrawText(TextFormat("FPS: %i", GetFPS()), WIDTH - 100, 10, 20, WHITE);
            DrawText(TextFormat("Generation: %i", generation), 10, 10, 20, WHITE);
        }
        EndDrawing();
        
        if(!isPaused){
            UpdateGrid(grid, new_grid, screen_width, screen_height);
            generation++;
        }
    }
    CloseWindow();
}