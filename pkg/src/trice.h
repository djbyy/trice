/*! \file trice.h
\author thomas.hoehenleitner [at] seerose.net
*******************************************************************************/

#ifndef TRICE_H_
#define TRICE_H_

#ifdef TRICE_OFF // do not generate trice code for files defining TRICE_OFF before including "trice.h"
#define TRICE_CYCLE_COUNTER 0 // why needed here?
#define TRICE_INTO
#define TRICE_PUT32(n) do{ ((void)(n)); }while(0)
#define PUT_BUFFER(b,l) do{ ((void)(b)); ((void)(l)); }while(0)
#define TRICE_LEAVE
#define TRICE_S( id, p, s )  do{ ((void)(id)); ((void)(p)); ((void)(s)); }while(0)
#define TRICE_N( id, p, s, n )  do{ ((void)(id)); ((void)(p)); ((void)(s)); ((void)(n)); }while(0)
#endif

#include "triceConfig.h"
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
// Declarations and Defaults

size_t TriceDepthMax( void );
extern uint16_t* TriceBufferWritePosition;
size_t TriceCOBSEncode( void * restrict output, const void * restrict input, size_t length);
//unsigned TCOBSEncode( uint8_t* restrict output, const uint8_t * restrict input, unsigned length);
void TriceOut( uint16_t* tb, size_t tLen );
void TriceTransfer( void );
void TriceCheckSet( int index ); //!< tests

#if defined( TRICE_UART ) && !defined( TRICE_HALF_BUFFER_SIZE ) // direct out to UART
#define TRICE_WRITE( buf, len ) do{ TriceBlockingWrite( buf, len ); }while(0)
#endif

#ifdef TRICE_RTT_CHANNEL
#include "SEGGER_RTT.h"
#if defined(TRICE_HALF_BUFFER_SIZE) && TRICE_HALF_BUFFER_SIZE > BUFFER_SIZE_UP
#error
#endif
#if defined(TRICE_STACK_BUFFER_SIZE) && TRICE_STACK_BUFFER_SIZE > BUFFER_SIZE_UP
#error
#endif
//#define TRICE_WRITE( buf, len ) do{ SEGGER_RTT_Write(TRICE_RTT_CHANNEL, buf, len ); }while(0)
static inline int TriceOutDepth( void ){ return 0; }
#endif // #ifdef TRICE_RTT_CHANNEL
/*
//! The TRICE_PUT_PREFIX macro adds optionally target timestamp and location in front of each trice
#if !defined(TRICE_LOCATION) && !defined(TRICE_TIMESTAMP)
#define TRICE_COBS_PACKAGE_MODE 0
#define TRICE_PUT_PREFIX
#define TRICE_PREFIX_SIZE 0
#endif
#if !defined(TRICE_LOCATION) &&  defined(TRICE_TIMESTAMP)
#define TRICE_COBS_PACKAGE_MODE 1
#define TRICE_PUT_PREFIX TRICE_PUT32(TRICE_TIMESTAMP);
#define TRICE_PREFIX_SIZE 4
#endif
#if  defined(TRICE_LOCATION) && !defined(TRICE_TIMESTAMP)
#define TRICE_COBS_PACKAGE_MODE 2
#define TRICE_PUT_PREFIX TRICE_PUT32(TRICE_LOCATION); 
#define TRICE_PREFIX_SIZE 4
#endif
#if  defined(TRICE_LOCATION) &&  defined(TRICE_TIMESTAMP)
#define TRICE_COBS_PACKAGE_MODE 3
#define TRICE_PUT_PREFIX TRICE_PUT32(TRICE_LOCATION); TRICE_PUT32(TRICE_TIMESTAMP); 
#define TRICE_PREFIX_SIZE 8
#endif
*/
#ifndef TRICE_CYCLE_COUNTER
#define TRICE_CYCLE_COUNTER 1 //! TRICE_CYCLE_COUNTER adds a cycle counter to each trice message. The TRICE macros are a bit slower. Lost TRICEs are detectable by the trice tool.
#endif

//! TRICE_DATA_OFFSET is the space in front of trice data for in-buffer COBS encoding. It must be be a multiple of uint32_t.
#if defined(TRICE_HALF_BUFFER_SIZE)
#define TRICE_DATA_OFFSET ((9+(TRICE_HALF_BUFFER_SIZE/256))&~3) // 9: COBS_DESCRIPTOR size plus start byte plus up to 4 0-delimiters
#else
#define TRICE_DATA_OFFSET 16 // usually 8 is enough: 4 for COBS_DESCRIPTOR and additional bytes for COBS encoding, but the buffer can get big.
#endif

#if defined(TRICE_STACK_BUFFER_MAX_SIZE) && !defined(TRICE_SINGLE_MAX_SIZE)
#define TRICE_SINGLE_MAX_SIZE (TRICE_STACK_BUFFER_MAX_SIZE - TRICE_DATA_OFFSET)
#endif

#ifndef TRICE_SINGLE_MAX_SIZE
#define TRICE_SINGLE_MAX_SIZE 1008 //!< TRICE_SINGLE_MAX_SIZE ist the head size plus string length size plus max dynamic string size. Must be a multiple of 4. 1008 is the max allowed value.
#endif

#if TRICE_SINGLE_MAX_SIZE > 1008
#error
#endif

#if defined(TRICE_STACK_BUFFER_MAX_SIZE) && defined(TRICE_SINGLE_MAX_SIZE) && TRICE_SINGLE_MAX_SIZE + TRICE_DATA_OFFSET > TRICE_STACK_BUFFER_MAX_SIZE
#error
#endif

#if defined(TRICE_HALF_BUFFER_SIZE) && TRICE_HALF_BUFFER_SIZE < TRICE_SINGLE_MAX_SIZE + TRICE_DATA_OFFSET
#error
#endif

