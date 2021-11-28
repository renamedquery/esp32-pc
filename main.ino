// code is for an arduino at the moment since the esp32s have been delayed in shipping

#include <Arduino.h>
#include <stdlib.h>

#define VGA_WIDTH 40
#define VGA_HEIGHT 40
#define DEJITTER_OFFSET 1
#define DEJITTER_SYNC -2
#define CLONED_LINES  6-1
#define SKIPLINES 90

struct _PINS {

    const byte R = 6;
    const byte G = 7;
    const byte B = 5;

    const byte VSYNC = 9;
    const byte HSYNC = 3;
};

struct _VGA {

    const uint16_t WIDTH = VGA_WIDTH;
    const uint16_t HEIGHT = VGA_HEIGHT;

    const byte CLEAR_COLOR = 33;
    const byte TEXT_COLOR = 00;
};

const _PINS PINS;
const _VGA VGA;

byte vga_framebuffer[VGA_WIDTH * VGA_HEIGHT];
static byte aline, rlinecnt, vskip, afreq, afreq0;
unsigned long vtimer;

ISR(TIMER1_OVF_vect) {

    aline -= 1;
    vskip = SKIPLINES;
    vtimer++;
    rlinecnt = 0;
}

ISR(TIMER2_OVF_VECT) {

    // this is taken from VGAX - I have little to no idea what it does and it was not written by me
    // full credit to VGAX
    asm volatile (                                   //4c to load Z and Y
    "      ld r16, Z                        \n\t" //c1 r16=afreq
    "      cpi %[freq0], 0                  \n\t" //c1 afreq0==0 ?
    "      breq no_audio                    \n\t" //c1/2 *0
    "play_audio:                            \n\t" 
    "      cpi r16, 0                       \n\t" //c1 afreq==0 ?
    "      brne dont_flip_audio_pin         \n\t" //c1/2 *1
    "flip_audio_pin:                        \n\t" 
    "      ldi r18, 1                       \n\t" //c1
    "      out %[audiopin], r18             \n\t" //c1
    "      st Z, %[freq0]                   \n\t" //c1 afreq=afreq0
    "      rjmp end                         \n\t" //c2
    //"    mov r16, %[freq0]\n\r"
    //"    dec r16\n\r"
    "no_audio:                              \n\t" 
    "      nop                              \n\t" //c1
    "      nop                              \n\t" //c1
    "      nop                              \n\t" //c1
    //"    nop                              \n\t" //c1
    "      nop                              \n\t" //c1
    "      nop                              \n\t" //c1
    "      nop                              \n\t" //c1
    "      rjmp end                         \n\t" //c2
    "dont_flip_audio_pin:                   \n\t" 
    "      dec r16                          \n\t" //c1
    "      st Z, r16                        \n\t" //c1
    //"    nop                              \n\t" //c1
    "end:                                   \n\t"
    :
    : "z" (&afreq),
        [freq0] "r" (afreq0),
        [audiopin] "i" _SFR_IO_ADDR(PINC)
    : "r16", "r18");

    // check for vertical sync porch boundary
    if (vskip) {
        
        vskip--;
        return;
    }

    // interrupt jitter fix
    // code from https://github.com/cnlohr/avrcraft/tree/master/terminal and VGAX
    if (rlinecnt < VGA.HEIGHT) {

        asm volatile (
        "     lds r16, %[timer0]    \n\t" //
        "     subi r16, %[tsync]    \n\t" //
        "     andi r16, 7           \n\t" //
        "     call TL               \n\t" //
        "TL:                        \n\t" //
        "     pop r31               \n\t" //
        "     pop r30               \n\t" //
        "     adiw r30, (LW-TL-5)   \n\t" //
        "     add r30, r16          \n\t" //
        //"   adc r31, __zero_reg__ \n\t" //
        "     ijmp                  \n\t" //
        "LW:                        \n\t" //
        "     nop                   \n\t" //
        "     nop                   \n\t" //
        "     nop                   \n\t" //
        "     nop                   \n\t" //
        "     nop                   \n\t" //
        "     nop                   \n\t" //
        "     nop                   \n\t" //
        //"   nop                   \n\t" //
        "LBEND:                     \n\t" //
        :
        : [timer0] "i" (&TCNT0),
        [toffset] "i" ((uint8_t)DEJITTER_OFFSET),
        [tsync] "i" ((uint8_t)DEJITTER_SYNC)
        : "r30", "r31", "r16", "r17");

        // output all pixels
        // check VGAX.cpp on their github repo for more information
        asm volatile (
        "    ldi r20, 4       \n\t" //const for <<2bit
        ".rept 30             \n\t" //output 4 pixels for each iteration
        "    ld r16, Z+       \n\t" //
        "    out %[port], r16 \n\t" //write pixel 1
        "    mul r16, r20     \n\t" //<<2
        "    out %[port], r0  \n\t" //write pixel 2
        "    mul r0, r20      \n\t" //<<4
        "    out %[port], r0  \n\t" //write pixel 3
        "    mul r0, r20      \n\t" //<<6
        "    out %[port], r0  \n\t" //write pixel 4
        ".endr                \n\t" //
        "    nop              \n\t" //expand last pixel
        "    ldi r16, 0       \n\t" //
        "    out %[port], r16 \n\t" //write black for next pixels
        :
        : [port] "I" (_SFR_IO_ADDR(PORTD)),
        "z" "I" (/*rline*/(byte*)vga_framebuffer + rlinecnt * VGA.WIDTH)
        : "r16", "r17", "r20", "r21", "memory");

        if (++aline == CLONED_LINES) {

            aline -= 1;
            rlinecnt++;

        } else {

            asm volatile(
            ".rept 17 \n\t" //
            "    nop  \n\t" //
            ".endr    \n\t" //
            :::);
        }
    }
}

