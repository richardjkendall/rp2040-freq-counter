#include "lcd.h"
#include "main.h"

void setup_out_pin(int pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 0);
}

void pulse_e(uint e) {
    gpio_put(e, 1);
    sleep_us(80);
    gpio_put(e, 0);
    sleep_us(80);
}

void push_nibble_to_display(uint8_t word, uint db4, uint db5, uint db6, uint db7, uint e) {
    gpio_put(db4, (word & 0b00000001));
    gpio_put(db5, ((word & 0b00000010) >> 1));
    gpio_put(db6, ((word & 0b00000100) >> 2));
    gpio_put(db7, ((word & 0b00001000) >> 3));
    pulse_e(e);
}

void push_word_to_display(uint8_t word, uint db4, uint db5, uint db6, uint db7, uint e) {
    push_nibble_to_display((word & 0b11110000) >> 4, db4, db5, db6, db7, e);
    push_nibble_to_display((word & 0b00001111), db4, db5, db6, db7, e);
}

struct LcdDisplay setup_display(int rs, int e, int db4, int db5, int db6, int db7) {
    // setup pins
    setup_out_pin(rs);
    setup_out_pin(e);
    setup_out_pin(db4);
    setup_out_pin(db5);
    setup_out_pin(db6);
    setup_out_pin(db7);
    // register select to IR
    gpio_put(rs, 0);                                                
    push_nibble_to_display(0b0011, db4, db5, db6, db7, e);          // initial function set
    push_nibble_to_display(0b0011, db4, db5, db6, db7, e);
    push_nibble_to_display(0b0011, db4, db5, db6, db7, e);
    push_nibble_to_display(0b0010, db4, db5, db6, db7, e);
    push_word_to_display(0b00101000, db4, db5, db6, db7, e);        // two lines, 4-bit, 5x8 font
    push_word_to_display(0b00001100, db4, db5, db6, db7, e);
    push_word_to_display(0b00000110, db4, db5, db6, db7, e);
    push_word_to_display(0b00000001, db4, db5, db6, db7, e);
    sleep_ms(2);
    gpio_put(rs, 1); 

    struct LcdDisplay d;
    d.rs_pin = rs;
    d.e_pin = e;
    d.db4_pin = db4;
    d.db5_pin = db5;
    d.db6_pin = db6;
    d.db7_pin = db7;
    return d;
}

void clear_display(struct LcdDisplay d) {
    gpio_put(d.rs_pin, 0);
    push_word_to_display(0b00000001, d.db4_pin, d.db5_pin, d.db6_pin, d.db7_pin, d.e_pin);
    sleep_ms(2);
    gpio_put(d.rs_pin, 1);
}

void home_cursor(struct LcdDisplay d) {
    gpio_put(d.rs_pin, 0);
    push_word_to_display(0b00000010, d.db4_pin, d.db5_pin, d.db6_pin, d.db7_pin, d.e_pin);
    gpio_put(d.rs_pin, 1);
}

void goto_line2(int line1len, struct LcdDisplay d) {
    gpio_put(d.rs_pin, 0);
    for(int i = line1len; i < MAX_LINE_LENGTH; i++) {
        push_word_to_display(0b00010100, d.db4_pin, d.db5_pin, d.db6_pin, d.db7_pin, d.e_pin);
    }
    gpio_put(d.rs_pin, 1);
}

void write_to_display(char *line1, struct LcdDisplay d) {
    clear_display(d);
    for(int i = 0; i < strlen(line1); i++) {
        push_word_to_display(line1[i], d.db4_pin, d.db5_pin, d.db6_pin, d.db7_pin, d.e_pin);
    }
}

void write_to_display_2_lines(char *line1, char *line2, struct LcdDisplay d) {
    write_to_display(line1, d);
    goto_line2(strlen(line1), d);
    for(int i = 0; i < strlen(line2); i++) {
        push_word_to_display(line2[i], d.db4_pin, d.db5_pin, d.db6_pin, d.db7_pin, d.e_pin);
    }
}