/*
 * gsample.c
 *
 * ==================================
 *
 * Gradient Sample (the rules of RIS)
 *
 * ==================================
 *
 * Author: 2016 zvezdochiot (mykaralw@yandex.ru)
 *
 * Public Domain Mark 1.0
 * No Copyright
 *
 */

#include "../libris.h"

// Upscale by a factor of N from source (sp) to dest (dp)
// Expects 32 bit alignment (RGBA 4BPP) for both source and dest pointers
void gsample(uint32_t *sp,  uint32_t *dp, int Xres, int Yres, int scale_factor)
{
    int i, j, d, k, l, dx, dy, ix, iy, iz, ir, isz, stepl, deststep;
    int n = 9;
    int prevline, nextline;
    uint32_t wt;
    ARGBpixel w[n];
    ARGBpixel wm, wr;
    uint32_t *dest = (uint32_t *) dp;

    double imx, k0, kx, ky, kz, dd;

    deststep = Xres * scale_factor;
    ir = 2 * scale_factor;
    isz = ir * ir;
    dd = 1.0 / (double)isz;

    for (j = 0; j < Yres; j++)
    {
        if (j > 0)      prevline = -Xres;
        else prevline = 0;
        if (j < Yres-1) nextline =  Xres;
        else nextline = 0;

        for (i = 0; i < Xres; i++)
        {
            wt = *(sp + prevline);
            w[1] = ARGBtoPixel(wt);
            wt = *sp;
            w[4] = ARGBtoPixel(wt);
            wt = *(sp + nextline);
            w[7] = ARGBtoPixel(wt);

            if (i > 0)
            {
                wt = *(sp + prevline - 1);
                w[0] = ARGBtoPixel(wt);
                wt = *(sp - 1);
                w[3] = ARGBtoPixel(wt);
                wt = *(sp + nextline - 1);
                w[6] = ARGBtoPixel(wt);
            }
            else
            {
                w[0] = w[1];
                w[3] = w[4];
                w[6] = w[7];
            }
            if (i < Xres-1)
            {
                wt = *(sp + prevline + 1);
                w[2] = ARGBtoPixel(wt);
                wt = *(sp + 1);
                w[5] = ARGBtoPixel(wt);
                wt = *(sp + nextline + 1);
                w[8] = ARGBtoPixel(wt);
            }
            else
            {
                w[2] = w[1];
                w[5] = w[4];
                w[8] = w[7];
            }
            for (d = 0; d < BYTE_SIZE_RGBA_4BPP; d++)
            {
                imx = (double)w[4].c[d];
                imx *= 36.0;
                imx -= (double)w[0].c[d];
                imx -= (double)w[1].c[d];
                imx -= (double)w[1].c[d];
                imx -= (double)w[2].c[d];
                imx -= (double)w[3].c[d];
                imx -= (double)w[3].c[d];
                imx -= (double)w[5].c[d];
                imx -= (double)w[5].c[d];
                imx -= (double)w[6].c[d];
                imx -= (double)w[7].c[d];
                imx -= (double)w[7].c[d];
                imx -= (double)w[8].c[d];
                imx /= 24.0;
                wm.c[d] = ByteClamp((int)(imx + 0.5));
            }
            for (k =  0; k < scale_factor; k++)
            {
                dx = (k + k - scale_factor + 1);
                ix = (dx < 0) ? 3 : 5;
                stepl = 0;
                for (l =  0; l < scale_factor; l++)
                {
                    dy = (l + l - scale_factor + 1);
                    iy = (dy < 0) ? 1 : 7;
                    k0 = (double)isz;
                    kx = (double)ABS(dx * ir);
                    ky = (double)ABS(dy * ir);
                    if (dx < 0)
                    {
                        iz = (dy < 0) ? 0 : 6;
                    } else {
                        iz = (dy < 0) ? 2 : 8;
                    }
                    kz = (double)ABS(dx * dy);
                    kx -= kz;
                    ky -= kz;
                    k0 -= kz;
                    k0 -= kx;
                    k0 -= ky;
                    for (d = 0; d < BYTE_SIZE_RGBA_4BPP; d++)
                    {
                        imx = (k0 * (double)wm.c[d]);
                        imx += (kx * (double)w[ix].c[d]);
                        imx += (ky * (double)w[iy].c[d]);
                        imx += (kz * (double)w[iz].c[d]);
                        imx *= dd;
                        wr.c[d] = ByteClamp((int)(imx + 0.5));
                    }
                    wt = PixeltoARGB(wr);
                    *(dest + stepl) = wt;
                    stepl += deststep;
                }
                dest++;
            }
            sp++;
        }
        dest += deststep * (scale_factor - 1);
    }
}

// gsample_2x
//
// Scales image in *sp up by 2x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 2 * Xres * 2 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void gsample_2x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    gsample(sp, dp, Xres, Yres, GSAMPLE_2X);
}

// gsample_3x
//
// Scales image in *sp up by 3x into *dp
//
// *sp : pointer to source uint32 buffer of Xres * Yres, 4BPP RGBA
// *dp : pointer to output uint32 buffer of 3 * Xres * 3 * Yres, 4BPP RGBA
// Xres, Yres: resolution of source image
//
void gsample_3x(uint32_t * sp,  uint32_t * dp, int Xres, int Yres)
{
    gsample(sp, dp, Xres, Yres, GSAMPLE_3X);
}
