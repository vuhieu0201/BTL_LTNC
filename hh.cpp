#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <iostream>
#include <cmath> // For std::sqrt

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 20;
const int BALL_RADIUS = 10;
const int BRICK_WIDTH = 60;
const int BRICK_HEIGHT = 20;
const int FALL_SPEED = 2;
const int MAX_HITS = 3;
const int MAX_LEVEL = 4;
const int INITIAL_LIVES = 3;

struct Brick {
    SDL_Rect rect;
    bool isActive;
    bool isFalling;
    int fallSpeed;
};

bool initFont() {
    if (TTF_Init() == -1) {
        std::cerr << "TTF Error: " << TTF_GetError() << std::endl;
        return false;
    }
    return true;
}

TTF_Font* loadFont(const std::string& fontPath, int fontSize) {
    TTF_Font* font = TTF_OpenFont(fontPath.c_str(), fontSize);
    if (!font) {
        std::cerr << "TTF Error: " << TTF_GetError() << std::endl;
    }
    return font;
}

void drawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) {
        std::cerr << "TTF Error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
    }

    SDL_Rect destRect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void drawCircle(SDL_Renderer* renderer, int x, int y, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int w = 0; w < radius * 2; ++w) {
        for (int h = 0; h < radius * 2; ++h) {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
    drawText(renderer, font, text, x, y, color);
}

void renderMainMenu(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color blue = {0, 0, 255, 255};
    SDL_Rect titleRect = { WINDOW_WIDTH / 2 - 150, 50, 300, 50 };
    SDL_Rect mode1Rect = { WINDOW_WIDTH / 2 - 150, 150, 300, 50 };
    SDL_Rect mode2Rect = { WINDOW_WIDTH / 2 - 150, 200, 300, 50 };
    SDL_Rect mode3Rect = { WINDOW_WIDTH / 2 - 150, 250, 300, 50 };

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    renderText(renderer, font, "Bricks&Ball", titleRect.x, titleRect.y, white);
    renderText(renderer, font, "Basic Mode", mode1Rect.x, mode1Rect.y, blue);
    renderText(renderer, font, "Advanced Mode", mode2Rect.x, mode2Rect.y, blue);
    renderText(renderer, font, "Two-Face Mode", mode3Rect.x, mode3Rect.y, blue);

    SDL_RenderPresent(renderer);
}

int handleMainMenuEvents(SDL_Event* event) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    SDL_Rect mode1Rect = { WINDOW_WIDTH / 2 - 150, 150, 300, 50 };
    SDL_Rect mode2Rect = { WINDOW_WIDTH / 2 - 150, 200, 300, 50 };
    SDL_Rect mode3Rect = { WINDOW_WIDTH / 2 - 150, 250, 300, 50 };

    if (event->type == SDL_MOUSEBUTTONDOWN) {
        if (mouseX >= mode1Rect.x && mouseX <= mode1Rect.x + mode1Rect.w &&
            mouseY >= mode1Rect.y && mouseY <= mode1Rect.y + mode1Rect.h) {
            return 1; // Basic Mode
        }
        if (mouseX >= mode2Rect.x && mouseX <= mode2Rect.x + mode2Rect.w &&
            mouseY >= mode2Rect.y && mouseY <= mode2Rect.y + mode2Rect.h) {
            return 2; // Advanced Mode
        }
        if (mouseX >= mode3Rect.x && mouseX <= mode3Rect.x + mode3Rect.w &&
            mouseY >= mode3Rect.y && mouseY <= mode3Rect.y + mode3Rect.h) {
            return 3; // Two-Face Mode
        }
    }
    return 0;
}

void displayGameOver(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Rect gameOverRect = { WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 50, 200, 50 };
    SDL_Rect menuRect = { WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 + 50, 300, 50 };

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    renderText(renderer, font, "Game Over!", gameOverRect.x, gameOverRect.y, white);
    renderText(renderer, font, "Press M for Menu", menuRect.x, menuRect.y, white);
    SDL_RenderPresent(renderer);
}

void initializeBricks(std::vector<Brick>& bricks, int gameMode, int level) {
    bricks.clear();
    int rows = level + 1; // Level 1 -> 2 rows, Level 2 -> 3 rows, etc.
    int columns = 12;

    int topPadding = gameMode == 3 ? PADDLE_HEIGHT + 50 : 20;

    for (int i = 0; i < columns; ++i) {
        for (int j = 0; j < rows; ++j) {
            Brick brick;
            brick.rect = { i * (BRICK_WIDTH + 5) + 20, j * (BRICK_HEIGHT + 5) + topPadding, BRICK_WIDTH, BRICK_HEIGHT };
            brick.isActive = true;
            brick.isFalling = (gameMode == 2 || gameMode == 3); // Falling bricks in Advanced and Two-Face modes
            brick.fallSpeed = FALL_SPEED;
            bricks.push_back(brick);
        }
    }
}

