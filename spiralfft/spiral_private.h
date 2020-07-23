/*/////////////////////////////////////////////////////////////////////////////
//
//     spiral_private.h
//
//     This library was generated by Spiral 6.0, developed at Carnegie Mellon
//     University, http://www.spiral.net. This code is distributed under the
//     GNU general purpose software license version 2.0.
//
//     Commercial licenses and support are available through SpiralGen, Inc.
//     fft@spiralgen.com
//
*/

#ifndef __SPIRAL_PRIVATE_H__
#define __SPIRAL_PRIVATE_H__

#ifndef __SPIRAL_INTRINSICS_H__
#define __SPIRAL_INTRINSICS_H__

#include <xmmintrin.h>
#include <emmintrin.h>

#define SPIRAL_MALLOC(size)     malloc(size)
#define __INLINE  static inline

#define SPIRAL_PRIV(res, name)  static res name
#ifdef __GNUC__
#define SPIRAL_ALIGNED(a)       a __attribute__((aligned(16)))
#else
#define SPIRAL_ALIGNED(a)       __declspec(align(16)) a
#endif

typedef struct {
    void*       X;
    void*       Y;
    double      scale;
    int (*func)(void*);
    SPIRAL_ALIGNED(unsigned char    data[]);
} handle_t;

#define CPX __m128d
#define FLT __m128d
#define INT long
#define is_aligned(ptr)  (((INT)((INT*)ptr) & 0xF)==0)

/* normal constants */
#define C_INT(a) (a)
#define C_CPX(r,i) _mm_set_pd(i, r)
#define C_I  C_CPX(0, 1)
#define C_NI C_CPX(0, -1)
/* special constants */
#define C_CPXN(r,i) C_CPX(r, i)
#define C_IM(a) _mm_set_pd(a, -(a))
#define C_FLT(a) _mm_set1_pd(a)

/* data: normal constants */
#define CD_INT(a) (a)
#define CD_CPX(r,i) {r, i}
#define CD_I  {0.0,  1.0}
#define CD_NI {0.0, -1.0}
/* data: special constants */
#define CD_CPXN(r,i) {r, i}
#define CD_IM(a)     {-(a), a}
#define CD_FLT(a)    {a, a}

/* load/store */
#define LOADU_CPX(ptr) _mm_loadu_pd((double*)ptr)
#define STOREU_CPX(ptr, val) _mm_storeu_pd((double*)ptr, val)
#define LOADU_FLT(ptr)          (*((double*)ptr))
#define STOREU_FLT(ptr, val)    *((double*)ptr) = val
#define NTH_SYM(v,i) ((v)[i])
#define NTH_CPX(v,i) ((v)[i])

/* real arithmetic */
#define ADD_FLT_FLT(a,b) _mm_add_pd(a, b)
#define SUB_FLT_FLT(a,b) (_mm_sub_pd(a, b))
#define NTH_FLT(v,i) ((v)[i])
#define MUL_FLT_FLT(a, b) (_mm_mul_pd(a, b))
#define MUL_FLT_INT(a, b) (_mm_mul_pd(a, C_FLT(b)))
#define DIV_FLT_FLT(a, b) (_mm_div_pd(a, b))
#define NEG_FLT(a) _mm_sub_pd(C_FLT(0), a)

/* mixed real/integer arithmetic */
#define SUB_INT_INT(a, b) ((a)-(b))
#define MUL_INT_FLT(a, b) (MUL_FLT_FLT(C_FLT(a), b))
#define ADD_INT_FLT(a,b) ADD_FLT_FLT(C_FLT(a), b)
#define ADD_FLT_INT(a,b) ADD_FLT_FLT(a, C_FLT(b))
#define FDIV_INT_INT(a,b) C_FLT(((double)a) / (b))
#define FDIV_FLT_INT(a,b) DIV_FLT_FLT(a, C_FLT(b))
#define FDIV_INT_FLT(a,b) DIV_FLT_FLT(C_FLT(a), b)
#define FDIV_FLT_FLT(a,b) DIV_FLT_FLT(a, b)

