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

static void yuv_to_rgb_pixel(int y, int u, int v, 
        unsigned char *r, unsigned char *g, unsigned char *b) {
    int shift = 8;
    int offset1 = -57344, offset2 = 34739, offset3 = -71117;
    int c0 = 298, c1 = 411, c2 = -101, c3 = -211, c4 = 519;

    int r_val = (c0 * y + c1 * v + offset1) >> shift;
    int g_val = (c0 * y + c2 * u + c3 * v + offset2) >> shift;
    int b_val = (c0 * y + c4 * u + offset3) >> shift;
    *r = clip_value(r_val, 0, 255);
    *g = clip_value(g_val, 0, 255);
    *b = clip_value(b_val, 0, 255);
}

unsigned int convert_rgb_bgr(unsigned char *rgb_or_bgr,
        unsigned short width, unsigned short height) {
    unsigned int rgb_size = width * height * 3;
    for (unsigned int i = 0; i < rgb_size; i += 3) {
        unsigned char c = *(rgb_or_bgr + i);
        *(rgb_or_bgr + i) = *(rgb_or_bgr + i + 2);
        *(rgb_or_bgr + i + 2) = c;
    }

    return width * height * 3;
}

unsigned int convert_nv12_nv21(unsigned char *nv12_or_nv21,
        unsigned short width, unsigned short height) {
    unsigned int y_size = width * height;
    unsigned int uv_size = width * height / 2;

    for (unsigned int i = 0; i < uv_size; i += 2) {
        unsigned char c = *(nv12_or_nv21 + y_size + i);
        *(nv12_or_nv21 + y_size + i) = *(nv12_or_nv21 + y_size + i + 1);
        *(nv12_or_nv21 + y_size + i + 1) = c;
    }

    return width * height * 3 / 2;
}

