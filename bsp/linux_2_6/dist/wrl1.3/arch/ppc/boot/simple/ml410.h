//----------------------------------------------------------------------------
// libc.h
//----------------------------------------------------------------------------

typedef unsigned int	bool;

#define TRUE		((bool) 1)
#define FALSE		((bool) 0)

//----------------------------------------------------------------------------

typedef unsigned char		byte;
typedef unsigned short		ui16;
typedef unsigned long int	ui32;
typedef signed long int		si32;

typedef unsigned long		addr;

#ifndef NULL
#define NULL		0
#endif

#define PACKED 		__attribute__ ((packed))

//----------------------------------------------------------------------------

#define B0(x)	((x    ) & 0xff)
#define B1(x)	((x>> 8) & 0xff)
#define B2(x)	((x>>16) & 0xff)
#define B3(x)	((x>>24) & 0xff)

#define BX16(b1, b0)         	(((b1)<< 8)|((b0)))
#define BX32(b3, b2, b1, b0) 	(((b3)<<24)|((b2)<<16)|((b1)<<8)|((b0)))

#define BSWAP16(x) (BX16(B0(x),B1(x)))
#define BSWAP32(x) (BX32(B0(x),B1(x),B2(x),B3(x)))

//----------------------------------------------------------------------------
// host-to-big-endian-short, little-endian-to-host-long, etc...

#define ARCH_ENDIAN_BIG

#ifdef ARCH_ENDIAN_BIG

#define htobe16(x)	(x)
#define htobe32(x)	(x)
#define htole16(x)	BSWAP16(x)
#define htole32(x)	BSWAP32(x)
#define betoh16(x)	(x)
#define betoh32(x)	(x)
#define letoh16(x)	BSWAP16(x)
#define letoh32(x)	BSWAP32(x)

#else 

#define htobe16(x)	BSWAP16(x)
#define htobe32(x)	BSWAP32(x)
#define htole16(x)	(x)
#define htole32(x)	(x)
#define betoh16(x)	BSWAP16(x)
#define betoh32(x)	BSWAP32(x)
#define letoh16(x)	(x)
#define letoh32(x)	(x)

#endif

//----------------------------------------------------------------------------

#define PSTR(x) x
#define PRG_RDB(x) ((int) (*x))

#define RD08(a)		(*((volatile byte *) (a)))
#define RD16(a)		(*((volatile ui16 *) (a)))
#define RD32(a)		(*((volatile ui32 *) (a)))

#define WR08(a, d)	(*((volatile byte *) (a)) = d)
#define WR16(a, d)	(*((volatile ui16 *) (a)) = d)
#define WR32(a, d)	(*((volatile ui32 *) (a)) = d)

//----------------------------------------------------------------------------

#define MEM_RD08(a)		(*((byte *)((ui32)(a))))
#define MEM_WR08(a, d)		*((byte *)((ui32)(a))) = d

#define MEM_RD16(a)		(*((ui16 *)((ui32)(a))))
#define MEM_WR16(a, d)		*((ui16 *)((ui32)(a))) = d

#define MEM_RD32(a)		(*((ui32 *)((ui32)(a))))
#define MEM_WR32(a, d)		*((ui32 *)((ui32)(a))) = d

//----------------------------------------------------------------------------

#define streq(a, b) (strcmp(a, b) == 0)

void jj_fprintf(char *, ...);
void jj_sprintf(char *, char *, ...);

// This works in gcc3.0 only...
// #define printf(fmt, args...) jj_fprintf(PSTR(fmt), ##args)

#define printf(fmt, args...)	jj_fprintf(PSTR(fmt), args)
#define prints(fmt)		jj_fprintf(PSTR(fmt))

#define sprintf(buf, fmt, args...)	jj_sprintf(buf, PSTR(fmt), args)

//----------------------------------------------------------------------------