/* complex arithmetic */
#define MUL_FLT_CPX(a, b) _mm_mul_pd(a, b)
#define ADD_CPX_CPX(a,b) _mm_add_pd(a, b)
#define SUB_CPX_CPX(a,b) _mm_sub_pd(a, b)
#define NEG_CPX(a) _mm_sub_pd(C_FLT(0), a)
#define RE_CPX(v) _mm_unpacklo_pd(v, v)
#define IM_CPX(v) _mm_unpackhi_pd(v, v)

/* MUL_I_CPX(x) : multiply complex x by I */
__INLINE __m128d MUL_I_CPX(__m128d x) {
     __m128d t = x;
     t = _mm_shuffle_pd(t,t,_MM_SHUFFLE2(0,1));
     return _mm_xor_pd(t, _mm_set_sd(-0.0));
}

/* MUL_NI_CPX(x) : multiply complex x by -I */
__INLINE __m128d MUL_NI_CPX(__m128d x) {
     __m128d t = x;
     t = _mm_xor_pd(t, _mm_set_sd(-0.0));
     return _mm_shuffle_pd(t,t,_MM_SHUFFLE2(0,1));
}

/* MUL_IM_CPX(a, x) : multiply complex x by a*I, in <a> 2 slots filled with same value */
__INLINE __m128d MUL_IM_CPX(__m128d a, __m128d x) {
     __m128d t = x;
     t = _mm_shuffle_pd(t, t,_MM_SHUFFLE2(0,1));
     //return _mm_xor_pd(_mm_mul_pd(a, t), _mm_set_sd(-0.0));
     return _mm_mul_pd(a, t);
}

/* MUL_CPX_CPX(x, y) : multiply two complex numbers, res = x * y */
__INLINE __m128d MUL_CPX_CPX(__m128d x, __m128d y) {
         __m128d xx = x; __m128d yy = y;
         __m128d re, im;
         re = _mm_unpacklo_pd(xx, xx);                 /* re = [x.re x.re] */
         re = _mm_mul_pd(re, yy);                      /* re = [x.re*y.re x.re*y.im] */
         im = _mm_unpackhi_pd(xx, xx);                 /* im = [x.im x.im] */
         yy = _mm_shuffle_pd(yy,yy,_MM_SHUFFLE2(0,1)); /* yy = [y.im y.re] */
         im = _mm_mul_pd(im, yy);                      /* im = [ x.im*y.im x.im*y.re] */
         im = _mm_xor_pd(im, _mm_set_sd(-0.0));        /* im = [-x.im*y.im x.im*y.re] */
         return _mm_add_pd(re, im);  /* res = [ x.re*y.re-x.im*y.im  x.re*y.im+x.im*y.re] */
}

/* MUL_CPXN_CPX(x, y) : multiply-conjugate two complex numbers, res = conj(x) * y,
   only last line is different */
__INLINE __m128d MUL_CPXN_CPX(__m128d x, __m128d y) {
         __m128d xx = x; __m128d yy = y;
         __m128d re, im;
         re = _mm_unpacklo_pd(xx, xx);                 /* re = [x.re x.re] */
         re = _mm_mul_pd(re, yy);                      /* re = [x.re*y.re x.re*y.im] */
         im = _mm_unpackhi_pd(xx, xx);                 /* im = [x.im x.im] */
         yy = _mm_shuffle_pd(yy,yy,_MM_SHUFFLE2(0,1)); /* yy = [y.im y.re] */
         im = _mm_mul_pd(im, yy);                      /* im = [ x.im*y.im x.im*y.re] */
         im = _mm_xor_pd(im, _mm_set_sd(-0.0));        /* im = [-x.im*y.im x.im*y.re] */
         return _mm_sub_pd(re, im);  /* res = [ x.re*y.re+x.im*y.im  x.re*y.im-x.im*y.re] */
}

#define SPIRAL_PI    3.14159265358979323846

extern double cos(double);
extern double sin(double);

