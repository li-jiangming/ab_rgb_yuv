/*
 * conv_rgb_yuv.h
 *
 *  Created on: 2022/1/23
 *      Author: ljm
 */

#ifndef CONV_RGB_YUV_H_
#define CONV_RGB_YUV_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NV12/YUV420SP     NV21/YVU420SP
 *  Y  Y  Y  Y        Y  Y  Y  Y
 *   UV    UV          VU    VU
 *  Y  Y  Y  Y        Y  Y  Y  Y
 *
 * YUV420P           YVU420P
 *  Y  Y  Y  Y        Y  Y  Y  Y
 *  Y  Y  Y  Y        Y  Y  Y  Y
 *  U     U           V     V
 *  V     V           U     U
 *
 * RGB               BGR
 * RGBRGBRGBRGB      BGRBGRBGRBGR
 * RGBRGBRGBRGB      BGRBGRBGRBGR
 */

extern unsigned int convert_rgb_bgr(unsigned char *rgb_or_bgr,
        unsigned short width, unsigned short height);

extern unsigned int convert_yuv420sp_yvu420sp(unsigned char *yuv420sp_or_yvu420sp,
        unsigned short width, unsigned short height);

extern unsigned int convert_yuv420p_yvu420p(unsigned char *yuv420p_or_yvu420p,
        unsigned short width, unsigned short height);

extern unsigned int rgb_to_yuv420sp(const unsigned char *rgb,
        unsigned short width, unsigned short height,
        unsigned char *yuv420sp_buf, unsigned int buf_size);

extern unsigned int bgr_to_yvu420sp(const unsigned char *bgr,
        unsigned short width, unsigned short height,
        unsigned char *yvu420sp_buf, unsigned int buf_size);

extern unsigned int rgb_to_yuv420p(const unsigned char *rgb,
        unsigned short width, unsigned short height,
        unsigned char *yuv420p_buf, unsigned int buf_size);

extern unsigned int bgr_to_yvu420p(const unsigned char *bgr,
        unsigned short width, unsigned short height,
        unsigned char *yvu420p_buf, unsigned int buf_size);

extern unsigned int yuv420sp_to_rgb(const unsigned char *yuv420sp,
        unsigned short width, unsigned short height,
        unsigned char *rgb_buf, unsigned int buf_size);

extern unsigned int yvu420sp_to_bgr(const unsigned char *yvu420sp,
        unsigned short width, unsigned short height,
        unsigned char *bgr_buf, unsigned int buf_size);

extern unsigned int yuv420p_to_rgb(const unsigned char *yuv420p,
        unsigned short width, unsigned short height,
        unsigned char *rgb_buf, unsigned int buf_size);

extern unsigned int yvu420p_to_bgr(const unsigned char *yvu420p,
        unsigned short width, unsigned short height,
        unsigned char *bgr_buf, unsigned int buf_size);

extern unsigned int yuv420p_to_yuv420sp(const unsigned char *yuv420p,
        unsigned short width, unsigned short height,
        unsigned char *yuv420sp_buf, unsigned int buf_size);

extern unsigned int yuv420sp_to_yuv420p(const unsigned char *yuv420sp,
        unsigned short width, unsigned short height,
        unsigned char *yuv420p_buf, unsigned int buf_size);

extern unsigned int yvu420p_to_yvu420sp(const unsigned char *yvu420p,
        unsigned short width, unsigned short height,
        unsigned char *yvu420sp_buf, unsigned int buf_size);

extern unsigned int yvu420sp_to_yvu420p(const unsigned char *yvu420sp,
        unsigned short width, unsigned short height,
        unsigned char *yvu420p_buf, unsigned int buf_size);

#ifdef __cplusplus
}
#endif

#endif // CONV_RGB_YUV_H_
