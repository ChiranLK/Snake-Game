#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

#define WIDTH 40
#define HEIGHT 20

int x, y, fruitX, fruitY, score;
int tailX[100], tailY[100];
int nTail = 0;
int gameOver = 0;

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
enum Direction dir;

struct termios orig_termios;

// Disable buffered input
void disableBuffering() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Restore terminal
void restoreBuffering() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

int kbhit() {
    fd_set fds;
    struct timeval tv = {0, 0};
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

// Clear screen
void clearScreen() {
    printf("\033[2J\033[1;1H");
}

void setup() {
    disableBuffering();
    atexit(restoreBuffering);

    x = WIDTH / 2;
    y = HEIGHT / 2;

    srand(time(NULL));
    fruitX = rand() % WIDTH;
    fruitY = rand() % HEIGHT;

    score = 0;
    nTail = 0;
    dir = STOP;
}

void draw() {
    clearScreen();

    for (int i = 0; i < WIDTH + 2; i++) printf("\033[1;34m#\033[0m"); // Blue walls
    printf("\n");

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (j == 0) printf("\033[1;34m#\033[0m"); // Blue wall

            if (i == y && j == x)
                printf("\033[1;32mO\033[0m");  // Green snake head
            else if (i == fruitY && j == fruitX)
                printf("\033[1;31mF\033[0m");  // Red fruit
            else {
                int printed = 0;
                for (int k = 0; k < nTail; k++) {
                    if (tailX[k] == j && tailY[k] == i) {
                        printf("\033[0;32mo\033[0m"); // Light green tail
                        printed = 1;
                        break;
                    }
                }
                if (!printed) printf(" ");
            }

            if (j == WIDTH - 1) printf("\033[1;34m#\033[0m"); // Blue wall
        }
        printf("\n");
    }

    for (int i = 0; i < WIDTH + 2; i++) printf("\033[1;34m#\033[0m"); // Blue walls
    printf("\n");

    printf("\033[1;33mScore: %d\033[0m\n", score); // Yellow score
}

// Arrow keys + WASD input
void input() {
    if (!kbhit()) return;

    char c = getchar();

    if (c == '\033') {
        getchar(); // skip [
        switch (getchar()) {
            case 'A': dir = UP; break;    // Arrow Up
            case 'B': dir = DOWN; break;  // Arrow Down
            case 'C': dir = RIGHT; break; // Arrow Right
            case 'D': dir = LEFT; break;  // Arrow Left
        }
    } else {
        switch (c) {
            case 'w': dir = UP; break;
            case 's': dir = DOWN; break;
            case 'a': dir = LEFT; break;
            case 'd': dir = RIGHT; break;
            case 'x': gameOver = 1; break;
        }
    }
}

void logic() {
    int prevX = tailX[0];
    int prevY = tailY[0];
    int prev2X, prev2Y;

    tailX[0] = x;
    tailY[0] = y;

    for (int i = 1; i < nTail; i++) {
        prev2X = tailX[i];
        prev2Y = tailY[i];
        tailX[i] = prevX;
        tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }

    switch (dir) {
        case LEFT:  x--; break;
        case RIGHT: x++; break;
        case UP:    y--; break;
        case DOWN:  y++; break;
        default: break;
    }

    // Wall collision
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
        gameOver = 1;
        return;
    }

    // Snake hits its own tail
    for (int i = 0; i < nTail; i++)
        if (tailX[i] == x && tailY[i] == y)
            gameOver = 1;

    // Eat fruit
    if (x == fruitX && y == fruitY) {
        score += 10;
        fruitX = rand() % WIDTH;
        fruitY = rand() % HEIGHT;
        nTail++;
    }

    usleep(90000);
}

int main() {
    setup();

    while (!gameOver) {
        draw();
        input();
        logic();
    }

    clearScreen();
    printf("\033[1;31m\nGAME OVER!\033[0m\n"); // Red GAME OVER
    printf("\033[1;33mFinal Score: %d\033[0m\n\n", score); // Yellow score
    return 0;
}
