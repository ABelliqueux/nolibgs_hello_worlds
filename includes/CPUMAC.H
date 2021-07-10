/*
** cpumac.h
   mike acton
*/

// cpu_ldr(cpu register,data pointer)
// copy 32bit data from dp to r
#define cpu_ldr(r,dp)\
asm(\
  "lw %0, 0(%1);"\
  : "=r" (r)\
  : "r"  (dp)\
)

// cpu_gted0(cpu register)
// copy 32bit data from r to gte register 0
#define cpu_gted0(r)\
asm(\
  "mtc2 %0, $0;"\
  :\
  : "r" (r)\
)

// cpu_gted1(cpu register)
// copy 32bit data from r to gte register 1
#define cpu_gted1(r)\
asm(\
  "mtc2 %0, $1;"\
  :\
  : "r" (r)\
)

// cpu_gted2(cpu register)
// copy 32bit data from r to gte register 2
#define cpu_gted2(r)\
asm(\
  "mtc2 %0, $2;"\
  :\
  : "r" (r)\
)

// cpu_gted3(cpu register)
// copy 32bit data from r to gte register 3
#define cpu_gted3(r)\
asm(\
  "mtc2 %0, $3;"\
  :\
  : "r" (r)\
)

// cpu_gted4(cpu register)
// copy 32bit data from r to gte register 4
#define cpu_gted4(r)\
asm(\
  "mtc2 %0, $4;"\
  :\
  : "r" (r)\
)

// cpu_gted5(cpu register)
// copy 32bit data from r to gte register 5
#define cpu_gted5(r)\
asm(\
  "mtc2 %0, $5;"\
  :\
  : "r" (r)\
)
