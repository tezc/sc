/*
 * MIT License
 *
 * Copyright (c) 2020 Ozan Tezcan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "sc_rc4.h"

#include <memory.h>

void sc_rc4_init(struct sc_rc4 *rc4, const unsigned char rand[256])
{
    unsigned char t;

    rc4->j = 0;
    rc4->i = 0;

    memcpy(rc4->s, rand, 256);

    for (int i = 0; i < 256; i++) {
        rc4->j += rc4->s[i] + rand[i];
        t = rc4->s[rc4->j];
        rc4->s[rc4->j] = rc4->s[i];
        rc4->s[i] = t;
    }
}


void sc_rc4_random(struct sc_rc4 *rc4, void *buf, int size)
{
    unsigned char t;
    unsigned char *p = buf;

    if (size <= 0 || buf == NULL) {
        return;
    }

    do {
        rc4->i++;
        t = rc4->s[rc4->i];
        rc4->j += t;
        rc4->s[rc4->i] = rc4->s[rc4->j];
        rc4->s[rc4->j] = t;
        t += rc4->s[rc4->i];
        *(p++) = rc4->s[t];
    } while (--size);
}
