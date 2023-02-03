
struct LcdDisplay {
    int rs_pin;
    int e_pin;
    int db4_pin;
    int db5_pin;
    int db6_pin;
    int db7_pin;
};

#define MAX_LINE_LENGTH 40

struct LcdDisplay setup_display(int rs, int e, int db4, int db5, int db6, int db7);

void write_to_display(char *line1, struct LcdDisplay display);

void write_to_display_2_lines(char *line1, char *line2, struct LcdDisplay d);