// from VGAX
void vga_clear(byte color) {
    
    register byte c = color;
    c&=3;

    register byte c0 = (c * 4) | c;
    c0 |= c0 * 16;

    memset(vga_framebuffer, c0, VGA_WIDTH * VGA_HEIGHT);
}

// from VGAX
void vga_copy(byte *src) {

    byte *o = (byte*)vga_framebuffer;
    unsigned cnt = VGA.WIDTH * VGA.HEIGHT;
    while (cnt--) *o++ = pgm_read_byte(src++);
}

// from VGAX
void vga_delay(int milliseconds) {

    while (milliseconds--) {

        unsigned cnt = 16000 / 30;
        while (cnt--) asm volatile("nop\nnop\nnop\nnop\n");
    }
}

// from VGAX
// values can be 0-3
void vga_draw_pixel(byte x, byte y, byte color) {

    byte *p = vga_framebuffer + y * VGA.WIDTH + (x >> 2);
    byte bitpos = 6 - (x & 3) * 2;
    *p = (*p & ~(3 << bitpos)) | color << bitpos;
}

void setup() {

    cli();

    // from VGAX
    pinMode(PINS.VSYNC, OUTPUT);
    TIMSK0 = 0;
    TCCR0A = 0;
    TCCR0B = (1 << CS00); // enable 16MHz counter (used to fix the HSYNC interrupt jitter)
    OCR0A = 0;
    OCR0B = 0;
    TCNT0 = 0;

    // from VGAX
    TCCR1A = bit(WGM11) | bit(COM1A1);
    TCCR1B = bit(WGM12) | bit(WGM13) | bit(CS12) | bit(CS10); //1024 prescaler
    ICR1 = 259; //16666 / 64 uS=260 (less one)
    OCR1A = 0; //64 / 64 uS=1 (less one)
    TIFR1 = bit(TOV1); //clear overflow flag
    TIMSK1 = bit(TOIE1); //interrupt on overflow on TIMER1
    pinMode(PINS.HSYNC, OUTPUT);
    TCCR2A = bit(WGM20) | bit(WGM21) | bit(COM2B1); //pin3=COM2B1
    TCCR2B = bit(WGM22) | bit(CS21); //8 prescaler
    OCR2A = 63; //32 / 0.5 uS=64 (less one)
    OCR2B = 7; //4 / 0.5 uS=8 (less one)
    TIFR2 = bit(TOV2); //clear overflow flag
    TIMSK2 = bit(TOIE2); //interrupt on overflow on TIMER2

    pinMode(PINS.R, OUTPUT);
    pinMode(PINS.G, OUTPUT);
    pinMode(PINS.B, OUTPUT);

    sei();

    vga_clear(VGA.CLEAR_COLOR);
}

void loop() {

    vga_clear(VGA.CLEAR_COLOR);

    vga_delay(17);
}