#ifndef TRICE_TRANSFER_INTERVAL_MS
//! TRICE_TRANSFER_INTERVAL_MS is the milliseconds interval for TRICE buffer read out.
//! This time should be shorter than visible delays. The TRICE_HALF_BUFFER_SIZE must be able to hold all trice messages possibly occouring in this time.
#define TRICE_TRANSFER_INTERVAL_MS 100
#endif

#if TRICE_CYCLE_COUNTER == 1
extern uint8_t TriceCycle;
#define TRICE_CYCLE TriceCycle++ //! TRICE_CYCLE is the trice cycle counter as 8 bit count 0-255.
#else
#define TRICE_CYCLE 0xC0 //! TRICE_CYCLE is no trice cycle counter, just a static value.
#endif

//
///////////////////////////////////////////////////////////////////////////////

#ifndef TRICE_PUT16
#define TRICE_PUT16(x) do{ *TriceBufferWritePosition++ = x; }while(0) //! PUT copies a 16 bit x into the TRICE buffer.
#endif

#ifdef TRICE_BIG_ENDIANNESS
#define TRICE_PUT32(x) TRICE_PUT16( (uint16_t)((uint32_t)(x)>>16) ); TRICE_PUT16( (uint16_t)(x) ); 
#define TRICE_PUT64(x) TRICE_PUT32( (uint32_t)((uint64_t)(x)>>32) ); TRICE_PUT32( (uint32_t)(x) ); 
#else
#define TRICE_PUT32(x) TRICE_PUT16( (uint16_t)(x) ); TRICE_PUT16( (uint16_t)((uint32_t)(x)>>16) );
#define TRICE_PUT64(x) TRICE_PUT32( (uint32_t)(x) ); TRICE_PUT32( (uint32_t)((uint64_t)(x)>>32) );
#endif

///////////////////////////////////////////////////////////////////////////////
// trice time measurement (STM32 only?)
//
#if defined( __arm__ )    /* Defined by GNU C and RealView */ \
 || defined( __thumb__ )  /* Defined by GNU C and RealView in Thumb mode */ \
 || defined( _ARM )       /* Defined by ImageCraft C */ \
 || defined( _M_ARM )     /* Defined by Visual Studio */ \
 || defined( _M_ARMT )    /* Defined by Visual Studio in Thumb mode */ \
 || defined( __arm )      /* Defined by Diab */ \
 || defined( __ICCARM__ ) /* IAR */ \
 || defined( __CC_ARM )   /* ARM's (RealView) compiler */ \
 || defined( __ARM__ )    /* TASKING VX ARM toolset C compiler */ \
 || defined( __CARM__ )   /* TASKING VX ARM toolset C compiler */ \
 || defined( __CPARM__ )  /* TASKING VX ARM toolset C++ compiler */
#define SYSTICKVAL (*(volatile uint32_t*)0xE000E018UL)
#else
//#error "unknown architecture"
#define SYSTICKVAL 0
#endif

//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// UART interface
//

#if defined( TRICE_UART ) && !defined( TRICE_HALF_BUFFER_SIZE ) // direct out to UART
void TriceBlockingWrite( uint8_t const * buf, unsigned len );
#endif

#if defined( TRICE_UART ) && defined( TRICE_HALF_BUFFER_SIZE ) // buffered out to UART
uint8_t TriceNextUint8( void );
void triceServeTransmit(void);
void triceTriggerTransmit(void);
int TriceOutDepth( void );
uint8_t TriceNextUint8( void );
#endif // #if defined( TRICE_UART ) && TRICE_MODE != 0

//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Encryption
//
#ifdef TRICE_ENCRYPT

void TriceEncrypt( uint32_t* p, unsigned count );
void TriceDecrypt( uint32_t* p, unsigned count );

