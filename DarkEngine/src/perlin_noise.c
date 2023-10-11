// noise1234
//
// Author: Stefan Gustavson, 2003-2005
// Contact: stefan.gustavson@liu.se
//
// This code was GPL licensed until February 2011.
// As the original author of this code, I hereby
// release it into the public domain.
// Please feel free to use it for whatever you want.
// Credit is appreciated where appropriate, and I also
// appreciate being told where this code finds any use,
// but you may do as you like.

/*
 * This implementation is "Improved Noise" as presented by
 * Ken Perlin at Siggraph 2002. The 3D function is a direct port
 * of his Java reference code which was once publicly available
 * on www.noisemachine.com (although I cleaned it up, made it
 * faster and made the code more readable), but the 1D, 2D and
 * 4D functions were implemented from scratch by me.
 *
 * This is a backport to C of my improved noise class in C++
 * which was included in the Aqsis renderer project.
 * It is highly reusable without source code modifications.
 *
 */

#include "perlin_noise.h"
#include <stdlib.h>
#include <string.h>

// This is the new and improved, C(2) continuous interpolant
#define FADE(t) ( t * t * t * ( t * ( t * 6 - 15 ) + 10 ) )

#define FASTFLOOR(x) ( ((int)(x)<(x)) ? ((int)x) : ((int)x-1 ) )
#define LERP(t, a, b) ((a) + (t)*((b)-(a)))

void perlin_noise_randomize_perm(unsigned char perm[512]){
    for (int i = 0; i < 256; i++) perm[i]=i;
    for (int i = 0; i < 256; i++){
        int x = perm[i];
        int j = rand()%256;
        perm[i] = perm[j];
        perm[j] = x;
    }
    memcpy(perm+256,perm,256*sizeof(*perm));
}

//---------------------------------------------------------------------

/*
 * Helper functions to compute gradients-dot-residualvectors (1D to 4D)
 * Note that these generate gradients of more than unit length. To make
 * a close match with the value range of classic Perlin noise, the final
 * noise values need to be rescaled. To match the RenderMan noise in a
 * statistical sense, the approximate scaling values (empirically
 * determined from test renderings) are:
 * 1D noise needs rescaling with 0.188
 * 2D noise needs rescaling with 0.507
 * 3D noise needs rescaling with 0.936
 * 4D noise needs rescaling with 0.87
 * Note that these noise functions are the most practical and useful
 * signed version of Perlin noise. To return values according to the
 * RenderMan specification from the SL noise() and pnoise() functions,
 * the noise values need to be scaled and offset to [0,1], like this:
 * float SLnoise = (noise3(x,y,z) + 1.0) * 0.5;
 */

float grad1( int hash, float x ) {
    int h = hash & 15;
    float grad = 1.0 + (h & 7);  // Gradient value 1.0, 2.0, ..., 8.0
    if (h&8) grad = -grad;         // and a random sign for the gradient
    return ( grad * x );           // Multiply the gradient with the distance
}

float grad2( int hash, float x, float y ) {
    int h = hash & 7;      // Convert low 3 bits of hash code
    float u = h<4 ? x : y;  // into 8 simple gradient directions,
    float v = h<4 ? y : x;  // and compute the dot product with (x,y).
    return ((h&1)? -u : u) + ((h&2)? -2.0*v : 2.0*v);
}

float grad3( int hash, float x, float y , float z ) {
    int h = hash & 15;     // Convert low 4 bits of hash code into 12 simple
    float u = h<8 ? x : y; // gradient directions, and compute dot product.
    float v = h<4 ? y : h==12||h==14 ? x : z; // Fix repeats at h = 12 to 15
    return ((h&1)? -u : u) + ((h&2)? -v : v);
}

float grad4( int hash, float x, float y, float z, float t ) {
    int h = hash & 31;      // Convert low 5 bits of hash code into 32 simple
    float u = h<24 ? x : y; // gradient directions, and compute dot product.
    float v = h<16 ? y : z;
    float w = h<8 ? z : t;
    return ((h&1)? -u : u) + ((h&2)? -v : v) + ((h&4)? -w : w);
}

