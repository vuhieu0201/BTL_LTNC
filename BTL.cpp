#include <SDL2/SDL.h>
#include <vector>
#include <iostream>

// Hằng số
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 20;
const int BALL_SIZE = 10;
const int BRICK_WIDTH = 60;
const int BRICK_HEIGHT = 20;
const int FALL_SPEED = 2; // Tốc độ rơi của các viên gạch
const int MAX_HITS = 3;   // Số lần viên gạch trúng bệ đỡ bóng trước khi thua

// Cấu trúc để định nghĩa một viên gạch
struct Brick {
    SDL_Rect rect;
    bool isActive;
    bool isFalling;
    int fallSpeed; // Tốc độ rơi
};

// Hàm vẽ văn bản (đơn giản)
void drawText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) {
    // Để đơn giản, ví dụ này không bao gồm việc vẽ văn bản
    // Trong thực tế, sử dụng thư viện SDL_ttf để vẽ văn bản
}

// Hàm vẽ menu chính
void renderMainMenu(SDL_Renderer* renderer) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color blue = {0, 0, 255, 255};
    SDL_Rect titleRect = { WINDOW_WIDTH / 2 - 150, 50, 300, 50 };
    SDL_Rect modeRect = { WINDOW_WIDTH / 2 - 150, 150, 300, 50 };

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Xóa màn hình với màu đen
    SDL_RenderClear(renderer);

    drawText(renderer, "Breakout Game", titleRect.x, titleRect.y, white);
    drawText(renderer, "1. Basic Mode", modeRect.x, modeRect.y, blue);
    drawText(renderer, "2. Advanced Mode", modeRect.x, modeRect.y + 50, blue);

    SDL_RenderPresent(renderer);
}

// Hàm vẽ văn bản (cài đặt giả, thay thế bằng việc vẽ văn bản thực sự)
void renderText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) {
    // Hàm giả cho đơn giản
}

// Hàm hiển thị màn hình kết thúc trò chơi
void displayGameOver(SDL_Renderer* renderer) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Rect gameOverRect = { WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50, 200, 100 };

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Xóa màn hình với màu đen
    SDL_RenderClear(renderer);
    renderText(renderer, "Game Over!", gameOverRect.x, gameOverRect.y, white);
    SDL_RenderPresent(renderer);
}

// Hàm chính
int main(int argc, char* argv[]) {
    // Khởi tạo SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL không thể khởi tạo! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Tạo cửa sổ và renderer
    SDL_Window* window = SDL_CreateWindow("Breakout Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Không thể tạo cửa sổ! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Không thể tạo renderer! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Vòng lặp menu chính
    bool inMenu = true;
    bool running = true;
    int gameMode = 0; // 1 cho Chế độ Cơ bản, 2 cho Chế độ Nâng cao
    SDL_Event event;
    
    while (inMenu) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
                inMenu = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_1) {
                    gameMode = 1; // Chế độ Cơ bản
                    inMenu = false;
                } else if (event.key.keysym.sym == SDLK_2) {
                    gameMode = 2; // Chế độ Nâng cao
                    inMenu = false;
                }
            }
        }
        renderMainMenu(renderer);
    }

    // Khởi tạo các đối tượng trò chơi
    SDL_Rect paddle = { WINDOW_WIDTH / 2 - PADDLE_WIDTH / 2, WINDOW_HEIGHT - PADDLE_HEIGHT - 10, PADDLE_WIDTH, PADDLE_HEIGHT };
    SDL_Rect ball = { WINDOW_WIDTH / 2 - BALL_SIZE / 2, WINDOW_HEIGHT / 2 - BALL_SIZE / 2, BALL_SIZE, BALL_SIZE };
    int ballVelocityX = 4, ballVelocityY = -4;

    std::vector<Brick> bricks;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 5; ++j) {
            Brick brick;
            brick.rect = { i * (BRICK_WIDTH + 5) + 20, j * (BRICK_HEIGHT + 5) + 20, BRICK_WIDTH, BRICK_HEIGHT };
            brick.isActive = true;
            brick.isFalling = (gameMode == 2); // Bật tính năng rơi trong chế độ nâng cao
            brick.fallSpeed = FALL_SPEED;
            bricks.push_back(brick);
        }
    }

    // Thiết lập màu sắc
    SDL_Color paddleColor = { 0, 255, 0, 255 };
    SDL_Color ballColor = { 255, 0, 0, 255 };
    SDL_Color brickColor = { 0, 0, 255, 255 };

    // Biến trạng thái trò chơi
    int hitsOnPaddle = 0;

    // Vòng lặp chính của trò chơi
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Di chuyển bệ đỡ bóng
        const Uint8* state = SDL_GetKeyboardState(nullptr);
        if (state[SDL_SCANCODE_LEFT]) {
            paddle.x -= 5;
            if (paddle.x < 0) paddle.x = 0;
        }
        if (state[SDL_SCANCODE_RIGHT]) {
            paddle.x += 5;
            if (paddle.x + PADDLE_WIDTH > WINDOW_WIDTH) paddle.x = WINDOW_WIDTH - PADDLE_WIDTH;
        }

        // Di chuyển viên bóng
        ball.x += ballVelocityX;
        ball.y += ballVelocityY;

        // Va chạm của viên bóng với các bức tường
        if (ball.x <= 0 || ball.x + BALL_SIZE >= WINDOW_WIDTH) {
            ballVelocityX = -ballVelocityX;
        }
        if (ball.y <= 0) {
            ballVelocityY = -ballVelocityY;
        }
        if (ball.y + BALL_SIZE >= WINDOW_HEIGHT) {
            ball = { WINDOW_WIDTH / 2 - BALL_SIZE / 2, WINDOW_HEIGHT / 2 - BALL_SIZE / 2, BALL_SIZE, BALL_SIZE };
            ballVelocityX = 4;
            ballVelocityY = -4;
        }

        // Va chạm của viên bóng với bệ đỡ bóng
        if (SDL_HasIntersection(&ball, &paddle)) {
            ballVelocityY = -ballVelocityY;
        }

        // Va chạm của viên bóng với viên gạch
        for (auto& brick : bricks) {
            if (brick.isActive && SDL_HasIntersection(&ball, &brick.rect)) {
                ballVelocityY = -ballVelocityY;
                brick.isActive = false;
                if (brick.isFalling) {
                    brick.fallSpeed = FALL_SPEED; // Bắt đầu rơi
                }
            }
        }

        // Cập nhật vị trí của các viên gạch đang rơi
        for (auto& brick : bricks) {
            if (!brick.isActive && brick.isFalling) {
                brick.rect.y += brick.fallSpeed;
                if (brick.rect.y + BRICK_HEIGHT >= paddle.y && brick.rect.x + BRICK_WIDTH >= paddle.x && brick.rect.x <= paddle.x + PADDLE_WIDTH) {