__INLINE FLT COSPI_FLT(FLT a) { double aa = ((double*)&a)[0]; return C_FLT(cos(SPIRAL_PI*aa)); }
__INLINE FLT SINPI_FLT(FLT a) { double aa = ((double*)&a)[0]; return C_FLT(sin(SPIRAL_PI*aa)); }

__INLINE float cospi(double a) { return cos(SPIRAL_PI*a); }
__INLINE float sinpi(double a) { return sin(SPIRAL_PI*a); }

/* for RDFT */
#define v_lo2(a, b) (_mm_shuffle_pd(a, b, _MM_SHUFFLE2(0,0)))
#define v_hi2(a, b) (_mm_shuffle_pd(a, b, _MM_SHUFFLE2(1,1)))
#define v_rev2(a) (_mm_shuffle_pd(a, a, _MM_SHUFFLE2(0,1)))
#define v_neg23(a)     _mm_xor_pd(a, _mm_set_pd( -0.0,  0.0))
#define v_neg03(a)     _mm_xor_pd(a, _mm_set_pd( -0.0,  0.0))

#define v_neg01(a)     _mm_xor_pd(a, _mm_set_pd(  0.0, -0.0))

#endif  /* __SPIRAL_INTRINSICS_H__ */


#ifdef __cplusplus
extern "C" {
#endif

extern  SPIRAL_ALIGNED(__m128 D962[32]);

extern  SPIRAL_ALIGNED(__m128 D963[32]);

extern  SPIRAL_ALIGNED(__m128 D964[64]);

extern  SPIRAL_ALIGNED(__m128 D965[128]);

extern  SPIRAL_ALIGNED(__m128 D966[128]);

extern  SPIRAL_ALIGNED(__m128 D967[256]);

extern  SPIRAL_ALIGNED(__m128 D968[512]);

extern  SPIRAL_ALIGNED(__m128 D969[1024]);

extern  SPIRAL_ALIGNED(__m128 D970[64]);

extern  SPIRAL_ALIGNED(__m128 D971[2048]);

extern  SPIRAL_ALIGNED(__m128 D972[4096]);

extern  SPIRAL_ALIGNED(__m128 D973[512]);

extern  SPIRAL_ALIGNED(__m128 D974[4096]);

extern  SPIRAL_ALIGNED(__m128 D975[32]);

extern  SPIRAL_ALIGNED(__m128 D976[32]);

extern  SPIRAL_ALIGNED(__m128 D977[64]);

extern  SPIRAL_ALIGNED(__m128 D978[128]);

extern  SPIRAL_ALIGNED(__m128 D979[128]);

extern  SPIRAL_ALIGNED(__m128 D980[256]);

extern  SPIRAL_ALIGNED(__m128 D981[512]);

extern  SPIRAL_ALIGNED(__m128 D982[1024]);

extern  SPIRAL_ALIGNED(__m128 D983[64]);

extern  SPIRAL_ALIGNED(__m128 D984[2048]);

extern  SPIRAL_ALIGNED(__m128 D985[4096]);

extern  SPIRAL_ALIGNED(__m128 D986[512]);

extern  SPIRAL_ALIGNED(__m128 D987[4096]);

extern  SPIRAL_ALIGNED(FLT D989[210]);

extern  SPIRAL_ALIGNED(FLT D992[378]);

extern  SPIRAL_ALIGNED(FLT D996[882]);

extern  SPIRAL_ALIGNED(FLT D1003[1890]);

extern  SPIRAL_ALIGNED(FLT D1010[2046]);

extern  SPIRAL_ALIGNED(FLT D1011[1778]);

extern  SPIRAL_ALIGNED(FLT D1012[210]);

extern  SPIRAL_ALIGNED(FLT D1018[7154]);

extern  SPIRAL_ALIGNED(FLT D1040[2046]);

extern  SPIRAL_ALIGNED(FLT D1041[1778]);

extern  SPIRAL_ALIGNED(FLT D1042[210]);


#ifdef __cplusplus
}
#endif

#endif /* __SPIRAL_PRIVATE_H__ */