//---------------------------------------------------------------------
/** 1D float Perlin noise, SL "noise()"
 */
float perlin_noise1(unsigned char perm[512], float x)
{
    int ix0, ix1;
    float fx0, fx1;
    float s, n0, n1;

    ix0 = FASTFLOOR( x ); // Integer part of x
    fx0 = x - ix0;       // Fractional part of x
    fx1 = fx0 - 1.0f;
    ix1 = ( ix0+1 ) & 0xff;
    ix0 = ix0 & 0xff;    // Wrap to 0..255

    s = FADE( fx0 );

    n0 = grad1( perm[ ix0 ], fx0 );
    n1 = grad1( perm[ ix1 ], fx1 );
    return 0.188f * ( LERP( s, n0, n1 ) );
}

//---------------------------------------------------------------------
/** 1D float Perlin periodic noise, SL "pnoise()"
 */
float periodic_perlin_noise1(unsigned char perm[512], float x, int px)
{
    int ix0, ix1;
    float fx0, fx1;
    float s, n0, n1;

    ix0 = FASTFLOOR( x ); // Integer part of x
    fx0 = x - ix0;       // Fractional part of x
    fx1 = fx0 - 1.0f;
    ix1 = (( ix0 + 1 ) % px) & 0xff; // Wrap to 0..px-1 *and* wrap to 0..255
    ix0 = ( ix0 % px ) & 0xff;      // (because px might be greater than 256)

    s = FADE( fx0 );

    n0 = grad1( perm[ ix0 ], fx0 );
    n1 = grad1( perm[ ix1 ], fx1 );
    return 0.188f * ( LERP( s, n0, n1 ) );
}


//---------------------------------------------------------------------
/** 2D float Perlin noise.
 */
float perlin_noise2(unsigned char perm[512], float x, float y)
{
    int ix0, iy0, ix1, iy1;
    float fx0, fy0, fx1, fy1;
    float s, t, nx0, nx1, n0, n1;

    ix0 = FASTFLOOR( x ); // Integer part of x
    iy0 = FASTFLOOR( y ); // Integer part of y
    fx0 = x - ix0;        // Fractional part of x
    fy0 = y - iy0;        // Fractional part of y
    fx1 = fx0 - 1.0f;
    fy1 = fy0 - 1.0f;
    ix1 = (ix0 + 1) & 0xff;  // Wrap to 0..255
    iy1 = (iy0 + 1) & 0xff;
    ix0 = ix0 & 0xff;
    iy0 = iy0 & 0xff;
    
    t = FADE( fy0 );
    s = FADE( fx0 );

    nx0 = grad2(perm[ix0 + perm[iy0]], fx0, fy0);
    nx1 = grad2(perm[ix0 + perm[iy1]], fx0, fy1);
    n0 = LERP( t, nx0, nx1 );

    nx0 = grad2(perm[ix1 + perm[iy0]], fx1, fy0);
    nx1 = grad2(perm[ix1 + perm[iy1]], fx1, fy1);
    n1 = LERP(t, nx0, nx1);

    return 0.507f * ( LERP( s, n0, n1 ) );
}

//---------------------------------------------------------------------
/** 2D float Perlin periodic noise.
 */
