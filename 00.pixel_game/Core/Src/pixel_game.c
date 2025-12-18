#include "pixel_game.h"

GameState game_state;
// 간단한 PRNG (Linear Congruential Generator)
static unsigned long rng_seed = 1;

void set_seed(unsigned long seed) {
    rng_seed = seed;
}

int simple_rand() {
    rng_seed = rng_seed * 1103515245 + 12345;
    return (rng_seed / 65536) % 32768;
}

// HAL_GetTick() 함수 (실제 STM32 HAL에서 제공)
//unsigned long HAL_GetTick(void) {
//    // 실제 하드웨어에서는 시스템 틱 카운터를 반환
//    // 여기서는 시뮬레이션용으로 간단히 구현
//    static unsigned long tick_counter = 0;
//    return tick_counter += 10; // 10ms씩 증가하도록 시뮬레이션
//}

// 하드웨어 제어 함수들
void init_hardware() {
    // GPIO 초기화
	joy_sw = 0;
	copy_memory(pattern, default_pattern, 8);
	display_heart(pattern);
    // 조이스틱 핀들을 입력으로 설정
    printf("Hardware initialized\n");
}

void set_led(int row, int col, int state) {

	pattern_modify(pattern, row, col, state);
    // 실제 하드웨어에서는 해당 LED를 켜거나 끔
    // 매트릭스 스캐닝 방식으로 구현
    static int display_buffer[BOARD_HEIGHT][BOARD_WIDTH];
    display_buffer[row][col] = state;
}

int read_joystick_x() {
    // ADC로 조이스틱 X축 값 읽기 (0-1023)
    // 실제 하드웨어에서 구현
    return joy_x;
}

int read_joystick_y() {
    // ADC로 조이스틱 Y축 값 읽기 (0-1023)
    return joy_y;
}

int read_joystick_button() {
    return joy_sw--;
}

unsigned long get_time_ms() {
    // STM32 HAL 라이브러리의 시스템 틱 사용
    return HAL_GetTick();
}

// 문자열 복사 함수
void copy_memory(void* dest, const void* src, int size) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    int i;
    for (i = 0; i < size; i++) {
        d[i] = s[i];
    }
}

// 메모리 초기화 함수
void clear_memory(void* ptr, int size) {
    char* p = (char*)ptr;
    int i;
    for (i = 0; i < size; i++) {
        p[i] = 0;
    }
}

// 게임 로직 함수들
void init_game() {
    clear_memory(&game_state, sizeof(GameState));

    // 지렁이 초기화 (중앙에서 시작)
    game_state.snake_length = 3;
    game_state.snake[0].x = BOARD_WIDTH / 2;
    game_state.snake[0].y = BOARD_HEIGHT / 2;
    game_state.snake[1].x = BOARD_WIDTH / 2 - 1;
    game_state.snake[1].y = BOARD_HEIGHT / 2;
    game_state.snake[2].x = BOARD_WIDTH / 2 - 2;
    game_state.snake[2].y = BOARD_HEIGHT / 2;

    game_state.direction = DIR_RIGHT;
    game_state.next_direction = DIR_RIGHT;
    game_state.speed = 500;
    game_state.last_move_time = get_time_ms();
    game_state.last_input_time = get_time_ms();

    // HAL_GetTick() 기반으로 시드 설정 (시간 기반 랜덤)
    set_seed(HAL_GetTick());

    // 첫 번째 먹이 생성
    generate_food();
}

void generate_food() {
    int attempts = 0;
    do {
        game_state.food.x = simple_rand() % BOARD_WIDTH;
        game_state.food.y = simple_rand() % BOARD_HEIGHT;
        attempts++;
    } while (is_snake_position(game_state.food.x, game_state.food.y) && attempts < 100);

    if (attempts >= 100) {
        game_state.game_over = 1;
    }
}

int is_snake_position(int x, int y) {
    int i;
    for (i = 0; i < game_state.snake_length; i++) {
        if (game_state.snake[i].x == x && game_state.snake[i].y == y) {
            return 1;
        }
    }
    return 0;
}

int is_valid_direction(int current, int new_dir) {
    if ((current == DIR_UP && new_dir == DIR_DOWN) ||
        (current == DIR_DOWN && new_dir == DIR_UP) ||
        (current == DIR_LEFT && new_dir == DIR_RIGHT) ||
        (current == DIR_RIGHT && new_dir == DIR_LEFT)) {
        return 0;
    }
    return 1;
}

void handle_input() {
    unsigned long current_time = get_time_ms();

    if (current_time - game_state.last_input_time < 100) {
        return;
    }

    int joy_x = read_joystick_x();
    int joy_y = read_joystick_y();
    int button = read_joystick_button();

    int new_direction = game_state.direction;

    if (joy_x < JOYSTICK_CENTER - JOYSTICK_THRESHOLD) {
        new_direction = DIR_LEFT;
    }
    else if (joy_x > JOYSTICK_CENTER + JOYSTICK_THRESHOLD) {
        new_direction = DIR_RIGHT;
    }
    else if (joy_y < JOYSTICK_CENTER - JOYSTICK_THRESHOLD) {
        new_direction = DIR_UP;
    }
    else if (joy_y > JOYSTICK_CENTER + JOYSTICK_THRESHOLD) {
        new_direction = DIR_DOWN;
    }

    if (is_valid_direction(game_state.direction, new_direction)) {
        game_state.next_direction = new_direction;
        game_state.last_input_time = current_time;
    }

    if (button) {
        game_state.speed = 150;
    } else {
        game_state.speed = 300 - (game_state.score / 5) * 20;
        if (game_state.speed < 100) game_state.speed = 100;
    }
}

