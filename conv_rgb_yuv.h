/*
 * conv_rgb_yuv.h
 *
 *  Created on: 2022年1月23日
 *      Author: ljm
 */

#ifndef CONV_RGB_YUV_H_
#define CONV_RGB_YUV_H_

extern unsigned int convert_rgb_bgr(unsigned char *rgb_or_bgr,
        unsigned short width, unsigned short height);

extern unsigned int convert_nv12_nv21(unsigned char *nv12_or_nv21,
        unsigned short width, unsigned short height);

extern unsigned int rgb_to_nv12(const unsigned char *rgb,
        unsigned short width, unsigned short height,
        unsigned char *nv12_buf, unsigned int buf_size);

extern unsigned int bgr_to_nv21(const unsigned char *bgr,
        unsigned short width, unsigned short height,
        unsigned char *nv21_buf, unsigned int buf_size);

extern unsigned int nv12_to_rgb(const unsigned char *nv12,
        unsigned short width, unsigned short height,
        unsigned char *rgb_buf, unsigned int buf_size);

extern unsigned int nv21_to_bgr(const unsigned char *nv21,
        unsigned short width, unsigned short height,
        unsigned char *bgr_buf, unsigned int buf_size);

#endif /* CONV_RGB_YUV_H_ */