float periodic_perlin_noise2(unsigned char perm[512], float x, float y, int px, int py)
{
    int ix0, iy0, ix1, iy1;
    float fx0, fy0, fx1, fy1;
    float s, t, nx0, nx1, n0, n1;

    ix0 = FASTFLOOR( x ); // Integer part of x
    iy0 = FASTFLOOR( y ); // Integer part of y
    fx0 = x - ix0;        // Fractional part of x
    fy0 = y - iy0;        // Fractional part of y
    fx1 = fx0 - 1.0f;
    fy1 = fy0 - 1.0f;
    ix1 = (( ix0 + 1 ) % px) & 0xff;  // Wrap to 0..px-1 and wrap to 0..255
    iy1 = (( iy0 + 1 ) % py) & 0xff;  // Wrap to 0..py-1 and wrap to 0..255
    ix0 = ( ix0 % px ) & 0xff;
    iy0 = ( iy0 % py ) & 0xff;
    
    t = FADE( fy0 );
    s = FADE( fx0 );

    nx0 = grad2(perm[ix0 + perm[iy0]], fx0, fy0);
    nx1 = grad2(perm[ix0 + perm[iy1]], fx0, fy1);
    n0 = LERP( t, nx0, nx1 );

    nx0 = grad2(perm[ix1 + perm[iy0]], fx1, fy0);
    nx1 = grad2(perm[ix1 + perm[iy1]], fx1, fy1);
    n1 = LERP(t, nx0, nx1);

    return 0.507f * ( LERP( s, n0, n1 ) );
}


//---------------------------------------------------------------------
/** 3D float Perlin noise.
 */
float perlin_noise3(unsigned char perm[512], float x, float y, float z)
{
    int ix0, iy0, ix1, iy1, iz0, iz1;
    float fx0, fy0, fz0, fx1, fy1, fz1;
    float s, t, r;
    float nxy0, nxy1, nx0, nx1, n0, n1;

    ix0 = FASTFLOOR( x ); // Integer part of x
    iy0 = FASTFLOOR( y ); // Integer part of y
    iz0 = FASTFLOOR( z ); // Integer part of z
    fx0 = x - ix0;        // Fractional part of x
    fy0 = y - iy0;        // Fractional part of y
    fz0 = z - iz0;        // Fractional part of z
    fx1 = fx0 - 1.0f;
    fy1 = fy0 - 1.0f;
    fz1 = fz0 - 1.0f;
    ix1 = ( ix0 + 1 ) & 0xff; // Wrap to 0..255
    iy1 = ( iy0 + 1 ) & 0xff;
    iz1 = ( iz0 + 1 ) & 0xff;
    ix0 = ix0 & 0xff;
    iy0 = iy0 & 0xff;
    iz0 = iz0 & 0xff;
    
    r = FADE( fz0 );
    t = FADE( fy0 );
    s = FADE( fx0 );

    nxy0 = grad3(perm[ix0 + perm[iy0 + perm[iz0]]], fx0, fy0, fz0);
    nxy1 = grad3(perm[ix0 + perm[iy0 + perm[iz1]]], fx0, fy0, fz1);
    nx0 = LERP( r, nxy0, nxy1 );

    nxy0 = grad3(perm[ix0 + perm[iy1 + perm[iz0]]], fx0, fy1, fz0);
    nxy1 = grad3(perm[ix0 + perm[iy1 + perm[iz1]]], fx0, fy1, fz1);
    nx1 = LERP( r, nxy0, nxy1 );

    n0 = LERP( t, nx0, nx1 );

    nxy0 = grad3(perm[ix1 + perm[iy0 + perm[iz0]]], fx1, fy0, fz0);
    nxy1 = grad3(perm[ix1 + perm[iy0 + perm[iz1]]], fx1, fy0, fz1);
    nx0 = LERP( r, nxy0, nxy1 );

    nxy0 = grad3(perm[ix1 + perm[iy1 + perm[iz0]]], fx1, fy1, fz0);
    nxy1 = grad3(perm[ix1 + perm[iy1 + perm[iz1]]], fx1, fy1, fz1);
    nx1 = LERP( r, nxy0, nxy1 );

    n1 = LERP( t, nx0, nx1 );
    
    return 0.936f * ( LERP( s, n0, n1 ) );
}

//---------------------------------------------------------------------
/** 3D float Perlin periodic noise.
 */
