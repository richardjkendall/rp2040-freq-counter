#include "main.h"
#include "counter.pio.h"

#define PPS_PIN 2
#define SIGNAL_PIN 15

// these are used by the PPS interrupt routine
volatile int ppsFlag = 0;
volatile absolute_time_t ppsTime;
volatile int32_t pulseCountSnapshot = 0;

// this is the target for the DMA channels from PIO
int32_t pulseCount = 0;

// declare the IDs for the DMA channels
int dma_chan = 0;
int dma_chan2 = 0;

void pps_callback(uint gpio, uint32_t events) {
    pulseCountSnapshot = pulseCount;
    ppsTime = get_absolute_time();
    ppsFlag = 1;
}

int main() {
    stdio_init_all();

    printf("Starting...\n");

    // setup PIO
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &counter_program);
    uint sm = pio_claim_unused_sm(pio, true);
    counter_program_init(pio, sm, offset, SIGNAL_PIN);

    // setup DMA
    dma_chan = dma_claim_unused_channel(true);
    dma_chan2 = dma_claim_unused_channel(true);

    // channel 1, this starts and than hands over to the second channel when it is done
    // channel 2 then hands back to channel 1, so we get a continous DMA stream to a single target variable
    dma_channel_config dc = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&dc, DMA_SIZE_32);
    channel_config_set_read_increment(&dc, false);
    channel_config_set_write_increment(&dc, false);
    channel_config_set_chain_to(&dc, dma_chan2);
    channel_config_set_dreq(&dc, pio_get_dreq(pio, sm, false));
    dma_channel_configure(dma_chan, &dc,
        &pulseCount,
        &pio->rxf[sm],
        0xFFFFFFFF,
        true
    );

    // channel 2 as above
    dma_channel_config dc2 = dma_channel_get_default_config(dma_chan2);
    channel_config_set_transfer_data_size(&dc2, DMA_SIZE_32);
    channel_config_set_read_increment(&dc2, false);
    channel_config_set_write_increment(&dc2, false);
    channel_config_set_dreq(&dc2, pio_get_dreq(pio, sm, false));
    channel_config_set_chain_to(&dc2, dma_chan);
    dma_channel_configure(dma_chan2, &dc2,
        &pulseCount,
        &pio->rxf[sm],
        0xFFFFFFFF,
        false
    );

    // setup PPS
    // the PPS signal is the trigger to get a snapshot of the time / pulse counters
    gpio_init(PPS_PIN);
    gpio_set_dir(PPS_PIN, GPIO_IN);
    gpio_pull_down(PPS_PIN);
    gpio_set_irq_enabled_with_callback(PPS_PIN, 0x08, 1, pps_callback);

    uint32_t seconds = 0;
    uint32_t prevCount = 0;

    while(1) {
        if(ppsFlag == 1) {
            uint32_t delta = prevCount - pulseCountSnapshot;
            uint32_t millis = to_ms_since_boot(ppsTime);
            printf("%d %d %d %d\n", seconds, millis, pulseCountSnapshot, delta);
            seconds++;
            prevCount = pulseCountSnapshot;
            ppsFlag = 0;
        }
        tight_loop_contents();
    }

}