unsigned int convert_yuv420p_yvu420p(unsigned char *yuv420p_or_yvu420p,
        unsigned short width, unsigned short height) {
    unsigned int uv_len = width * height / 4;
    unsigned char *u = yuv420p_or_yvu420p + width * height;
    unsigned char *v = yuv420p_or_yvu420p + width * height * 5 / 4;
    for (unsigned int i = 0; i < uv_len; ++i) {
        char c = u[i];
        u[i] = v[i];
        v[i] = c;
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
    if (buf_size < width * height * 3 / 2)
        return 0;

    for (unsigned int h = 0; h < height; h += 2) {
        for (unsigned int w = 0; w < width; w += 2) {
            unsigned int rgb_offset = (width * h + w) * 3;
            unsigned int y_offset   = width * h + w;
            unsigned int uv_offset  = width * height + width * h / 2 + w;
            int b = *(bgr + rgb_offset);
            int g = *(bgr + rgb_offset + 1);
            int r = *(bgr + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b,
                    nv21_buf + y_offset,
                    nv21_buf + uv_offset + 1,
                    nv21_buf + uv_offset);

            rgb_offset  += 3;
            y_offset    += 1;
            b = *(bgr + rgb_offset);
            g = *(bgr + rgb_offset + 1);
            r = *(bgr + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b, nv21_buf + y_offset, NULL, NULL);

            rgb_offset  = (width * (h + 1) + w) * 3;
            y_offset    = width * (h + 1) + w;
            b = *(bgr + rgb_offset);
            g = *(bgr + rgb_offset + 1);
            r = *(bgr + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b, nv21_buf + y_offset, NULL, NULL);

            rgb_offset  += 3;
            y_offset    += 1;
            b = *(bgr + rgb_offset);
            g = *(bgr + rgb_offset + 1);
            r = *(bgr + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b, nv21_buf + y_offset, NULL, NULL);
        }
    }

    return width * height * 3 / 2;
}

unsigned int rgb_to_yuv420p(const unsigned char *rgb,
        unsigned short width, unsigned short height,
        unsigned char *yuv420p_buf, unsigned int buf_size) {
    if (buf_size < width * height * 3 / 2)
        return 0;

    for (unsigned int h = 0; h < height; h += 2) {
        for (unsigned int w = 0; w < width; w += 2) {
            unsigned int rgb_offset = (width * h + w) * 3;
            unsigned int y_offset   = width * h + w;
            unsigned int u_offset  = width * height + (width * h / 2 + w) / 2;
            unsigned int v_offset  = width * height * 5 / 4 + (width * h / 2 + w) / 2;
            int r = *(rgb + rgb_offset);
            int g = *(rgb + rgb_offset + 1);
            int b = *(rgb + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b,
                    yuv420p_buf + y_offset,
                    yuv420p_buf + u_offset,
                    yuv420p_buf + v_offset);

            rgb_offset  += 3;
            y_offset    += 1;
            r = *(rgb + rgb_offset);
            g = *(rgb + rgb_offset + 1);
            b = *(rgb + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b, yuv420p_buf + y_offset, NULL, NULL);

            rgb_offset  = (width * (h + 1) + w) * 3;
            y_offset    = width * (h + 1) + w;
            r = *(rgb + rgb_offset);
            g = *(rgb + rgb_offset + 1);
            b = *(rgb + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b, yuv420p_buf + y_offset, NULL, NULL);

            rgb_offset  += 3;
            y_offset    += 1;
            r = *(rgb + rgb_offset);
            g = *(rgb + rgb_offset + 1);
            b = *(rgb + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b, yuv420p_buf + y_offset, NULL, NULL);
        }
    }

    return width * height * 3 / 2;
}

unsigned int bgr_to_yvu420p(const unsigned char *bgr,
        unsigned short width, unsigned short height,
        unsigned char *yvu420p_buf, unsigned int buf_size) {
    if (buf_size < width * height * 3 / 2)
        return 0;

    for (unsigned int h = 0; h < height; h += 2) {
        for (unsigned int w = 0; w < width; w += 2) {
            unsigned int rgb_offset = (width * h + w) * 3;
            unsigned int y_offset   = width * h + w;
            unsigned int v_offset   = width * height + (width * h / 2 + w) / 2;
            unsigned int u_offset   = width * height * 5 / 4 + 
                                      (width * h / 2 + w) / 2;
            int b = *(bgr + rgb_offset);
            int g = *(bgr + rgb_offset + 1);
            int r = *(bgr + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b,
                    yvu420p_buf + y_offset,
                    yvu420p_buf + u_offset,
                    yvu420p_buf + v_offset);

            rgb_offset  += 3;
            y_offset    += 1;
            b = *(bgr + rgb_offset);
            g = *(bgr + rgb_offset + 1);
            r = *(bgr + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b, yvu420p_buf + y_offset, NULL, NULL);

            rgb_offset  = (width * (h + 1) + w) * 3;
            y_offset    = width * (h + 1) + w;
            b = *(bgr + rgb_offset);
            g = *(bgr + rgb_offset + 1);
            r = *(bgr + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b, yvu420p_buf + y_offset, NULL, NULL);

            rgb_offset  += 3;
            y_offset    += 1;
            b = *(bgr + rgb_offset);
            g = *(bgr + rgb_offset + 1);
            r = *(bgr + rgb_offset + 2);
            rgb_to_yuv_pixel(r, g, b, yvu420p_buf + y_offset, NULL, NULL);
        }
    }

    return width * height * 3 / 2;
}

unsigned int nv12_to_rgb(const unsigned char *nv12,
        unsigned short width, unsigned short height,
        unsigned char *rgb_buf, unsigned int buf_size) {
    if (buf_size < width * height * 3)
        return 0;

    for (int h = 0; h < height; h += 2) {
        for (int w = 0; w < width; w += 2) {
            // 四个像素点共用一个UV
            unsigned int uv_offset   = width * height + width * h / 2 + w;
            int u = nv12[uv_offset];
            int v = nv12[uv_offset + 1];

            // 1-1
            unsigned int y_offset    = width * h + w;
            unsigned int rgb_offset  = (width * h + w) * 3;
            int y = nv12[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    rgb_buf + rgb_offset,
                    rgb_buf + rgb_offset + 1,
                    rgb_buf + rgb_offset + 2);

            // 1-2
            y_offset    += 1;
            rgb_offset  += 3;
            y = nv12[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    rgb_buf + rgb_offset,
                    rgb_buf + rgb_offset + 1,
                    rgb_buf + rgb_offset + 2);

            // 2-1
            y_offset    = width * (h + 1) + w;
            rgb_offset  = (width * (h + 1) + w) * 3;
            y = nv12[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    rgb_buf + rgb_offset,
                    rgb_buf + rgb_offset + 1,
                    rgb_buf + rgb_offset + 2);

            // 2-2
            y_offset    += 1;
            rgb_offset  += 3;
            y = nv12[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    rgb_buf + rgb_offset,
                    rgb_buf + rgb_offset + 1,
                    rgb_buf + rgb_offset + 2);
        }
    }

    return width * height * 3;
}

unsigned int nv21_to_bgr(const unsigned char *nv21,
        unsigned short width, unsigned short height,
        unsigned char *bgr_buf, unsigned int buf_size) {
    if (buf_size < width * height * 3)
        return 0;

    for (int h = 0; h < height; h += 2) {
        for (int w = 0; w < width; w += 2) {
            // 四个像素点共用一个UV
            unsigned int uv_offset   = width * height + width * h / 2 + w;
            int v = nv21[uv_offset];
            int u = nv21[uv_offset + 1];

            // 1-1
            unsigned int y_offset    = width * h + w;
            unsigned int rgb_offset  = (width * h + w) * 3;
            int y = nv21[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    bgr_buf + rgb_offset + 2,
                    bgr_buf + rgb_offset + 1,
                    bgr_buf + rgb_offset);

            // 1-2
            y_offset    += 1;
            rgb_offset  += 3;
            y = nv21[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    bgr_buf + rgb_offset + 2,
                    bgr_buf + rgb_offset + 1,
                    bgr_buf + rgb_offset);

            // 2-1
            y_offset    = width * (h + 1) + w;
            rgb_offset  = (width * (h + 1) + w) * 3;
            y = nv21[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    bgr_buf + rgb_offset + 2,
                    bgr_buf + rgb_offset + 1,
                    bgr_buf + rgb_offset);

            // 2-2
            y_offset    += 1;
            rgb_offset  += 3;
            y = nv21[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    bgr_buf + rgb_offset + 2,
                    bgr_buf + rgb_offset + 1,
                    bgr_buf + rgb_offset);
        }
    }

    return width * height * 3;
}

unsigned int yuv420p_to_rgb(const unsigned char *yuv420p,
        unsigned short width, unsigned short height,
        unsigned char *rgb_buf, unsigned int buf_size) {
    if (buf_size < width * height * 3)
        return 0;

    for (int h = 0; h < height; h += 2) {
        for (int w = 0; w < width; w += 2) {
            // 四个像素点共用一个UV
            unsigned int uv_offset   = (width * h / 2 + w) / 2;
            unsigned int u_offset    = width * height + uv_offset;
            unsigned int v_offset    = width * height * 5 / 4 + uv_offset;
            int u = yuv420p[u_offset];
            int v = yuv420p[v_offset];

            // 1-1
            unsigned y_offset    = width * h + w;
            unsigned int rgb_offset  = (width * h + w) * 3;
            int y = yuv420p[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    rgb_buf + rgb_offset,
                    rgb_buf + rgb_offset + 1,
                    rgb_buf + rgb_offset + 2);

            // 1-2
            y_offset    += 1;
            rgb_offset  += 3;
            y = yuv420p[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    rgb_buf + rgb_offset,
                    rgb_buf + rgb_offset + 1,
                    rgb_buf + rgb_offset + 2);

            // 2-1
            y_offset    = width * (h + 1) + w;
            rgb_offset  = (width * (h + 1) + w) * 3;
            y = yuv420p[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    rgb_buf + rgb_offset,
                    rgb_buf + rgb_offset + 1,
                    rgb_buf + rgb_offset + 2);

            // 2-2
            y_offset    += 1;
            rgb_offset  += 3;
            y = yuv420p[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    rgb_buf + rgb_offset,
                    rgb_buf + rgb_offset + 1,
                    rgb_buf + rgb_offset + 2);
        }
    }

    return width * height * 3;
}

unsigned int yvu420p_to_bgr(const unsigned char *yvu420p,
        unsigned short width, unsigned short height,
        unsigned char *bgr_buf, unsigned int buf_size) {
    if (buf_size < width * height * 3)
        return 0;

    for (int h = 0; h < height; h += 2) {
        for (int w = 0; w < width; w += 2) {
            // 四个像素点共用一个UV
            unsigned int uv_offset   = (width * h / 2 + w) / 2;
            unsigned int v_offset    = width * height + uv_offset;
            unsigned int u_offset    = width * height * 5 / 4 + uv_offset;
            int v = yvu420p[v_offset];
            int u = yvu420p[u_offset];

            // 1-1
            unsigned int y_offset    = width * h + w;
            unsigned int rgb_offset  = (width * h + w) * 3;
            int y = yvu420p[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    bgr_buf + rgb_offset + 2,
                    bgr_buf + rgb_offset + 1,
                    bgr_buf + rgb_offset);

            // 1-2
            y_offset    += 1;
            rgb_offset  += 3;
            y = yvu420p[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    bgr_buf + rgb_offset + 2,
                    bgr_buf + rgb_offset + 1,
                    bgr_buf + rgb_offset);

            // 2-1
            y_offset    = width * (h + 1) + w;
            rgb_offset  = (width * (h + 1) + w) * 3;
            y = yvu420p[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    bgr_buf + rgb_offset + 2,
                    bgr_buf + rgb_offset + 1,
                    bgr_buf + rgb_offset);

            // 2-2
            y_offset    += 1;
            rgb_offset  += 3;
            y = yvu420p[y_offset];
            yuv_to_rgb_pixel(y, u, v, 
                    bgr_buf + rgb_offset + 2,
                    bgr_buf + rgb_offset + 1,
                    bgr_buf + rgb_offset);
        }
    }

    return width * height * 3;
}
