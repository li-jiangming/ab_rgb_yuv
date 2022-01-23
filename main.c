/*
 * main.c
 *  Created on: 2022年1月19日
 *      Author: ljm
 */

#include "conv_rgb_yuv.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 6) {
        printf("Usage: %s csc in_file out_file width height\n"
               "csc:\n"
               "\t1. rgb24 --> nv12\n"
               "\t2. bgr24 --> nv21\n"
               "\t3. nv12 --> rgb24\n"
               "\t4. nv21 --> bgr24\n"
               "\t5. nv12 <-> nv21\n"
               "\t6. rgb24 <-> bgr24\n", argv[0]);
        return EXIT_FAILURE;
    }

    int csc = atoi(argv[1]);

    const char *in_file = argv[2];
    const char *out_file = argv[3];
    int width = atoi(argv[4]);
    int height = atoi(argv[5]);

    FILE *file = fopen(in_file, "rb");
    if (file != NULL) {
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        unsigned char *file_data = (unsigned char *) malloc(file_size);
        fread(file_data, 1, file_size, file);
        fclose(file);

        unsigned int buf_size = width * height * 3;
        unsigned char *buf = (unsigned char *) malloc(buf_size);

        int out_size = 0;
        if (1 == csc)
            out_size = rgb_to_nv12(file_data, width, height, buf, buf_size);
        else if (2 == csc)
            out_size = bgr_to_nv21(file_data, width, height, buf, buf_size);
        else if (3 == csc)
            out_size = nv12_to_rgb(file_data, width, height, buf, buf_size);
        else if (4 == csc)
            out_size = nv21_to_bgr(file_data, width, height, buf, buf_size);
        else if (5 == csc)
            out_size = convert_nv12_nv21(file_data, width, height);
        else if (6 == csc)
            out_size = convert_rgb_bgr(file_data, width, height);
        else {
            free(buf);
            buf = NULL;

            free(file_data);
            file_data = NULL;
            return -1;
        }

        file = fopen(out_file, "wb");
        if (file != NULL) {
            if (5 == csc || 6 == csc)
                fwrite(file_data, 1, out_size, file);
            else
                fwrite(buf, 1, out_size, file);

            fflush(file);
            fclose(file);
        }

        free(buf);
        buf = NULL;

        free(file_data);
        file_data = NULL;
    }

    return EXIT_SUCCESS;
}
