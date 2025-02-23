#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "ssd1306.h"
#include "hardware/i2c.h"
#include "play_audio.h"

const uint I2C_SDA_PIN = 14;
const uint I2C_SCL_PIN = 15;
const int WORK_DURATION = 25 * 60; // 25 minutos em segundos
const int BREAK_DURATION = 5 * 60; // 5 minutos em segundos

void update_display(int minutes, int seconds, bool is_working) {
    char buffer[32];
    if (is_working) {
        sprintf(buffer, "Trabalho: %02d:%02d", minutes, seconds);
    } else {
        sprintf(buffer, "Descanso: %02d:%02d", minutes, seconds);
    }

    uint8_t buf[SSD1306_BUF_LEN];
    memset(buf, 0, SSD1306_BUF_LEN);
    struct render_area frame_area = {
        .start_col = 0,
        .end_col = SSD1306_WIDTH - 1,
        .start_page = 0,
        .end_page = SSD1306_NUM_PAGES - 1
    };
    WriteString(buf, 0, 0, buffer);
    render(buf, &frame_area);
}

void pomodoro_timer(int duration_seconds, bool is_working) {
    int minutes = duration_seconds / 60;
    int seconds = duration_seconds % 60;

    for (int i = duration_seconds; i >= 0; i--) {
        if (is_working) {
            printf("Trabalho - Tempo restante: %02d:%02d\n", minutes, seconds);
        } else {
            printf("Descanso - Tempo restante: %02d:%02d\n", minutes, seconds);
        }

        update_display(minutes, seconds, is_working);
        sleep_ms(1000); // Espera por 1 segundo

        seconds--;
        if (seconds < 0) {
            seconds = 59;
            minutes--;
        }
    }
    play_audio_buzzer();
}

int main() {
    stdio_init_all();
    setup_audio();

    i2c_init(i2c1, SSD1306_I2C_CLK * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    SSD1306_init();

    struct render_area frame_area = {
        .start_col = 0,
        .end_col = SSD1306_WIDTH - 1,
        .start_page = 0,
        .end_page = SSD1306_NUM_PAGES - 1
    };

    calc_render_area_buflen(&frame_area);

    uint8_t buf[SSD1306_BUF_LEN];
    memset(buf, 0, SSD1306_BUF_LEN);
    render(buf, &frame_area);

restart:
    SSD1306_scroll(true);
    sleep_ms(5000);
    SSD1306_scroll(false);

    while (true) {
        // Exibir mensagens de trabalho
        char *text_work[] = {
            "Estudar",
            "Tempo de 25   ",
            "Minutos        "
        };

        int y = 0;
        for (uint i = 0; i < count_of(text_work); i++) {
            WriteString(buf, 5, y, text_work[i]);
            y += 8;
        }
        render(buf, &frame_area);

        SSD1306_init();

        pomodoro_timer(WORK_DURATION, true); // 25 minutos de trabalho

        // Exibir mensagens de descanso
        char *text_break[] = {
            "Descansar 5 M",
            "Beba agua      ",
            "Se alongue"
        };

        y = 0;
        for (uint i = 0; i < count_of(text_break); i++) {
            WriteString(buf, 5, y, text_break[i]);
            y += 8;
        }
        render(buf, &frame_area);

        SSD1306_init();

        pomodoro_timer(BREAK_DURATION, false); // 5 minutos de descanso
    }
    return 0;
}