float periodic_perlin_noise3(unsigned char perm[512], float x, float y, float z, int px, int py, int pz)
{
    int ix0, iy0, ix1, iy1, iz0, iz1;
    float fx0, fy0, fz0, fx1, fy1, fz1;
    float s, t, r;
    float nxy0, nxy1, nx0, nx1, n0, n1;

    ix0 = FASTFLOOR( x ); // Integer part of x
    iy0 = FASTFLOOR( y ); // Integer part of y
    iz0 = FASTFLOOR( z ); // Integer part of z
    fx0 = x - ix0;        // Fractional part of x
    fy0 = y - iy0;        // Fractional part of y
    fz0 = z - iz0;        // Fractional part of z
    fx1 = fx0 - 1.0f;
    fy1 = fy0 - 1.0f;
    fz1 = fz0 - 1.0f;
    ix1 = (( ix0 + 1 ) % px ) & 0xff; // Wrap to 0..px-1 and wrap to 0..255
    iy1 = (( iy0 + 1 ) % py ) & 0xff; // Wrap to 0..py-1 and wrap to 0..255
    iz1 = (( iz0 + 1 ) % pz ) & 0xff; // Wrap to 0..pz-1 and wrap to 0..255
    ix0 = ( ix0 % px ) & 0xff;
    iy0 = ( iy0 % py ) & 0xff;
    iz0 = ( iz0 % pz ) & 0xff;
    
    r = FADE( fz0 );
    t = FADE( fy0 );
    s = FADE( fx0 );

    nxy0 = grad3(perm[ix0 + perm[iy0 + perm[iz0]]], fx0, fy0, fz0);
    nxy1 = grad3(perm[ix0 + perm[iy0 + perm[iz1]]], fx0, fy0, fz1);
    nx0 = LERP( r, nxy0, nxy1 );

    nxy0 = grad3(perm[ix0 + perm[iy1 + perm[iz0]]], fx0, fy1, fz0);
    nxy1 = grad3(perm[ix0 + perm[iy1 + perm[iz1]]], fx0, fy1, fz1);
    nx1 = LERP( r, nxy0, nxy1 );

    n0 = LERP( t, nx0, nx1 );

    nxy0 = grad3(perm[ix1 + perm[iy0 + perm[iz0]]], fx1, fy0, fz0);
    nxy1 = grad3(perm[ix1 + perm[iy0 + perm[iz1]]], fx1, fy0, fz1);
    nx0 = LERP( r, nxy0, nxy1 );

    nxy0 = grad3(perm[ix1 + perm[iy1 + perm[iz0]]], fx1, fy1, fz0);
    nxy1 = grad3(perm[ix1 + perm[iy1 + perm[iz1]]], fx1, fy1, fz1);
    nx1 = LERP( r, nxy0, nxy1 );

    n1 = LERP( t, nx0, nx1 );
    
    return 0.936f * ( LERP( s, n0, n1 ) );
}


//---------------------------------------------------------------------
/** 4D float Perlin noise.
 */

