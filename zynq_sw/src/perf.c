/*
Copyright (c) 2020 Ryan Blais, Hugo Burd, Byron Kontou, and Jeff Stacey

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "perf.h"

void init_ccount() {
    uint32_t cfg = 1; // enable all counters

    cfg |= 1 << 1; //reset all counters
    cfg |= 1 << 2; //reset cycle counters

    cfg |= 1 << 3; //enables incrementing the counter every 64 cycles

    // set the config register
    asm volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(cfg));

    // enable the counters
    asm volatile ("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x80000007));

    // clear overflow flags
    asm volatile ("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x80000007));
}

uint32_t get_ccount() {
    uint32_t value;
    asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n" : "=r"(value));
    return value;
}