//! little endian! change byte order for big endian machines
#define XTEA_KEY(b00, b01, b02, b03, \
                  b10, b11, b12, b13, \
                  b20, b21, b22, b23, \
                  b30, b31, b32, b33) { \
    0x##b00##b01##b02##b03, \
    0x##b10##b11##b12##b13, \
    0x##b20##b21##b22##b23, \
    0x##b30##b31##b32##b33 }

void TriceInitXteaTable(void);
    
#endif // #ifdef TRICE_ENCRYPT
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Variadic macros (https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms)
// This is extendable until a 1000 bytes payload.
#define TRICE8_COUNT(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12, NAME,...) NAME
#define TRICE8(id,frmt, ...) TRICE8_COUNT(__VA_ARGS__,TRICE8_12,TRICE8_11,TRICE8_10,TRICE8_9,TRICE8_8,TRICE8_7,TRICE8_6,TRICE8_5,TRICE8_4,TRICE8_3,TRICE8_2,TRICE8_1)(id,frmt, __VA_ARGS__)

#define TRICE16_COUNT(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12, NAME,...) NAME
#define TRICE16(id,frmt, ...) TRICE16_COUNT(__VA_ARGS__,TRICE16_12,TRICE16_11,TRICE16_10,TRICE16_9,TRICE16_8,TRICE16_7,TRICE16_6,TRICE16_5,TRICE16_4,TRICE16_3,TRICE16_2,TRICE16_1)(id,frmt, __VA_ARGS__)

#define TRICE32_COUNT(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12, NAME,...) NAME
#define TRICE32(id,frmt, ...) TRICE32_COUNT(__VA_ARGS__,TRICE32_12,TRICE32_11,TRICE32_10,TRICE32_9,TRICE32_8,TRICE32_7,TRICE32_6,TRICE32_5,TRICE32_4,TRICE32_3,TRICE32_2,TRICE32_1)(id,frmt, __VA_ARGS__)

#define TRICE64_COUNT(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12, NAME,...) NAME
#define TRICE64(id,frmt, ...) TRICE64_COUNT(__VA_ARGS__,TRICE64_12,TRICE64_11,TRICE64_10,TRICE64_9,TRICE64_8,TRICE64_7,TRICE64_6,TRICE64_5,TRICE64_4,TRICE64_3,TRICE64_2,TRICE64_1)(id,frmt, __VA_ARGS__)

// See for more explanation https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/preprocessor/macros/__VA_ARGS__/count-arguments

//! NTH_ARGUMENT just evaluates to the 14th argument. It is extendable.
#define NTH_ARGUMENT(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, ...) a14 

//! COUNT_ARGUMENTS builds upon NTH_ARGUMENT. The more arguments that are passed to COUNT_ARGUMENTS, 
//! the more the »counting arguments« (12, 11, 10, 9, 8, 7…) are pushed to the right. 
//! Thus the macro evaluates to the number of arguments that are passed to the macro.
//! If you set the C language to strict C (C90, C99, C11 or C17) the `##` operator doesn't remove the comma before it when `__VA_ARGS__` expands to nothing.
//! In this case, the TRICE macro doesn't work with no parameters. You must explicitly use TRICE0 instead of TRICE for a no parameter value TRICE in this case.
//! For more details see closed Issue #279. Special thanks @escherstair.
#define COUNT_ARGUMENTS(...) NTH_ARGUMENT(dummy, ## __VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

//! CONCAT concatenates the 2 arguments a and b (helper macro).
#define CONCAT(a, b) a ## b 

//! CONCAT2 concatenates the 2 arguments a and b (helper macro).
#define CONCAT2(a, b) CONCAT(a, b)

//! TRICE_VARIABLE_ARGUMENTS concatenates DEBUG_ with the result of COUNT_ARGUMENTS to produce something like DEBUG_2 which takes a printf-format and two arguments.
#define TRICE(id, fmt, ...) CONCAT2(TRICE_, COUNT_ARGUMENTS(__VA_ARGS__))(id, fmt, ##__VA_ARGS__)

//
///////////////////////////////////////////////////////////////////////////////

/* pre C99
// aFloat returns passed float value x as bit pattern in a uint32_t type.
static inline uint32_t aFloat( float x ){
    union {
        float f;
        uint32_t u;
    } t;
    t.f = x;
    return t.u;
}
*/

// aFloat returns passed float value x as bit pattern in a uint32_t type.
static inline uint32_t aFloat( float f ){
    union {
        float    from;
        uint32_t to;
    } pun = { .from = f };
    return pun.to;
}

// asFloat returns passed uint32_t value x bit pattern as float type.
static inline float asFloat( uint32_t x ){
    union {
        uint32_t from;
        float    to;
    } pun = { .from = x };
    return pun.to;
}

// aDouble returns passed double value x as bit pattern in a uint64_t type.
static inline uint64_t aDouble( double x ){
    union {
        double d;
        uint64_t u;
    } t;
    t.d = x;
    return t.u;
}

///////////////////////////////////////////////////////////////////////////////
// TRICE macros
//

#define TRICE_0  TRICE0  //!< Only the format string without parameter values.

#ifdef TRICE_INTO
#error
#endif

#define ID(n) do{ uint32_t ts = 0x11111111; TRICE_PUT16(0xC000 | (n)); TRICE_PUT32(ts); } while(0)
#define Id(n) do{ uint16_t ts =     0x1111; TRICE_PUT16(0x8000 | (n)); TRICE_PUT16(ts); } while(0)
#define id(n) do{                           TRICE_PUT16(0x4000 | (n));                  } while(0)
#define  N(n) do{ TRICE_PUT16( ((n)<<8) | TRICE_CYCLE ); }while(0)
#define NC(n) do{ TRICE_PUT16( (0x8000 | (n)) ); TRICE_CYCLE }while(0) // increment TRICE_CYCLE but do not transmit it

#ifndef TRICE_N
//! TRICE_N writes id and buffer of size len.
//! \param id trice identifier
//! \param pFmt formatstring for trice (ignored here but used by the trice tool), could contain any add on information. The trice tool "sees" the "TRICE_N" and can handle that.
//! \param buf runtime generated buffer
//! \param n valid data size in buf
//
// todo: for some reason this macro is not working well wit name len instead of len_, probably when injected len as value.
//
#define TRICE_N( id, pFmt, buf, n) do { \
    uint32_t limit = TRICE_SINGLE_MAX_SIZE-TRICE_PREFIX_SIZE-8; /* 8 = head + len size */ \
    uint32_t len = n; /* n could be a constant */ \
    if( len > limit ){ \
        TRICE32( Id(61732), "wrn:Transmit buffer truncated from %u to %u\n", len, limit ); \
        len = limit; \
    } \
    TRICE_ENTER; id; \
    if( len < 128 ){ N(len); }else{ NC(len); } } \
    memcpy( TriceBufferWritePosition, buf, len ); \
    TriceBufferWritePosition += (len+1)>>1; \
    TRICE_LEAVE \
} while(0)
#endif // #ifndef TRICE_N

#ifndef TRICE_S
//! TRICE_S writes id and dynString.
//! \param id trice identifier
//! \param pFmt formatstring for trice (ignored here but used by the trice tool)
//! \param dynString 0-terminated runtime generated string
#define TRICE_S( id, pFmt, dynString) do { \
    uint32_t ssiz = strlen( dynString ); \
    TRICE_N( id, pFmt, dynString, ssiz ); \
} while(0)
#endif // #ifndef TRICE_S

//! TRICE0 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
#define TRICE0( id, pFmt ) \
    TRICE_ENTER; id; N(0); \
    TRICE_LEAVE

#define TRICE_BYTEL(v)((uint8_t)(v))
#define TRICE_BYTEH(v)((uint16_t)(v)<< 8)

