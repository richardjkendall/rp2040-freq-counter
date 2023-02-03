#include "main.h"
#include "counter.pio.h"
#include "pps.pio.h"
#include "buf.h"
#include "lcd.h"

#define AVG_BUFFER_SIZE 10

#define    PPS_PIN 2
#define SIGNAL_PIN 15

#define  LCD_RS_PIN 10
#define   LCD_E_PIN 11
#define LCD_DB4_PIN 18
#define LCD_DB5_PIN 19
#define LCD_DB6_PIN 20
#define LCD_DB7_PIN 21

// these are used by the PPS interrupt routine
volatile int ppsFlag = 0;
volatile absolute_time_t ppsTime;
volatile int32_t pulseCountSnapshot = 0;

// globals
// pulseCount is the target for the DMA stream from the PPS SM
uint32_t pulseCount = 0;

// declare the IDs for the DMA channels
int dma_chan = 0;
int dma_chan2 = 0;
int dma_chan3 = 0;

// PIO globals
PIO pio = pio0;
uint sm_pps = 0;

void pps_callback() {
    pulseCountSnapshot = pulseCount;
    ppsTime = get_absolute_time();
    ppsFlag = 1;
    pio_interrupt_clear(pio, 0);
}

void update_display(struct LcdDisplay display, uint32_t freq, float avg) {
    char *line1 = malloc(50 * sizeof(char));
    sprintf(line1, "F: %dHz", freq);
    char *line2 = malloc(50 * sizeof(char));
    sprintf(line2, "A: %.1fHz", avg);

    write_to_display_2_lines(line1, line2, display);

    free(line1);
    free(line2);
}

int main() {
    stdio_init_all();

    // setup display
    struct LcdDisplay display = setup_display(LCD_RS_PIN, LCD_E_PIN, LCD_DB4_PIN, LCD_DB5_PIN, LCD_DB6_PIN, LCD_DB7_PIN);

    // load programs
    uint offset = pio_add_program(pio, &counter_program);
    uint offset_pps = pio_add_program(pio, &pps_program);

    // PPS program
    sm_pps = 0;
    pps_program_init(pio, sm_pps, offset_pps, PPS_PIN);

    // pulse counter program
    uint sm = 1;
    counter_program_init(pio, sm, offset, SIGNAL_PIN);

    // setup DMA between SMs
    dma_chan = dma_claim_unused_channel(true);
    dma_chan2 = dma_claim_unused_channel(true);

    // channel 1, this starts and than hands over to the second channel when it is done
    // channel 2 then hands back to channel 1, so we get a continous DMA stream to a single target variable
    dma_channel_config dc = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&dc, DMA_SIZE_32);
    channel_config_set_read_increment(&dc, false);
    channel_config_set_write_increment(&dc, false);
    channel_config_set_chain_to(&dc, dma_chan2);
    channel_config_set_dreq(&dc, pio_get_dreq(pio, sm_pps, true));
    dma_channel_configure(dma_chan, &dc,
        &pio->txf[sm_pps],
        &pio->rxf[sm],
        0xFFFFFFFF,
        true
    );

    // channel 2 as above
    dma_channel_config dc2 = dma_channel_get_default_config(dma_chan2);
    channel_config_set_transfer_data_size(&dc2, DMA_SIZE_32);
    channel_config_set_read_increment(&dc2, false);
    channel_config_set_write_increment(&dc2, false);
    channel_config_set_dreq(&dc2, pio_get_dreq(pio, sm_pps, true));
    channel_config_set_chain_to(&dc2, dma_chan);
    dma_channel_configure(dma_chan2, &dc2,
        &pio->txf[sm_pps],
        &pio->rxf[sm],
        0xFFFFFFFF,
        false
    );

    // setup DMA from PPS SM to CPU
    dma_chan3 = dma_claim_unused_channel(true);

    dma_channel_config dc3 = dma_channel_get_default_config(dma_chan3);
    channel_config_set_transfer_data_size(&dc3, DMA_SIZE_32);
    channel_config_set_read_increment(&dc3, false);
    channel_config_set_write_increment(&dc3, false);
    //channel_config_set_chain_to(&dc, dma_chan3);
    channel_config_set_dreq(&dc3, pio_get_dreq(pio, sm_pps, false));
    dma_channel_configure(dma_chan3, &dc3,
        &pulseCount,
        &pio->rxf[sm_pps],
        0xFFFFFFFF,
        true
    );

    // setup PPS IRQ/ISR
    irq_set_exclusive_handler(PIO0_IRQ_0, pps_callback);
    irq_set_enabled(PIO0_IRQ_0, true);

    // enable state machines
    pio_sm_set_enabled(pio, sm_pps, true);
    pio_sm_set_enabled(pio, sm, true);

    uint32_t seconds = 0;
    uint32_t prevCount = 0;

    // need to create buffer to store data for averaging
    uint32_t *buf = malloc(AVG_BUFFER_SIZE * sizeof(uint32_t));
    struct cir_buf* avg_buf = cir_buf_init(buf, AVG_BUFFER_SIZE);

    while(1) {
        if(ppsFlag == 1) {
            uint32_t delta = prevCount - pulseCountSnapshot;
            uint32_t millis = to_ms_since_boot(ppsTime);
            cir_buf_push(avg_buf, delta);
            float avg = cir_buf_avg(avg_buf);
            printf("%d %d %d %d %f\n", seconds, millis, pulseCountSnapshot, delta, avg);
            update_display(display, delta, avg);
            seconds++;
            prevCount = pulseCountSnapshot;
            ppsFlag = 0;
        }
        tight_loop_contents();
    }

}