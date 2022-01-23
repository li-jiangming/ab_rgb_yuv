/*
 * conv_rgb_yuv.c
 *  Created on: 2022年1月23日
 *      Author: ljm
 */

#include "conv_rgb_yuv.h"

#include <stdlib.h>

/*
 * YUV --> RGB
 *      R = (298Y + 411 * V - 57344)>>8
 *      G = (298Y - 101* U- 211* V+ 34739)>>8
 *      B = (298Y + 519 U- 71117)>>8
 * RGB --> YUV
 *      Y = ( 66R + 129G + 25B)>>8 + 16
 *      U = (-38R - 74G + 112B)>>8 +128
 *      V = (112R - 94G - 18*B)>>8 + 128
 */

static unsigned int clip_value(int value, int min, int max) {
    return value < min ? min : value > max ? max : value;
}

static void rgb_to_yuv_pixel(int r, int g, int b,
        unsigned char *y, unsigned char *u, unsigned char *v) {
    int shift = 8;
    int offset0 = 16, offset1 = 128;
    int c0 = 66, c1 = 129, c2 = 25, c3 = -38, c4 = -74,
        c5 = 112, c6 = 112, c7 = -94, c8 = -18;

    int y_val = ((c0 * r + c1 * g + c2 * c2) >> shift) + offset0;
    *y = clip_value(y_val, 0, 255);

    if (u != NULL && v != NULL) {
        int u_val = ((c3 * r + c4 * g + c5 * b) >> shift) + offset1;
        int v_val = ((c6 * r + c7 * g + c8 * b) >> shift) + offset1;
        *u = clip_value(u_val, 0, 255);
        *v = clip_value(v_val, 0, 255);
    }
}

static void yuv_to_rgb_pixel(int y, int u, int v, unsigned char *rgb) {
    int shift = 8;
    int offset1 = -57344, offset2 = 34739, offset3 = -71117;
    int c0 = 298, c1 = 411, c2 = -101, c3 = -211, c4 = 519;

    int r = (c0 * y + c1 * v + offset1) >> shift;
    int g = (c0 * y + c2 * u + c3 * v + offset2) >> shift;
    int b = (c0 * y + c4 * u + offset3) >> shift;
    rgb[0] = clip_value(r, 0, 255);
    rgb[1] = clip_value(g, 0, 255);
    rgb[2] = clip_value(b, 0, 255);
}

unsigned int convert_rgb_bgr(unsigned char *rgb,
        unsigned short width, unsigned short height) {
    unsigned int rgb_size = width * height * 3;
    for (unsigned int i = 0; i < rgb_size; i += 3) {
        unsigned char c = *(rgb + i);
        *(rgb + i) = *(rgb + i + 2);
        *(rgb + i + 2) = c;
    }

    return width * height * 3;
}

unsigned int convert_nv12_nv21(unsigned char *yuv,
        unsigned short width, unsigned short height) {
    unsigned int y_size = width * height;
    unsigned int uv_size = width * height / 2;

    for (unsigned int i = 0; i < uv_size; i += 2) {
        unsigned char c = *(yuv + y_size + i);
        *(yuv + y_size + i) = *(yuv + y_size + i + 1);
        *(yuv + y_size + i + 1) = c;
    }

    return width * height * 3 / 2;
}

unsigned int rgb_to_nv12(const unsigned char *rgb,
        unsigned short width, unsigned short height,
        unsigned char *nv12_buf, unsigned int buf_size) {
    if (buf_size < width * height * 3 / 2)
        return 0;

    for (unsigned int h = 0; h < height; h += 2) {
        for (unsigned int w = 0; w < width; w += 2) {
            unsigned int rgb_offset = (width * h + w) * 3;
            unsigned int y_offset   = width * h + w;
            unsigned int uv_offset  = width * height + width * h / 2 + w;
            int r = *(rgb + rgb_offset);
            int g = *(rgb + rgb_offset + 1);
            int b = *(rgb + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b,
                    nv12_buf + y_offset,
                    nv12_buf + uv_offset,
                    nv12_buf + uv_offset + 1);

            rgb_offset  += 3;
            y_offset    += 1;
            r = *(rgb + rgb_offset);
            g = *(rgb + rgb_offset + 1);
            b = *(rgb + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b, nv12_buf + y_offset, NULL, NULL);

            rgb_offset  = (width * (h + 1) + w) * 3;
            y_offset    = width * (h + 1) + w;
            r = *(rgb + rgb_offset);
            g = *(rgb + rgb_offset + 1);
            b = *(rgb + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b, nv12_buf + y_offset, NULL, NULL);

            rgb_offset  += 3;
            y_offset    += 1;
            r = *(rgb + rgb_offset);
            g = *(rgb + rgb_offset + 1);
            b = *(rgb + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b, nv12_buf + y_offset, NULL, NULL);
        }
    }

    return width * height * 3 / 2;
}

unsigned int bgr_to_nv21(const unsigned char *bgr,
        unsigned short width, unsigned short height,
        unsigned char *nv21_buf, unsigned int buf_size) {
    return rgb_to_nv12(bgr, width, height, nv21_buf, buf_size);
}

unsigned int nv12_to_rgb(const unsigned char *nv12,
        unsigned short width, unsigned short height,
        unsigned char *rgb_buf, unsigned int buf_size) {
    if (buf_size < width * height * 3)
        return 0;

    unsigned int y_offset = 0, uv_offset = 0, rgb_offset = 0;
    int y = 0, u = 0, v = 0;

    for (int h = 0; h < height; h += 2) {
        for (int w = 0; w < width; w += 2) {
            // 四个像素点共用一个UV
            uv_offset   = width * height + width * h / 2 + w;
            u = nv12[uv_offset];
            v = nv12[uv_offset + 1];

            // 1-1
            y_offset    = width * h + w;
            rgb_offset  = (width * h + w) * 3;
            y = nv12[y_offset];
            yuv_to_rgb_pixel(y, u, v, rgb_buf + rgb_offset);

            // 1-2
            y_offset    += 1;
            rgb_offset  += 3;
            y = nv12[y_offset];
            yuv_to_rgb_pixel(y, u, v, rgb_buf + rgb_offset);

            // 2-1
            y_offset    = width * (h + 1) + w;
            rgb_offset  = (width * (h + 1) + w) * 3;
            y = nv12[y_offset];
            yuv_to_rgb_pixel(y, u, v, rgb_buf + rgb_offset);

            // 2-2
            y_offset    += 1;
            rgb_offset  += 3;
            y = nv12[y_offset];
            yuv_to_rgb_pixel(y, u, v, rgb_buf + rgb_offset);
        }
    }

    return width * height * 3;
}

unsigned int nv21_to_bgr(const unsigned char *nv21,
        unsigned short width, unsigned short height,
        unsigned char *bgr_buf, unsigned int buf_size) {
    return nv12_to_rgb(nv21, width, height, bgr_buf, buf_size);
}
