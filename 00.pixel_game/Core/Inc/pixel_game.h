#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <stdio.h>
#include "heart_pattern.h"
#include "main.h"

// 하드웨어 핀 정의
#define ROW_PINS 8
#define COL_PINS 8
#define JOYSTICK_X_PIN 9
#define JOYSTICK_Y_PIN 10
#define JOYSTICK_BUTTON_PIN 11

// 게임 보드 크기
#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8
#define MAX_SNAKE_LENGTH 64

// 조이스틱 임계값
#define JOYSTICK_THRESHOLD 300
#define JOYSTICK_CENTER 512

// 방향 정의
#define DIR_UP 0
#define DIR_RIGHT 1
#define DIR_DOWN 2
#define DIR_LEFT 3

// 게임 속도 설정
#define INITIAL_SPEED 500
#define TURBO_SPEED 150
#define MIN_SPEED 100
#define SPEED_INCREMENT 20

// 점수 설정
#define FOOD_SCORE 10
#define SPEED_LEVEL_THRESHOLD 5

// 타이밍 설정
#define INPUT_DEBOUNCE_MS 100
#define BLINK_RATE 5

// 좌표 구조체
typedef struct {
    int x, y;
} Point;

// 게임 상태 구조체
typedef struct {
    Point snake[MAX_SNAKE_LENGTH];
    int snake_length;
    int direction;
    int next_direction;
    Point food;
    int score;
    int game_over;
    unsigned long last_move_time;
    unsigned long last_input_time;
    int speed;
} GameState;

// 전역 변수 선언
extern GameState game_state;

// 하드웨어 제어 함수들
void init_hardware(void);
void set_led(int row, int col, int state);
int read_joystick_x(void);
int read_joystick_y(void);
int read_joystick_button(void);
unsigned long HAL_GetTick(void);
unsigned long get_time_ms(void);

// 유틸리티 함수들
void copy_memory(void* dest, const void* src, int size);
void clear_memory(void* ptr, int size);
void set_seed(unsigned long seed);
int simple_rand(void);
void simple_delay(int count);

// 게임 로직 함수들
void init_game(void);
void generate_food(void);
int is_snake_position(int x, int y);
int is_valid_direction(int current, int new_dir);
void handle_input(void);
void update_game(void);
void render_display(void);
void game_over_display(void);

// 시뮬레이션/테스트 함수
void simulate_input(void);

#endif // SNAKE_GAME_H
