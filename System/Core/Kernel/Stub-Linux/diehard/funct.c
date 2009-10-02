
#include "header.h"
#include "macro.h"

/*gamma(z) when 2z is a integer*/
double G(double z)
{
  int tmp=2*z;

  if( tmp!=2*z || z==0 ) printf("Error in calling G(z)!!!");

  switch(tmp){
    case 1: return sqrt(PI);
    case 2: return 1;
    default: break;
  }

  return (z-1)*G(z-1);
}

int ucmpr( const void *i1, const void *i2)
{
  uniform *u1=(uniform*)i1, *u2=(uniform*)i2;

  if( *u1<*u2 ) return -1;
  if( *u1==*u2 ) return 0;

  return 1;
}

int fcmpr(const void *f1, const void *f2)
{
  real *u1=(real*)f1, *u2=(real*)f2;

  if( *u1<*u2 ) return -1;
  if( *u1==*u2 ) return 0;

  return 1;
}

// Читаем случайные числа исключительно из CoreRandom
uniform uni(char *filename)
{
	if (strcmp(filename, "core") == 0) {
		return (uniform)CoreRandom ();
	}

	return 0;
}

/*show the bit-pattern of an integer*/
void showbit(int n)
{
  short int i, bit[32];
  int tmp=n;

  for(i=0; i<32; ++i){
    bit[i]=0;
    if(tmp<0) bit[i]=1;
    tmp*=2;
    printf("%d",bit[i]);
  }

  return;
}
