/*/////////////////////////////////////////////////////////////////////////////
//
//     spiral_fft.h
//
//     This library was generated by Spiral 6.0, developed at Carnegie Mellon
//     University, http://www.spiral.net. This code is distributed under the
//     GNU general purpose software license version 2.0.
//
//     Commercial licenses and support are available through SpiralGen, Inc.
//     fft@spiralgen.com
//
*/

#if !defined( __SPIRAL_FFT_H__ )
#define __SPIRAL_FFT_H__

#define SPIRAL_API(res, name)	res name

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SPIRAL_OK = 0, 
    SPIRAL_SIZE_NOT_SUPPORTED,
    SPIRAL_INVALID_PARAM,
    SPIRAL_OUT_OF_MEMORY
} spiral_status_t;

/*****************************************************
 *  spiral_fft_float
 *  Complex DFT
 */
SPIRAL_API(spiral_status_t, spiral_fft_float)(int n, int sign, const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftinv_float_2)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftinv_float_4)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftinv_float_8)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftinv_float_16)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftinv_float_32)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftinv_float_64)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftinv_float_128)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftinv_float_256)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftinv_float_512)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftinv_float_1024)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftinv_float_2048)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftinv_float_4096)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftinv_float_8192)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftfwd_float_2)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftfwd_float_4)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftfwd_float_8)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftfwd_float_16)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftfwd_float_32)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftfwd_float_64)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftfwd_float_128)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftfwd_float_256)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftfwd_float_512)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftfwd_float_1024)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftfwd_float_2048)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftfwd_float_4096)(const float *pSrc, float *pDst );
SPIRAL_API(spiral_status_t, spiral_fftfwd_float_8192)(const float *pSrc, float *pDst );

#ifdef __cplusplus
}
#endif

#endif /* __SPIRAL_FFT_H__ */