bool checkLevelCompletion(const std::vector<Brick>& bricks) {
    for (const auto& brick : bricks) {
        if (brick.isActive) {
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
     if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (!initFont()) {
        SDL_Quit();
        return 1;
    }

     if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) == 0) {
        std::cerr << "Mix_Init Error: " << Mix_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Mix_OpenAudio Error: " << Mix_GetError() << std::endl;
        Mix_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Bricks&Ball", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load the font
    TTF_Font* font = loadFont("Sans.ttf", 24); // Update with your font path and size
    if (!font) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Load sounds
    Mix_Chunk* bounceSound = Mix_LoadWAV("bounce.wav");
    Mix_Chunk* breakSound = Mix_LoadWAV("break.wav");
    Mix_Music* backgroundMusic = Mix_LoadMUS("background.mp3");

    if (!bounceSound || !breakSound || !backgroundMusic) {
        std::cerr << "Mix_Load Error: " << Mix_GetError() << std::endl;
        Mix_FreeChunk(bounceSound);
        Mix_FreeChunk(breakSound);
        Mix_FreeMusic(backgroundMusic);
        Mix_Quit();
        SDL_Quit();
        return 1;
    }

    Mix_PlayMusic(backgroundMusic, -1); // Play background music

    bool running = true;
    int gameMode = 0;
    int level = 1; // Start at level 1 when selecting mode
    int score = 0; // Initialize score
    int lives = INITIAL_LIVES; // Initialize lives
    SDL_Event event;

    while (running) {
        bool inMenu = true;
        while (inMenu) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                    inMenu = false;
                } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_KEYDOWN) {
                    gameMode = handleMainMenuEvents(&event);
                    if (gameMode > 0) {
                        inMenu = false;
                        level = 1; // Start at level 1 when selecting mode
                        score = 0; // Reset score
                        lives = INITIAL_LIVES; // Reset lives
                    }
                }
            }
            renderMainMenu(renderer, font);
        }

        SDL_Rect paddleTop = { WINDOW_WIDTH / 2 - PADDLE_WIDTH / 2, 10, PADDLE_WIDTH, PADDLE_HEIGHT };
        SDL_Rect paddleBottom = { WINDOW_WIDTH / 2 - PADDLE_WIDTH / 2, WINDOW_HEIGHT - 50, PADDLE_WIDTH, PADDLE_HEIGHT };
        int ballVelocityX = 5;
        int ballVelocityY = -5;
        SDL_Rect ball = { WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, BALL_RADIUS * 2, BALL_RADIUS * 2 };
        SDL_Color ballColor = {255, 255, 255, 255};
        SDL_Color paddleColor = {255, 255, 255, 255};
        SDL_Color brickColor = {255, 0, 0, 255};
        SDL_Color scoreColor = {255, 255, 255, 255};
        SDL_Color livesColor = {255, 255, 255, 255};

        std::vector<Brick> bricks;
        initializeBricks(bricks, gameMode, level);

        int hitsOnPaddle = 0;

        while (running && lives > 0) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                } else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_m) {
                        running = false;
                        break;
                    }
                }
            }

            const Uint8* state = SDL_GetKeyboardState(NULL);
            const int PADDLE_MOVE_SPEED = 10; // Speed of paddle movement

            // Move the paddles
            if (state[SDL_SCANCODE_LEFT]) {
                paddleBottom.x -= PADDLE_MOVE_SPEED;
                if (paddleBottom.x < 0) paddleBottom.x = 0;
                if (gameMode == 3) {
                    paddleTop.x -= PADDLE_MOVE_SPEED;
                    if (paddleTop.x < 0) paddleTop.x = 0;
                }
            }
            if (state[SDL_SCANCODE_RIGHT]) {
                paddleBottom.x += PADDLE_MOVE_SPEED;
                if (paddleBottom.x + PADDLE_WIDTH > WINDOW_WIDTH) paddleBottom.x = WINDOW_WIDTH - PADDLE_WIDTH;
                if (gameMode == 3) {
                    paddleTop.x += PADDLE_MOVE_SPEED;
                    if (paddleTop.x + PADDLE_WIDTH > WINDOW_WIDTH) paddleTop.x = WINDOW_WIDTH - PADDLE_WIDTH;
                }
            }

            ball.x += ballVelocityX;
            ball.y += ballVelocityY;

            if (ball.x <= 0 || ball.x + BALL_RADIUS * 2 >= WINDOW_WIDTH) {
                ballVelocityX = -ballVelocityX;
            }
            if (ball.y <= 0) {
                ballVelocityY = -ballVelocityY;
            }

           

            if (ball.y + BALL_RADIUS * 2 >= WINDOW_HEIGHT) {
                lives--;
                if (lives <= 0) {
                    break;
                }
                ball = { WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, BALL_RADIUS * 2, BALL_RADIUS * 2 };
                ballVelocityX = 5;
                ballVelocityY = -5;
            }

            // Collision with the bottom paddle
            if (ball.x + BALL_RADIUS >= paddleBottom.x && ball.x <= paddleBottom.x + PADDLE_WIDTH &&
                ball.y + BALL_RADIUS * 2 >= paddleBottom.y) {
                ballVelocityY = -ballVelocityY;
            }

            // Collision with the top paddle (Two-Face Mode)
            if (gameMode == 3 && ball.x + BALL_RADIUS >= paddleTop.x && ball.x <= paddleTop.x + PADDLE_WIDTH &&
                ball.y <= paddleTop.y + PADDLE_HEIGHT) {
                ballVelocityY = -ballVelocityY;
            } else if (gameMode == 3 && ball.y < 0) { // Ball missed by the top paddle in Two-Face Mode
                lives--;
                if (lives <= 0) {
                    break;
                }
                ball = { WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, BALL_RADIUS * 2, BALL_RADIUS * 2 };
                ballVelocityX = 5;
                ballVelocityY = -5;
            }

            for (auto& brick : bricks) {
                if (!brick.isActive) continue;

                if (ball.x + BALL_RADIUS >= brick.rect.x && ball.x <= brick.rect.x + BRICK_WIDTH &&
                    ball.y + BALL_RADIUS >= brick.rect.y && ball.y <= brick.rect.y + BRICK_HEIGHT) {
                    ballVelocityY = -ballVelocityY;
                    brick.isActive = false;
                    score += 100;
                    Mix_PlayChannel(-1, breakSound, 0);

                    if (gameMode == 2 || gameMode == 3) {
                        brick.isFalling = true;
                    }
                }
            }

            for (auto& brick : bricks) {
                if (!brick.isActive && brick.isFalling) {
                    brick.rect.y += brick.fallSpeed;

                    if (brick.rect.y + BRICK_HEIGHT >= paddleBottom.y &&
                        brick.rect.x + BRICK_WIDTH >= paddleBottom.x &&
                        brick.rect.x <= paddleBottom.x + PADDLE_WIDTH) {

                        hitsOnPaddle++;
                        brick.isActive = false;
                        brick.isFalling = false;

                        lives--;
                        if (lives <= 0) {
                            break;
                        }

                        std::cout << "Hit: " << hitsOnPaddle << " times" << std::endl;

                        if (hitsOnPaddle >= MAX_HITS) {
                            level++;
                            if (level < MAX_LEVEL) {
                                initializeBricks(bricks, gameMode, level);
                            } else {
                                break;
                            }
                        }
                    }

                    if (brick.rect.y > WINDOW_HEIGHT) {
                        brick.isActive = false;
                        brick.isFalling = false;
                    }
                }
            }

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, paddleColor.r, paddleColor.g, paddleColor.b, paddleColor.a);
            SDL_RenderFillRect(renderer, &paddleBottom);
            if (gameMode == 3) {
                SDL_RenderFillRect(renderer, &paddleTop);
            }

            drawCircle(renderer, ball.x, ball.y, BALL_RADIUS, ballColor);

            for (const auto& brick : bricks) {
                if (brick.isActive || brick.isFalling) {
                    SDL_SetRenderDrawColor(renderer, brickColor.r, brickColor.g, brickColor.b, brickColor.a);
                    SDL_RenderFillRect(renderer, &brick.rect);
                }
            }

            renderText(renderer, font, "Score: " + std::to_string(score), 20, 20, scoreColor);
            renderText(renderer, font, "Lives: " + std::to_string(lives), WINDOW_WIDTH - 120, 20, livesColor);

            SDL_RenderPresent(renderer);

            SDL_Delay(16); // Approx. 60 FPS

            if (checkLevelCompletion(bricks)) {
                level++;
                if (level < MAX_LEVEL) {
                    initializeBricks(bricks, gameMode, level);
                } else {
                    break;
                }
            }
        }

        if (lives <= 0) {
            displayGameOver(renderer, font);
            SDL_Delay(2000); // Display game over screen for 2 seconds
            bool inEndMenu = true;
            while (inEndMenu) {
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        running = false;
                        inEndMenu = false;
                    } else if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_m) {
                            inEndMenu = false;
                        }
                    }
                }
            }
        }
    }

    Mix_FreeChunk(bounceSound);
    Mix_FreeChunk(breakSound);
    Mix_FreeMusic(backgroundMusic);
    Mix_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();


    return 0;
}
