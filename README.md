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

## PIO

In this implementation the PWM counter is replaced with a PIO state machine which is doing the same job of counting the number of pulses seen on the measured clock pin.  The pulse count is streamed into a variable accessible in the CPU using DMA.  An interrupt is still used for the 1PPS signal to take a snapshot of the pulse counter.
