# rp2040-freq-counter
These are different implementations of a frequency counter for the Raspberry Pi RP2040 (Pico) microprocessor.

They each follow the same basic design:

* a pulse counter
* a gate which monitors a 1PPS (1 pulse per second) input from a GPS

Each subfolder contains a different implementation.

## PWM

This is in the `pwm` folder.  In this implementation, the hardware PWM block is used to count the pulses coming from the measured clock and a CPU interrupt attached to a GPIO pin is used to monitor the 1PPS signal.  

On each 1PPS a snapshot is taken of the current pulse count and it is compared to the previous snapshot to determine the number of pulses in the last second.

Because the PWM pulse counter is a uint8_t (a short integer), another interrupt is used to monitor the PWM counter overflow in order to determine the true count of pulses.

It writes one line per second (paced based on the 1PPS) to the serial port, and each line has the following fields, separated by spaces:

```
iterationcount milliseconds numberofpwmwraps rawpwmcounter calculatedcount deltasincelastpulse
```

The `deltasincelastpulse` is the measured frequency.

## PIO

In this implementation the PWM counter is replaced with a PIO state machine which is doing the same job of counting the number of pulses seen on the measured clock pin.  The pulse count is streamed into a variable accessible in the CPU using DMA.  An interrupt is still used for the 1PPS signal to take a snapshot of the pulse counter.

Because DMA streams are finite, two are used and they hand-off to each other as they finish, and this allows the stream to keep running perpetually.

Similarly to the PWM version it writes one line per second to the serial port.  The lines are similar in structure, but some fields are different:

```
iterationcount milliseconds rawpulsecount deltasincelastpulse
```

The `rawpulsecount` can go up as well as down because of the way the counter works, but the `deltasincelastpulse` (measured frequency) should always be 0 or positive.

## PIO-PPS

This extends the PIO implementation and uses PIO for the 1PPS signal as well.  In this version the pulse count from the first state machine is sent using DMA into a second state machine which is listening for the 1PPS rising edge.  When it sees it, the second state machine pushes the current pulse count to the CPU and raises an interrupt.

This version is the final one I made.  It was done to see if we could get better accuracy by responding to the 1PPS quicker than via an interrupt.

It also outputs one line per second to the serial port and can drive a HD44780 LCD display.  The lines sent to the serial port are formatted as follows:

```
iterationcount milliseconds rawpulsecount deltasincelastpulse avgdelta
```

`avgdelta` is the average of the last 10 cycles of data, it is a double precision number.
