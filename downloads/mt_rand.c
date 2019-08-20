#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <math.h>

#define NTHREADS 8

typedef struct seg seg;
struct seg {
    int threadNum;
    double start, end;
};

/**
   The code below is mostly stripped from the PHP sources.
 **/

#define MAX_SEED 0xffffffff


// Mersenne Twister macros and parameters
#define hiBit(u)      ((u) & 0x80000000U)  /* mask all but highest   bit of u */
#define loBit(u)      ((u) & 0x00000001U)  /* mask all but lowest    bit of u */
#define loBits(u)     ((u) & 0x7FFFFFFFU)  /* mask     the highest   bit of u */
#define mixBits(u, v) (hiBit(u)|loBits(v)) /* move hi bit of u to hi bit of v */


#define N             (624)                /* length of state vector */
#define M             (397)                /* a period parameter */


#define RAND_RANGE(__n, __min, __max, __tmax)				\
  (__n) = (__min) + (long) ((double) ( (double) (__max) - (__min) + 1.0) * ((__n) / ((__tmax) + 1.0)))

#define PHP_MT_RAND_MAX ((long) (0x7FFFFFFF)) /* (1<<31) - 1 */


typedef struct {
  uint32_t state[N];
  uint32_t left;
  uint32_t *next;
} MTState;

// Will define if the PHP generator will be used, change at compile time
unsigned short int PHPMtRand = 0;


static inline uint32_t
php_twist(uint32_t m, uint32_t u, uint32_t v)
{
  return (m ^ (mixBits(u,v)>>1) ^ ((uint32_t)(-(uint32_t)(loBit(u))) & 0x9908b0dfU));
}

static inline uint32_t
mt_twist(uint32_t m, uint32_t u, uint32_t v)
{
  return (m ^ (mixBits(u,v)>>1) ^ ((uint32_t)(-(uint32_t)(loBit(v))) & 0x9908b0dfU));
}



void
mtInitialize(uint32_t seed, MTState *mtInfo)
{
	/* Initialize generator state with seed
	   See Knuth TAOCP Vol 2, 3rd Ed, p.106 for multiplier.
	   In previous versions, most significant bits (MSBs) of the seed affect
	   only MSBs of the state array.  Modified 9 Jan 2002 by Makoto Matsumoto. */

	register uint32_t *s = mtInfo->state;
	register uint32_t *r = mtInfo->state;
	register int i = 1;

	*s++ = seed & 0xffffffffU;
	for( ; i < N; ++i ) {
	  *s++ = ( 1812433253U * ( *r ^ (*r >> 30) ) + i ) & 0xffffffffU;
	  r++;
	}
}


void
mtReload(MTState *mtInfo)
{
	/* Generate N new values in state
	   Made clearer and faster by Matthew Bellew (matthew.bellew@home.com) */

	register uint32_t *p = mtInfo->state;
	register int i;
	register uint32_t (*twist)(uint32_t, uint32_t, uint32_t) = mt_twist;
	  //(PHPMtRand)? php_twist : mt_twist;


	for (i = N - M; i--; ++p)
	  *p = twist(p[M], p[0], p[1]);
	for (i = M; --i; ++p)
	  *p = twist(p[M-N], p[0], p[1]);
	*p = twist(p[M-N], p[0], mtInfo->state[0]);
	mtInfo->left = N;
	mtInfo->next = mtInfo->state;
}


void
mt_srand(uint32_t seed, MTState *mtInfo) {
  mtInitialize(seed, mtInfo);
  mtInfo->left = 0;

  return;
}


uint32_t
mt_rand(MTState *mtInfo)
{
  /* Pull a 32-bit integer from the generator state
     Every other access function simply transforms the numbers extracted here */

  register uint32_t s1;

  if (mtInfo->left == 0)
    mtReload(mtInfo);

  -- (mtInfo->left);
  s1 = *mtInfo->next ++;

  s1 ^= (s1 >> 11);
  s1 ^= (s1 <<  7) & 0x9d2c5680U;
  s1 ^= (s1 << 15) & 0xefc60000U;
  s1 ^= (s1 >> 18) ;
  return s1;
}


uint32_t
php_mt_rand(MTState *mtInfo)
{
  return mt_rand(mtInfo);
}

uint32_t
php_mt_rand_range(MTState *mtInfo, uint32_t min, uint32_t max)
{
  uint32_t num = php_mt_rand(mtInfo);
  return num%(max-min+1)+min;
}

int compare(MTState *mtInfo, uint32_t value, uint32_t min, uint32_t max){
  if(php_mt_rand_range(mtInfo, min, max) == value) return 1;
  else return 0;
}

int crack(uint32_t seed, uint32_t *value, uint32_t length, uint32_t min, uint32_t max){
  MTState info;
  mt_srand(seed, &info);
  int i;

  for(i = 0; i < length; i++){
    if(!compare(&info, value[i], min, max)) return 0;
  }

  printf("Found seed: %u\n", seed);
  return 1;
}


uint32_t value[] = {19,13,29,20,26,18,26,29};
uint32_t length = (sizeof(value) / sizeof(value[0]));
uint32_t min = 0;
uint32_t max = 35;

void *thread_function(seg * range){
  uint32_t i;
  for(i = range->start; i < range->end; i++){
    crack(i, value, length, min, max);
  }
}

int
main(int argc, char *argv[1])
{
  pthread_t thread_id[NTHREADS];
  seg       range[NTHREADS];
  uint32_t    perThread = floor(MAX_SEED/NTHREADS);
  int i, j;

  for(i=0; i < NTHREADS; i++)
  {
     range[i].threadNum = i;
     range[i].start = perThread * i;
     if (i == NTHREADS-1) {
          range[i].end = MAX_SEED;
      } else {
          range[i].end = range[i].start + perThread;
      }
     pthread_create( &thread_id[i], NULL, (void *)thread_function, &range[i] );
  }

  for(j=0; j < NTHREADS; j++)
  {
     pthread_join( thread_id[j], NULL); 
  }
  return 0;
}
