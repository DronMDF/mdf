//
// Copyright (c) 2000-2006 Андрей Валяев (dron@infosec.ru)
// All rights reserved.
//
// Created: 12/12/06 22:45:26
//

#pragma once

#define NCC 8

struct termio {
        unsigned short c_iflag;         /* input mode flags */
        unsigned short c_oflag;         /* output mode flags */
        unsigned short c_cflag;         /* control mode flags */
        unsigned short c_lflag;         /* local mode flags */
        unsigned char c_line;           /* line discipline */
        unsigned char c_cc[NCC];        /* control characters */
};

#define PARENB   0000400
#define   CS8    0000060

#define ECHO     0000010
#define ECHOK    0000040
#define ECHONL   0000100

#define VTIME 5
#define VMIN 6

#define ISTRIP   0000040
#define INLCR    0000100
#define ICRNL    0000400
#define ISIG     0000001
#define ICANON   0000002

#define TCGETA             0x5405
#define TCSETAW            0x5407