void update_game() {
    unsigned long current_time;
    Point new_head;
    int ate_food;
    int i;

    if (game_state.game_over) return;

    current_time = get_time_ms();

    if (current_time - game_state.last_move_time > game_state.speed) {
        game_state.direction = game_state.next_direction;

        new_head = game_state.snake[0];

        if (game_state.direction == DIR_UP) {
            new_head.y--;
        } else if (game_state.direction == DIR_DOWN) {
            new_head.y++;
        } else if (game_state.direction == DIR_LEFT) {
            new_head.x--;
        } else if (game_state.direction == DIR_RIGHT) {
            new_head.x++;
        }

        // 벽 충돌 체크
        if (new_head.x < 0 || new_head.x >= BOARD_WIDTH ||
            new_head.y < 0 || new_head.y >= BOARD_HEIGHT) {
            game_state.game_over = 1;
            return;
        }

        // 자기 몸통 충돌 체크
        if (is_snake_position(new_head.x, new_head.y)) {
            game_state.game_over = 1;
            return;
        }

        // 먹이를 먹었는지 체크
        ate_food = (new_head.x == game_state.food.x && new_head.y == game_state.food.y);

        if (ate_food) {
            game_state.snake_length++;
            game_state.score += 10;
            generate_food();
        }

        // 몸통 이동 (뒤에서부터)
        for (i = game_state.snake_length - 1; i > 0; i--) {
            game_state.snake[i] = game_state.snake[i - 1];
        }

        game_state.snake[0] = new_head;
        game_state.last_move_time = current_time;
    }
}

void render_display() {
    static int blink_counter = 0;
    int x, y, i;

    // LED 매트릭스 클리어
    for (y = 0; y < BOARD_HEIGHT; y++) {
        for (x = 0; x < BOARD_WIDTH; x++) {
            set_led(y, x, 0);
        }
    }

    // 지렁이 표시
    for (i = 0; i < game_state.snake_length; i++) {
        x = game_state.snake[i].x;
        y = game_state.snake[i].y;
        if (x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT) {
            set_led(y, x, 1);
        }
    }

    // 먹이 표시 (깜빡이는 효과)
    blink_counter++;
    if ((blink_counter / 5) % 2 == 0) {
        set_led(game_state.food.y, game_state.food.x, 1);
    }

    // 콘솔 출력 (디버그용)
    printf("Score: %d | Length: %d\n", game_state.score, game_state.snake_length);

    for (y = 0; y < BOARD_HEIGHT; y++) {
        printf("|");
        for (x = 0; x < BOARD_WIDTH; x++) {
            if (x == game_state.food.x && y == game_state.food.y) {
                printf("@");
            } else if (is_snake_position(x, y)) {
                if (x == game_state.snake[0].x && y == game_state.snake[0].y) {
                    printf("O");
                } else {
                    printf("*");
                }
            } else {
                printf(" ");
            }
        }
        printf("|\n");
    }
    printf("+--------+\n");

    if (game_state.game_over) {
        printf("GAME OVER! Final Score: %d\n", game_state.score);
    }
}

// 간단한 딜레이 함수
void simple_delay(int count) {
    volatile int i;
    for (i = 0; i < count * 1000; i++) {
        // 빈 루프
    }
}

void game_over_display(void) {
    int row, col, i;

    // 1단계: 행 7부터 행 0까지 차례대로 켜기
    for (row = BOARD_HEIGHT - 1; row >= 0; row--) {
        for (col = 0; col < BOARD_WIDTH; col++) {
            set_led(row, col, PIXEL_ON);
        }
        simple_delay(200);
    }

    // 2단계: 전체 깜빡임 (3번)
    for (i = 0; i < 3; i++) {
        for (row = 0; row < BOARD_HEIGHT; row++) {
            for (col = 0; col < BOARD_WIDTH; col++) {
                set_led(row, col, PIXEL_OFF);
            }
        }
        simple_delay(150);

        for (row = 0; row < BOARD_HEIGHT; row++) {
            for (col = 0; col < BOARD_WIDTH; col++) {
                set_led(row, col, PIXEL_ON);
            }
        }
        simple_delay(150);
    }

    // 3단계: 행 0부터 행 7까지 차례대로 끄기
    for (row = 0; row < BOARD_HEIGHT; row++) {
        for (col = 0; col < BOARD_WIDTH; col++) {
            set_led(row, col, PIXEL_OFF);
        }
        simple_delay(150);
    }

    simple_delay(300);
}

// 시뮬레이션용 입력 (테스트용)
void simulate_input() {
    static int counter = 0;
    Point head;

    counter++;
    if (counter % 50 == 0) {
        head = game_state.snake[0];

        // 간단한 AI: 먹이 방향으로 이동
        if (game_state.food.x > head.x && is_valid_direction(game_state.direction, DIR_RIGHT)) {
            game_state.next_direction = DIR_RIGHT;
        } else if (game_state.food.x < head.x && is_valid_direction(game_state.direction, DIR_LEFT)) {
            game_state.next_direction = DIR_LEFT;
        } else if (game_state.food.y > head.y && is_valid_direction(game_state.direction, DIR_DOWN)) {
            game_state.next_direction = DIR_DOWN;
        } else if (game_state.food.y < head.y && is_valid_direction(game_state.direction, DIR_UP)) {
            game_state.next_direction = DIR_UP;
        }
    }
}