float perlin_noise4(unsigned char perm[512], float x, float y, float z, float w)
{
    int ix0, iy0, iz0, iw0, ix1, iy1, iz1, iw1;
    float fx0, fy0, fz0, fw0, fx1, fy1, fz1, fw1;
    float s, t, r, q;
    float nxyz0, nxyz1, nxy0, nxy1, nx0, nx1, n0, n1;

    ix0 = FASTFLOOR( x ); // Integer part of x
    iy0 = FASTFLOOR( y ); // Integer part of y
    iz0 = FASTFLOOR( z ); // Integer part of y
    iw0 = FASTFLOOR( w ); // Integer part of w
    fx0 = x - ix0;        // Fractional part of x
    fy0 = y - iy0;        // Fractional part of y
    fz0 = z - iz0;        // Fractional part of z
    fw0 = w - iw0;        // Fractional part of w
    fx1 = fx0 - 1.0f;
    fy1 = fy0 - 1.0f;
    fz1 = fz0 - 1.0f;
    fw1 = fw0 - 1.0f;
    ix1 = ( ix0 + 1 ) & 0xff;  // Wrap to 0..255
    iy1 = ( iy0 + 1 ) & 0xff;
    iz1 = ( iz0 + 1 ) & 0xff;
    iw1 = ( iw0 + 1 ) & 0xff;
    ix0 = ix0 & 0xff;
    iy0 = iy0 & 0xff;
    iz0 = iz0 & 0xff;
    iw0 = iw0 & 0xff;

    q = FADE( fw0 );
    r = FADE( fz0 );
    t = FADE( fy0 );
    s = FADE( fx0 );

    nxyz0 = grad4(perm[ix0 + perm[iy0 + perm[iz0 + perm[iw0]]]], fx0, fy0, fz0, fw0);
    nxyz1 = grad4(perm[ix0 + perm[iy0 + perm[iz0 + perm[iw1]]]], fx0, fy0, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad4(perm[ix0 + perm[iy0 + perm[iz1 + perm[iw0]]]], fx0, fy0, fz1, fw0);
    nxyz1 = grad4(perm[ix0 + perm[iy0 + perm[iz1 + perm[iw1]]]], fx0, fy0, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );
        
    nx0 = LERP ( r, nxy0, nxy1 );

    nxyz0 = grad4(perm[ix0 + perm[iy1 + perm[iz0 + perm[iw0]]]], fx0, fy1, fz0, fw0);
    nxyz1 = grad4(perm[ix0 + perm[iy1 + perm[iz0 + perm[iw1]]]], fx0, fy1, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad4(perm[ix0 + perm[iy1 + perm[iz1 + perm[iw0]]]], fx0, fy1, fz1, fw0);
    nxyz1 = grad4(perm[ix0 + perm[iy1 + perm[iz1 + perm[iw1]]]], fx0, fy1, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );

    nx1 = LERP ( r, nxy0, nxy1 );

    n0 = LERP( t, nx0, nx1 );

    nxyz0 = grad4(perm[ix1 + perm[iy0 + perm[iz0 + perm[iw0]]]], fx1, fy0, fz0, fw0);
    nxyz1 = grad4(perm[ix1 + perm[iy0 + perm[iz0 + perm[iw1]]]], fx1, fy0, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad4(perm[ix1 + perm[iy0 + perm[iz1 + perm[iw0]]]], fx1, fy0, fz1, fw0);
    nxyz1 = grad4(perm[ix1 + perm[iy0 + perm[iz1 + perm[iw1]]]], fx1, fy0, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );

    nx0 = LERP ( r, nxy0, nxy1 );

    nxyz0 = grad4(perm[ix1 + perm[iy1 + perm[iz0 + perm[iw0]]]], fx1, fy1, fz0, fw0);
    nxyz1 = grad4(perm[ix1 + perm[iy1 + perm[iz0 + perm[iw1]]]], fx1, fy1, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad4(perm[ix1 + perm[iy1 + perm[iz1 + perm[iw0]]]], fx1, fy1, fz1, fw0);
    nxyz1 = grad4(perm[ix1 + perm[iy1 + perm[iz1 + perm[iw1]]]], fx1, fy1, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );

    nx1 = LERP ( r, nxy0, nxy1 );

    n1 = LERP( t, nx0, nx1 );

    return 0.87f * ( LERP( s, n0, n1 ) );
}

//---------------------------------------------------------------------
/** 4D float Perlin periodic noise.
 */

float periodic_perlin_noise4(unsigned char perm[512], float x, float y, float z, float w,
                            int px, int py, int pz, int pw)
{
    int ix0, iy0, iz0, iw0, ix1, iy1, iz1, iw1;
    float fx0, fy0, fz0, fw0, fx1, fy1, fz1, fw1;
    float s, t, r, q;
    float nxyz0, nxyz1, nxy0, nxy1, nx0, nx1, n0, n1;

    ix0 = FASTFLOOR( x ); // Integer part of x
    iy0 = FASTFLOOR( y ); // Integer part of y
    iz0 = FASTFLOOR( z ); // Integer part of y
    iw0 = FASTFLOOR( w ); // Integer part of w
    fx0 = x - ix0;        // Fractional part of x
    fy0 = y - iy0;        // Fractional part of y
    fz0 = z - iz0;        // Fractional part of z
    fw0 = w - iw0;        // Fractional part of w
    fx1 = fx0 - 1.0f;
    fy1 = fy0 - 1.0f;
    fz1 = fz0 - 1.0f;
    fw1 = fw0 - 1.0f;
    ix1 = (( ix0 + 1 ) % px ) & 0xff;  // Wrap to 0..px-1 and wrap to 0..255
    iy1 = (( iy0 + 1 ) % py ) & 0xff;  // Wrap to 0..py-1 and wrap to 0..255
    iz1 = (( iz0 + 1 ) % pz ) & 0xff;  // Wrap to 0..pz-1 and wrap to 0..255
    iw1 = (( iw0 + 1 ) % pw ) & 0xff;  // Wrap to 0..pw-1 and wrap to 0..255
    ix0 = ( ix0 % px ) & 0xff;
    iy0 = ( iy0 % py ) & 0xff;
    iz0 = ( iz0 % pz ) & 0xff;
    iw0 = ( iw0 % pw ) & 0xff;

    q = FADE( fw0 );
    r = FADE( fz0 );
    t = FADE( fy0 );
    s = FADE( fx0 );

    nxyz0 = grad4(perm[ix0 + perm[iy0 + perm[iz0 + perm[iw0]]]], fx0, fy0, fz0, fw0);
    nxyz1 = grad4(perm[ix0 + perm[iy0 + perm[iz0 + perm[iw1]]]], fx0, fy0, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad4(perm[ix0 + perm[iy0 + perm[iz1 + perm[iw0]]]], fx0, fy0, fz1, fw0);
    nxyz1 = grad4(perm[ix0 + perm[iy0 + perm[iz1 + perm[iw1]]]], fx0, fy0, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );
        
    nx0 = LERP ( r, nxy0, nxy1 );

    nxyz0 = grad4(perm[ix0 + perm[iy1 + perm[iz0 + perm[iw0]]]], fx0, fy1, fz0, fw0);
    nxyz1 = grad4(perm[ix0 + perm[iy1 + perm[iz0 + perm[iw1]]]], fx0, fy1, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad4(perm[ix0 + perm[iy1 + perm[iz1 + perm[iw0]]]], fx0, fy1, fz1, fw0);
    nxyz1 = grad4(perm[ix0 + perm[iy1 + perm[iz1 + perm[iw1]]]], fx0, fy1, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );

    nx1 = LERP ( r, nxy0, nxy1 );

    n0 = LERP( t, nx0, nx1 );

    nxyz0 = grad4(perm[ix1 + perm[iy0 + perm[iz0 + perm[iw0]]]], fx1, fy0, fz0, fw0);
    nxyz1 = grad4(perm[ix1 + perm[iy0 + perm[iz0 + perm[iw1]]]], fx1, fy0, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad4(perm[ix1 + perm[iy0 + perm[iz1 + perm[iw0]]]], fx1, fy0, fz1, fw0);
    nxyz1 = grad4(perm[ix1 + perm[iy0 + perm[iz1 + perm[iw1]]]], fx1, fy0, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );

    nx0 = LERP ( r, nxy0, nxy1 );

    nxyz0 = grad4(perm[ix1 + perm[iy1 + perm[iz0 + perm[iw0]]]], fx1, fy1, fz0, fw0);
    nxyz1 = grad4(perm[ix1 + perm[iy1 + perm[iz0 + perm[iw1]]]], fx1, fy1, fz0, fw1);
    nxy0 = LERP( q, nxyz0, nxyz1 );
        
    nxyz0 = grad4(perm[ix1 + perm[iy1 + perm[iz1 + perm[iw0]]]], fx1, fy1, fz1, fw0);
    nxyz1 = grad4(perm[ix1 + perm[iy1 + perm[iz1 + perm[iw1]]]], fx1, fy1, fz1, fw1);
    nxy1 = LERP( q, nxyz0, nxyz1 );

    nx1 = LERP ( r, nxy0, nxy1 );

    n1 = LERP( t, nx0, nx1 );

    return 0.87f * ( LERP( s, n0, n1 ) );
}

//---------------------------------------------------------------------