//! TRICE8_1 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 a 8 bit bit value
#define TRICE8_1( id, pFmt, v0 ) \
    TRICE_ENTER; id; N(1); \
    TRICE_PUT16( TRICE_BYTEL(v0)); \
    TRICE_LEAVE

//! TRICE8_2 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v1 are 8 bit bit values
#define TRICE8_2( id, pFmt, v0, v1 ) \
    TRICE_ENTER; id; N(2); \
    TRICE_PUT16(TRICE_BYTEL(v0) | TRICE_BYTEH(v1)); \
    TRICE_LEAVE

//! TRICE8_3 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v2 are 8 bit bit values
#define TRICE8_3( id, pFmt, v0, v1, v2 ) \
    TRICE_ENTER; id; N(3); \
    TRICE_PUT16(TRICE_BYTEL(v0) | TRICE_BYTEH(v1)); \
    TRICE_PUT16(TRICE_BYTEL(v3) \
    TRICE_LEAVE

//! TRICE8_4 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v3 are 8 bit bit values
#define TRICE8_4( id, pFmt, v0, v1, v2, v3 ) \
    TRICE_ENTER; id; N(4); \
    TRICE_PUT16(TRICE_BYTEL(v0) | TRICE_BYTEH(v1)); \
    TRICE_PUT16(TRICE_BYTEL(v2) | TRICE_BYTEH(v3)); \
    TRICE_LEAVE

//! TRICE8_5 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v4 are 8 bit bit values
#define TRICE8_5( id, pFmt, v0, v1, v2, v3, v4 ) \
    TRICE_ENTER; id; N(5); \
    TRICE_PUT16(TRICE_BYTEL(v0) | TRICE_BYTEH(v1)); \
    TRICE_PUT16(TRICE_BYTEL(v2) | TRICE_BYTEH(v3)); \
    TRICE_PUT16(TRICE_BYTEL(v4) \
    TRICE_LEAVE

//! TRICE8_6 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v5 are 8 bit bit values
#define TRICE8_6( id, pFmt, v0, v1, v2, v3, v4, v5 ) \
    TRICE_ENTER; id; N(6); \
    TRICE_PUT16(TRICE_BYTEL(v0) | TRICE_BYTEH(v1)); \
    TRICE_PUT16(TRICE_BYTEL(v2) | TRICE_BYTEH(v3)); \
    TRICE_PUT16(TRICE_BYTEL(v4) | TRICE_BYTEH(v5)); \
    TRICE_LEAVE

//! TRICE8_8 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v6 are 8 bit bit values
#define TRICE8_7( id, pFmt, v0, v1, v2, v3, v4, v5, v6 ) \
    TRICE_ENTER; id; N(7); \
    TRICE_PUT16(TRICE_BYTEL(v0) | TRICE_BYTEH(v1)); \
    TRICE_PUT16(TRICE_BYTEL(v2) | TRICE_BYTEH(v3)); \
    TRICE_PUT16(TRICE_BYTEL(v4) | TRICE_BYTEH(v5)); \
    TRICE_PUT16(TRICE_BYTEL(v6) \
    TRICE_LEAVE

//! TRICE8_8 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v7 are 8 bit bit values
#define TRICE8_8( id, pFmt, v0, v1, v2, v3, v4, v5, v6, v7 ) \
    TRICE_ENTER; id; N(8); \
    TRICE_PUT16(TRICE_BYTEL(v0) | TRICE_BYTEH(v1)); \
    TRICE_PUT16(TRICE_BYTEL(v2) | TRICE_BYTEH(v3)); \
    TRICE_PUT16(TRICE_BYTEL(v4) | TRICE_BYTEH(v5)); \
    TRICE_PUT16(TRICE_BYTEL(v6) | TRICE_BYTEH(v7)); \
    TRICE_LEAVE

//! TRICE8_8 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v7 are 8 bit bit values
#define TRICE8_9( id, pFmt, v0, v1, v2, v3, v4, v5, v6, v7, v8 ) \
    TRICE_ENTER; id; N(9); \
    TRICE_PUT16(TRICE_BYTEL(v0) | TRICE_BYTEH(v1)); \
    TRICE_PUT16(TRICE_BYTEL(v2) | TRICE_BYTEH(v3)); \
    TRICE_PUT16(TRICE_BYTEL(v4) | TRICE_BYTEH(v5)); \
    TRICE_PUT16(TRICE_BYTEL(v6) | TRICE_BYTEH(v7)); \
    TRICE_PUT16(TRICE_BYTEL(v8) \
    TRICE_LEAVE

//! TRICE8_8 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v7 are 8 bit bit values
#define TRICE8_10( id, pFmt, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9 ) \
    TRICE_ENTER; id; N(10); \
    TRICE_PUT16(TRICE_BYTEL(v0) | TRICE_BYTEH(v1)); \
    TRICE_PUT16(TRICE_BYTEL(v2) | TRICE_BYTEH(v3)); \
    TRICE_PUT16(TRICE_BYTEL(v4) | TRICE_BYTEH(v5)); \
    TRICE_PUT16(TRICE_BYTEL(v6) | TRICE_BYTEH(v7)); \
    TRICE_PUT16(TRICE_BYTEL(v8) | TRICE_BYTEH(v9)); \
    TRICE_LEAVE

//! TRICE8_8 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v7 are 8 bit bit values
#define TRICE8_11( id, pFmt, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10 ) \
   TRICE_ENTER; id; N(11); \
    TRICE_PUT16(TRICE_BYTEL(v0) | TRICE_BYTEH(v1)); \
    TRICE_PUT16(TRICE_BYTEL(v2) | TRICE_BYTEH(v3)); \
    TRICE_PUT16(TRICE_BYTEL(v4) | TRICE_BYTEH(v5)); \
    TRICE_PUT16(TRICE_BYTEL(v6) | TRICE_BYTEH(v7)); \
    TRICE_PUT16(TRICE_BYTEL(v8) | TRICE_BYTEH(v9)); \
    TRICE_PUT16(TRICE_BYTEL(v10) \
    TRICE_LEAVE

//! TRICE8_12 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v11 are 8 bit bit values
#define TRICE8_12( id, pFmt, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11 ) \
    TRICE_ENTER; id; N(12); \
    TRICE_PUT16(TRICE_BYTEL(v0) | TRICE_BYTEH(v1)); \
    TRICE_PUT16(TRICE_BYTEL(v2) | TRICE_BYTEH(v3)); \
    TRICE_PUT16(TRICE_BYTEL(v4) | TRICE_BYTEH(v5)); \
    TRICE_PUT16(TRICE_BYTEL(v6) | TRICE_BYTEH(v7)); \
    TRICE_PUT16(TRICE_BYTEL(v8) | TRICE_BYTEH(v9)); \
    TRICE_PUT16(TRICE_BYTEL(v10) | TRICE_BYTEH(v11)); \
    TRICE_LEAVE

//! TRICE16_1 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 a 16 bit value
#define TRICE16_1( id, pFmt, v0 ) \
    TRICE_ENTER; id; N(2); \
    TRICE_PUT16( (uint16_t)(v0) ); \
    TRICE_LEAVE

//! TRICE16_2 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v1 are 16 bit values
#define TRICE16_2( id, pFmt, v0, v1 ) \
    TRICE_ENTER; id; N(4); \
    TRICE_PUT16( (uint16_t)(v0) ); \
    TRICE_PUT16( (uint16_t)(v1) ); \
    TRICE_LEAVE

//! TRICE16_3 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v2 are 16 bit values
#define TRICE16_3( id, pFmt, v0, v1, v2 ) \
    TRICE_ENTER; id; N(6); \
    TRICE_PUT16( (uint16_t)(v0) ); \
    TRICE_PUT16( (uint16_t)(v1) ); \
    TRICE_PUT16( (uint16_t)(v2) ); \
    TRICE_LEAVE

//! TRICE16_4 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v3 are 16 bit values
#define TRICE16_4( id, pFmt, v0, v1, v2, v3 ) \
    TRICE_ENTER; id; N(8); \
    TRICE_PUT16( (uint16_t)(v0) ); \
    TRICE_PUT16( (uint16_t)(v1) ); \
    TRICE_PUT16( (uint16_t)(v2) ); \
    TRICE_PUT16( (uint16_t)(v3) ); \
    TRICE_LEAVE

//! TRICE16_5 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v4 are 16 bit values
#define TRICE16_5( id, pFmt, v0, v1, v2, v3, v4 ) \
    TRICE_ENTER; id; N(10); \
    TRICE_PUT16( (uint16_t)(v0) ); \
    TRICE_PUT16( (uint16_t)(v1) ); \
    TRICE_PUT16( (uint16_t)(v2) ); \
    TRICE_PUT16( (uint16_t)(v3) ); \
    TRICE_PUT16( (uint16_t)(v4) ); \
    TRICE_LEAVE

//! TRICE16_6 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v5 are 16 bit values
#define TRICE16_6( id, pFmt, v0, v1, v2, v3, v4, v5 ) \
    TRICE_ENTER; id; N(12); \
    TRICE_PUT16( (uint16_t)(v0) ); \
    TRICE_PUT16( (uint16_t)(v1) ); \
    TRICE_PUT16( (uint16_t)(v2) ); \
    TRICE_PUT16( (uint16_t)(v3) ); \
    TRICE_PUT16( (uint16_t)(v4) ); \
    TRICE_PUT16( (uint16_t)(v5) ); \
    TRICE_LEAVE

//! TRICE16_7 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v6 are 16 bit values
#define TRICE16_7( id, pFmt, v0, v1, v2, v3, v4, v5, v6 ) \
    TRICE_ENTER; id; N(14); \
    TRICE_PUT16( (uint16_t)(v0) ); \
    TRICE_PUT16( (uint16_t)(v1) ); \
    TRICE_PUT16( (uint16_t)(v2) ); \
    TRICE_PUT16( (uint16_t)(v3) ); \
    TRICE_PUT16( (uint16_t)(v4) ); \
    TRICE_PUT16( (uint16_t)(v5) ); \
    TRICE_PUT16( (uint16_t)(v6) ); \
    TRICE_LEAVE

//! TRICE16_8 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v7 are 16 bit values
#define TRICE16_8( id, pFmt, v0, v1, v2, v3, v4, v5, v6, v7 ) \
    TRICE_ENTER; id; N(16); \
    TRICE_PUT16( (uint16_t)(v0) ); \
    TRICE_PUT16( (uint16_t)(v1) ); \
    TRICE_PUT16( (uint16_t)(v2) ); \
    TRICE_PUT16( (uint16_t)(v3) ); \
    TRICE_PUT16( (uint16_t)(v4) ); \
    TRICE_PUT16( (uint16_t)(v5) ); \
    TRICE_PUT16( (uint16_t)(v6) ); \
    TRICE_PUT16( (uint16_t)(v7) ); \
    TRICE_LEAVE
    
//! TRICE16_9 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v8 are 16 bit values
#define TRICE16_9( id, pFmt, v0, v1, v2, v3, v4, v5, v6, v7, v8 ) \
    TRICE_ENTER; id; N(18); \
    TRICE_PUT16( (uint16_t)(v0) ); \
    TRICE_PUT16( (uint16_t)(v1) ); \
    TRICE_PUT16( (uint16_t)(v2) ); \
    TRICE_PUT16( (uint16_t)(v3) ); \
    TRICE_PUT16( (uint16_t)(v4) ); \
    TRICE_PUT16( (uint16_t)(v5) ); \
    TRICE_PUT16( (uint16_t)(v6) ); \
    TRICE_PUT16( (uint16_t)(v7) ); \
    TRICE_PUT16( (uint16_t)(v8) ); \
    TRICE_LEAVE

//! TRICE16_10 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v9 are 16 bit values
#define TRICE16_10( id, pFmt, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9 ) \
    TRICE_ENTER; id; N(20); \
    TRICE_PUT16( (uint16_t)(v0) ); \
    TRICE_PUT16( (uint16_t)(v1) ); \
    TRICE_PUT16( (uint16_t)(v2) ); \
    TRICE_PUT16( (uint16_t)(v3) ); \
    TRICE_PUT16( (uint16_t)(v4) ); \
    TRICE_PUT16( (uint16_t)(v5) ); \
    TRICE_PUT16( (uint16_t)(v6) ); \
    TRICE_PUT16( (uint16_t)(v7) ); \
    TRICE_PUT16( (uint16_t)(v8) ); \
    TRICE_PUT16( (uint16_t)(v9) ); \
    TRICE_LEAVE

//! TRICE16_11 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v10 are 16 bit values
#define TRICE16_11( id, pFmt, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10 ) \
    TRICE_ENTER; id; N(22); \
    TRICE_PUT16( (uint16_t)(v0) ); \
    TRICE_PUT16( (uint16_t)(v1) ); \
    TRICE_PUT16( (uint16_t)(v2) ); \
    TRICE_PUT16( (uint16_t)(v3) ); \
    TRICE_PUT16( (uint16_t)(v4) ); \
    TRICE_PUT16( (uint16_t)(v5) ); \
    TRICE_PUT16( (uint16_t)(v6) ); \
    TRICE_PUT16( (uint16_t)(v7) ); \
    TRICE_PUT16( (uint16_t)(v8) ); \
    TRICE_PUT16( (uint16_t)(v9) ); \
    TRICE_PUT16( (uint16_t)(v10) ); \
    TRICE_LEAVE
    
//! TRICE16_12 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v11 are 16 bit values
#define TRICE16_12( id, pFmt, v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11 ) \
    TRICE_ENTER; id; N(24); \
    TRICE_PUT16( (uint16_t)(v0) ); \
    TRICE_PUT16( (uint16_t)(v1) ); \
    TRICE_PUT16( (uint16_t)(v2) ); \
    TRICE_PUT16( (uint16_t)(v3) ); \
    TRICE_PUT16( (uint16_t)(v4) ); \
    TRICE_PUT16( (uint16_t)(v5) ); \
    TRICE_PUT16( (uint16_t)(v6) ); \
    TRICE_PUT16( (uint16_t)(v7) ); \
    TRICE_PUT16( (uint16_t)(v8) ); \
    TRICE_PUT16( (uint16_t)(v9) ); \
    TRICE_PUT16( (uint16_t)(v10) ); \
    TRICE_PUT16( (uint16_t)(v11) ); \
    TRICE_LEAVE


//! TRICE32_1 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 the 32 bit value
#define TRICE32_1( id, pFmt, v0 ) \
    TRICE_ENTER; id; N(4); \
    TRICE_PUT32( (uint32_t)(v0) ); \
    TRICE_LEAVE

//! TRICE32_2 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v1 are 32 bit values
#define TRICE32_2( id, pFmt, v0, v1 ) \
    TRICE_ENTER; id; N(8); \
    TRICE_PUT32( (uint32_t)(v0) ); \
    TRICE_PUT32( (uint32_t)(v1) ); \
    TRICE_LEAVE

//! TRICE32_3 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v2 are 32 bit values
#define TRICE32_3( id, pFmt, v0, v1, v2 ) \
    TRICE_ENTER; id; N(12); \
    TRICE_PUT32( (uint32_t)(v0) ); \
    TRICE_PUT32( (uint32_t)(v1) ); \
    TRICE_PUT32( (uint32_t)(v2) ); \
    TRICE_LEAVE

//! TRICE32_4 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v3 are 32 bit values
#define TRICE32_4( id, pFmt, v0, v1, v2, v3 ) \
    TRICE_ENTER; id; N(16); \
    TRICE_PUT32( (uint32_t)(v0) ); \
    TRICE_PUT32( (uint32_t)(v1) ); \
    TRICE_PUT32( (uint32_t)(v2) ); \
    TRICE_PUT32( (uint32_t)(v3) ); \
    TRICE_LEAVE

//! TRICE32_5 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v4 are 32 bit values
#define TRICE32_5( id, pFmt,  v0, v1, v2, v3, v4 ) \
    TRICE_ENTER; id; N(20); \
    TRICE_PUT32( (uint32_t)(v0) ); \
    TRICE_PUT32( (uint32_t)(v1) ); \
    TRICE_PUT32( (uint32_t)(v2) ); \
    TRICE_PUT32( (uint32_t)(v3) ); \
    TRICE_PUT32( (uint32_t)(v4) ); \
    TRICE_LEAVE

//! TRICE32_6 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v5 are 32 bit values
#define TRICE32_6( id, pFmt,  v0, v1, v2, v3, v4, v5 ) \
    TRICE_ENTER; id; N(24); \
    TRICE_PUT32( (uint32_t)(v0) ); \
    TRICE_PUT32( (uint32_t)(v1) ); \
    TRICE_PUT32( (uint32_t)(v2) ); \
    TRICE_PUT32( (uint32_t)(v3) ); \
    TRICE_PUT32( (uint32_t)(v4) ); \
    TRICE_PUT32( (uint32_t)(v5) ); \
    TRICE_LEAVE

//! TRICE32_7 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v6 are 32 bit values
#define TRICE32_7( id, pFmt,  v0, v1, v2, v3, v4, v5, v6 ) \
    TRICE_ENTER; id; N(28); \
    TRICE_PUT32( (uint32_t)(v0) ); \
    TRICE_PUT32( (uint32_t)(v1) ); \
    TRICE_PUT32( (uint32_t)(v2) ); \
    TRICE_PUT32( (uint32_t)(v3) ); \
    TRICE_PUT32( (uint32_t)(v4) ); \
    TRICE_PUT32( (uint32_t)(v5) ); \
    TRICE_PUT32( (uint32_t)(v6) ); \
    TRICE_LEAVE

//! TRICE32_8 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v7 are 32 bit values
#define TRICE32_8( id, pFmt,  v0, v1, v2, v3, v4, v5, v6, v7 ) \
    TRICE_ENTER; id; N(32); \
    TRICE_PUT32( (uint32_t)(v0) ); \
    TRICE_PUT32( (uint32_t)(v1) ); \
    TRICE_PUT32( (uint32_t)(v2) ); \
    TRICE_PUT32( (uint32_t)(v3) ); \
    TRICE_PUT32( (uint32_t)(v4) ); \
    TRICE_PUT32( (uint32_t)(v5) ); \
    TRICE_PUT32( (uint32_t)(v6) ); \
    TRICE_PUT32( (uint32_t)(v7) ); \
    TRICE_LEAVE

//! TRICE32_9 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v8 are 32 bit values
#define TRICE32_9( id, pFmt,  v0, v1, v2, v3, v4, v5, v6, v7, v8 ) \
    TRICE_ENTER; id; N(36); \
    TRICE_PUT32( (uint32_t)(v0) ); \
    TRICE_PUT32( (uint32_t)(v1) ); \
    TRICE_PUT32( (uint32_t)(v2) ); \
    TRICE_PUT32( (uint32_t)(v3) ); \
    TRICE_PUT32( (uint32_t)(v4) ); \
    TRICE_PUT32( (uint32_t)(v5) ); \
    TRICE_PUT32( (uint32_t)(v6) ); \
    TRICE_PUT32( (uint32_t)(v7) ); \
    TRICE_PUT32( (uint32_t)(v8) ); \
    TRICE_LEAVE

//! TRICE32_10 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - 9 are 32 bit values
#define TRICE32_10( id, pFmt,  v0, v1, v2, v3, v4, v5, v6, v7, v8, v9 ) \
    TRICE_ENTER; id; N(40); \
    TRICE_PUT32( (uint32_t)(v0) ); \
    TRICE_PUT32( (uint32_t)(v1) ); \
    TRICE_PUT32( (uint32_t)(v2) ); \
    TRICE_PUT32( (uint32_t)(v3) ); \
    TRICE_PUT32( (uint32_t)(v4) ); \
    TRICE_PUT32( (uint32_t)(v5) ); \
    TRICE_PUT32( (uint32_t)(v6) ); \
    TRICE_PUT32( (uint32_t)(v7) ); \
    TRICE_PUT32( (uint32_t)(v8) ); \
    TRICE_PUT32( (uint32_t)(v9) ); \
    TRICE_LEAVE

//! TRICE32_11 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v10 are 32 bit values
#define TRICE32_11( id, pFmt,  v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10 ) \
    TRICE_ENTER; id; N(44); \
    TRICE_PUT32( (uint32_t)(v0) ); \
    TRICE_PUT32( (uint32_t)(v1) ); \
    TRICE_PUT32( (uint32_t)(v2) ); \
    TRICE_PUT32( (uint32_t)(v3) ); \
    TRICE_PUT32( (uint32_t)(v4) ); \
    TRICE_PUT32( (uint32_t)(v5) ); \
    TRICE_PUT32( (uint32_t)(v6) ); \
    TRICE_PUT32( (uint32_t)(v7) ); \
    TRICE_PUT32( (uint32_t)(v8) ); \
    TRICE_PUT32( (uint32_t)(v9) ); \
    TRICE_PUT32( (uint32_t)(v10) ); \
    TRICE_LEAVE

//! TRICE32_12 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v11 are 32 bit values
#define TRICE32_12( id, pFmt,  v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11 ) \
    TRICE_ENTER; id; N(48); \
    TRICE_PUT32( (uint32_t)(v0) ); \
    TRICE_PUT32( (uint32_t)(v1) ); \
    TRICE_PUT32( (uint32_t)(v2) ); \
    TRICE_PUT32( (uint32_t)(v3) ); \
    TRICE_PUT32( (uint32_t)(v4) ); \
    TRICE_PUT32( (uint32_t)(v5) ); \
    TRICE_PUT32( (uint32_t)(v6) ); \
    TRICE_PUT32( (uint32_t)(v7) ); \
    TRICE_PUT32( (uint32_t)(v8) ); \
    TRICE_PUT32( (uint32_t)(v9) ); \
    TRICE_PUT32( (uint32_t)(v10) ); \
    TRICE_PUT32( (uint32_t)(v11) ); \
    TRICE_LEAVE

//! TRICE64_1 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 is a 64 bit values
#define TRICE64_1( id, pFmt, v0 ) \
    TRICE_ENTER; id; N(8); \
    TRICE_PUT64( v0 ); \
    TRICE_LEAVE

//! TRICE64_2 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v1 are 64 bit values
#define TRICE64_2( id, pFmt, v0, v1 ) \
    TRICE_ENTER; id; N(16); \
    TRICE_PUT64( v0 ); \
    TRICE_PUT64( v1 ); \
    TRICE_LEAVE

//! TRICE64_3 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v2 are 64 bit values
#define TRICE64_3( id, pFmt, v0, v1, v2 ) \
    TRICE_ENTER; id; N(3*8); \
    TRICE_PUT64( v0 ); \
    TRICE_PUT64( v1 ); \
    TRICE_PUT64( v2 ); \
    TRICE_LEAVE


//! TRICE64_4 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v3 are 64 bit values
#define TRICE64_4( id, pFmt, v0, v1, v2, v3 ) \
    TRICE_ENTER; id; N(4*8); \
    TRICE_PUT64( v0 ); \
    TRICE_PUT64( v1 ); \
    TRICE_PUT64( v2 ); \
    TRICE_PUT64( v3 ); \
    TRICE_LEAVE

//! TRICE64_5 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v4 are 64 bit values
#define TRICE64_5( id, pFmt,  v0, v1, v2, v3, v4 ) \
    TRICE_ENTER; id; N(5*8); \
    TRICE_PUT64( v0 ); \
    TRICE_PUT64( v1 ); \
    TRICE_PUT64( v2 ); \
    TRICE_PUT64( v3 ); \
    TRICE_PUT64( v4 ); \
    TRICE_LEAVE

//! TRICE64_6 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v5 are 64 bit values
#define TRICE64_6( id, pFmt,  v0, v1, v2, v3, v4, v5 ) \
    TRICE_ENTER; id; N(6*8); \
    TRICE_PUT64( v0 ); \
    TRICE_PUT64( v1 ); \
    TRICE_PUT64( v2 ); \
    TRICE_PUT64( v3 ); \
    TRICE_PUT64( v4 ); \
    TRICE_PUT64( v5 ); \
    TRICE_LEAVE

//! TRICE64_7 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v6 are 64 bit values
#define TRICE64_7( id, pFmt,  v0, v1, v2, v3, v4, v5, v6 ) \
    TRICE_ENTER; id; N(7*8); \
    TRICE_PUT64( v0 ); \
    TRICE_PUT64( v1 ); \
    TRICE_PUT64( v2 ); \
    TRICE_PUT64( v3 ); \
    TRICE_PUT64( v4 ); \
    TRICE_PUT64( v5 ); \
    TRICE_PUT64( v6 ); \
    TRICE_LEAVE
    
//! TRICE64_8 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v7 are 64 bit values
#define TRICE64_8( id, pFmt,  v0, v1, v2, v3, v4, v5, v6, v7 ) \
    TRICE_ENTER; id; N(8*8); \
    TRICE_PUT64( v0 ); \
    TRICE_PUT64( v1 ); \
    TRICE_PUT64( v2 ); \
    TRICE_PUT64( v3 ); \
    TRICE_PUT64( v4 ); \
    TRICE_PUT64( v5 ); \
    TRICE_PUT64( v6 ); \
    TRICE_PUT64( v7 ); \
    TRICE_LEAVE

//! TRICE64_9 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v8 are 64 bit values
#define TRICE64_9( id, pFmt,  v0, v1, v2, v3, v4, v5, v6, v7, v8 ) \
    TRICE_ENTER; id; N(9*8); \
    TRICE_PUT64( v0 ); \
    TRICE_PUT64( v1 ); \
    TRICE_PUT64( v2 ); \
    TRICE_PUT64( v3 ); \
    TRICE_PUT64( v4 ); \
    TRICE_PUT64( v5 ); \
    TRICE_PUT64( v6 ); \
    TRICE_PUT64( v7 ); \
    TRICE_PUT64( v8 ); \
    TRICE_LEAVE

//! TRICE64_10 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v9 are 64 bit values
#define TRICE64_10( id, pFmt,  v0, v1, v2, v3, v4, v5, v6, v7, v8, v9 ) \
    TRICE_ENTER; id; N(10*8); \
    TRICE_PUT64( v0 ); \
    TRICE_PUT64( v1 ); \
    TRICE_PUT64( v2 ); \
    TRICE_PUT64( v3 ); \
    TRICE_PUT64( v4 ); \
    TRICE_PUT64( v5 ); \
    TRICE_PUT64( v6 ); \
    TRICE_PUT64( v7 ); \
    TRICE_PUT64( v8 ); \
    TRICE_PUT64( v9 ); \
    TRICE_LEAVE

//! TRICE64_11 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v10 are 64 bit values
#define TRICE64_11( id, pFmt,  v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10 ) \
    TRICE_ENTER; id; N(11*8); \
    TRICE_PUT64( v0 ); \
    TRICE_PUT64( v1 ); \
    TRICE_PUT64( v2 ); \
    TRICE_PUT64( v3 ); \
    TRICE_PUT64( v4 ); \
    TRICE_PUT64( v5 ); \
    TRICE_PUT64( v6 ); \
    TRICE_PUT64( v7 ); \
    TRICE_PUT64( v8 ); \
    TRICE_PUT64( v9 ); \
    TRICE_PUT64( v10 ); \
    TRICE_LEAVE

//! TRICE64_12 writes trice data as fast as possible in a buffer.
//! \param id is a 16 bit Trice id in upper 2 bytes of a 32 bit value
//! \param v0 - v11 are 64 bit values
#define TRICE64_12( id, pFmt,  v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11 ) \
    TRICE_ENTER; id; N(12*8); \
    TRICE_PUT64( v0 ); \
    TRICE_PUT64( v1 ); \
    TRICE_PUT64( v2 ); \
    TRICE_PUT64( v3 ); \
    TRICE_PUT64( v4 ); \
    TRICE_PUT64( v5 ); \
    TRICE_PUT64( v6 ); \
    TRICE_PUT64( v7 ); \
    TRICE_PUT64( v8 ); \
    TRICE_PUT64( v9 ); \
    TRICE_PUT64( v10 ); \
    TRICE_PUT64( v11 ); \
    TRICE_LEAVE


#ifdef __cplusplus
}
#endif

#endif // TRICE_H_
