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
 
extern void perlin_noise_randomize_perm(unsigned char perm[512]);

/** 1D, 2D, 3D and 4D float Perlin noise
 */
extern float perlin_noise1(unsigned char perm[512], float x);
extern float perlin_noise2(unsigned char perm[512], float x, float y);
extern float perlin_noise3(unsigned char perm[512], float x, float y, float z);
extern float perlin_noise4(unsigned char perm[512], float x, float y, float z, float w);

/** 1D, 2D, 3D and 4D float Perlin periodic noise
 */
extern float periodic_perlin_noise1(unsigned char perm[512], float x, int px);
extern float periodic_perlin_noise2(unsigned char perm[512], float x, float y, int px, int py);
extern float periodic_perlin_noise3(unsigned char perm[512], float x, float y, float z, int px, int py, int pz);
extern float periodic_perlin_noise4(unsigned char perm[512], float x, float y, float z, float w, int px, int py, int pz, int pw);
