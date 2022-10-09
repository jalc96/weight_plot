#include<stdint.h>
#include<limits.h>
#include<float.h>


typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

typedef uintptr_t umm;
typedef intptr_t  smm;


#define U8_MIN  0
#define U8_MAX  255
#define U16_MIN 0
#define U16_MAX 65535
#define U32_MIN 0
#define U32_MAX ((u32)-1)
#define U64_MIN 0
#define U64_MAX ((u64)-1)

#define S8_MIN  SCHAR_MIN
#define S8_MAX  SCHAR_MAX
#define S16_MIN INT_MIN
#define S16_MAX INT_MAX
#define S32_MIN LONG_MIN
#define S32_MAX LONG_MAX
#define S64_MIN LLONG_MIN
#define S64_MAX LLONG_MAX

#define F32_MAX  FLT_MAX
#define F32_MIN -FLT_MAX
#define F64_MAX  DBL_MAX
#define F64_MIN -DBL_MAX

#define internal static
#define global_variable static
#define local_persist static

