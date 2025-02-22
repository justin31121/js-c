#ifndef TYPES_H
#define TYPES_H

// MIT License
// 
// Copyright (c) 2024 Justin Schartner
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdio.h>
#include <stdlib.h>

#define da_append(xs, x) do{\
    if((xs)->cap <= (xs)->len) {\
        (xs)->cap = ((xs)->cap == 0) ? 256 : ((xs)->cap << 1);\
        (xs)->data = realloc((xs)->data, sizeof(*((xs)->data)) * (xs)->cap);\
        if(!((xs)->data)) panic("realloc failed");\
    }\
    (xs)->data[(xs)->len++] = (x);\
  }while(0)

#define here_fmt "%s:%d:"
#define here_arg() __FILE__, __LINE__

#define panic(...) do{					\
    fprintf(stderr, here_fmt"ERROR: ", here_arg());	\
    fflush(stderr);					\
    fprintf(stderr, __VA_ARGS__); fflush(stderr);	\
    exit(1);						\
  }while(0)

#define UNREACHABLE() panic("UNREACHABLE")
#define TODO() panic("TODO")

#define cast(t, o) (*(t *) &(o))
#define return_defer(n) do{ result = (n); goto defer; }while(0)

typedef                    char   s8;
typedef unsigned           char   u8;
typedef                    short s16;
typedef unsigned           short u16;
typedef                    int   s32;
typedef unsigned           int   u32;
typedef          long long int   s64;
typedef unsigned long long int   u64;

typedef float  f32;
typedef double f64;

#endif // TYPES_H
