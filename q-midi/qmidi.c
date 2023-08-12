


#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "lib/qsim/qsim.h"
#include "lib/qsim/measure.h"


#define UART_ID uart0
#define BAUD_RATE 31250
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// button
#define BUTTON1 5 // GPIO 5
bool button_push = 0;

// note on/off arrays for channel 1!
char noteon[] = {0x91,60,40};
char noteoff[] = {0x81,60,0};
char prev_bytes[3];
char note_bytes[3];
char added_note = 50;
int ctr = 0;
uint8_t quantum_adjustment = 0;

//static int chars_rxed = 0;
const uint LED_PIN = PICO_DEFAULT_LED_PIN;

// RX interrupt handler
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        if (uart_is_writable(UART_ID)) {
            if((ch & 0xF0) == 0x90) { // check first four bits of the first byte...
                // if it is a note-on message, do the following:
                note_bytes[0] = ch; // get the note
                note_bytes[1] = uart_getc(UART_ID);
                note_bytes[2] = uart_getc(UART_ID);
                for (int i = 0; i<3; i++){ // send the note
                    uart_putc(UART_ID, note_bytes[i]);
                }
                uart_putc(UART_ID, 0x81); // change the added note...
                uart_putc(UART_ID, added_note);
                uart_putc(UART_ID, 0);
                added_note = note_bytes[1]+4+quantum_adjustment; 
                uart_putc(UART_ID, 0x91);
                uart_putc(UART_ID, added_note);
                uart_putc(UART_ID, note_bytes[2]); // match the velocity
                ctr = 0; // reset the counter
            }
            else{
                uart_putc(UART_ID, ch); // just send the character
            }
        }
        prev_bytes[2] = prev_bytes[1];
        prev_bytes[1] = prev_bytes[0];
        prev_bytes[0] = ch;
    }

}

void send_bytes(char buff[], uint8_t len){
    for(int i = 0; i < len; i++){
        uart_putc(UART_ID, buff[i]);
    }
    for(int i = 0; i < len; i++){
        //i == 0 ? uart_putc(UART_ID, buff[i]+1) : uart_putc(UART_ID, buff[i]);
        i == 1 ? uart_putc(UART_ID, buff[i]+7) : uart_putc(UART_ID, buff[i]);
    }
}

void uart_buff2(char buff[], int len){
    for(int i = 0; i < len; i++){
        uart_putc(UART_ID, buff[i]);
    }
    return;
}

void qmidi_instrument(void){
    char ctrl_buff[3] = {0xB0, 0x7F, 0x0}; // set everything to POLYphonic mode
    uart_buff2(ctrl_buff, 3);
    char cb2[3] = {0xB0, 80, 1};
    uart_buff2(cb2, 3);
    // reface DX settings
    uint8_t results[25];
    uint8_t resb[25*4];
    q_instrument(255, 25, results, resb);
    int ctrl_codes[25] = {80, 85, 86, 87, 88, 89, 90, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119};
    int mod_values[25] = {127,127,127,127,127,31, 99, 127, 127, 127, 127, 31,  99,  127, 127, 127, 127, 31,  99,  127, 127, 127, 127, 31,  99};
    for(int i = 0; i < 25; i++){
        // ignore OSC1 settings for now...
        if (i >= 1 && i <= 6){
            continue;
        }
        int mod_val = mod_values[i] + 1;
        ctrl_buff[1] = ctrl_codes[i];
        ctrl_buff[2] = results[i] % mod_val;
        uart_buff2(ctrl_buff, 3);
    }
    return;
}

// button IRQ handler
void gpio_callback(){
    // we don't really care about any data... just do stuff...
    button_push = 1;
    return;
}

int main() {
    // LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    //button

    gpio_init(BUTTON1);
    gpio_set_dir(BUTTON1, GPIO_IN);
    gpio_pull_up(BUTTON1);
    //gpio_set_irq_enabled_with_callback(BUTTON1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BUTTON1, 0x1 /*level LOW*/, true, &gpio_callback);
    
    // Set up our UART with a basic baud rate.
    uart_init(UART_ID, BAUD_RATE);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

    // OK, all set up.
    // Lets send a basic string out, and then run a loop and wait for RX interrupts
    // The handler will count them, but also reflect the incoming data back with a slight change!
    //uart_puts(UART_ID, "\nHello, uart interrupts\n");

    uart_putc(UART_ID, 0xFF); // init MIDI host


    
    while (1){
        if (button_push){
            // handle a button push
            button_push = 0;
            gpio_put(LED_PIN, 1);
            qmidi_instrument();
            sleep_ms(125);
            gpio_put(LED_PIN, 0);
        }

        gpio_put(LED_PIN, 1);

        quantum_adjustment = qmidi(note_bytes[1]/63) % 8;
        gpio_put(LED_PIN, 0);
        sleep_ms(100);
        if (ctr == 30){ // kill the added note once a every 2 secs 
            ctr = 0;
            noteoff[1] = added_note;
            send_bytes(noteoff, 3);
        }
        ctr++;
    }
}


