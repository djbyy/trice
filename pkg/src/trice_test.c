/*! \file trice_test.c
\brief wrapper for tests
\author thomas.hoehenleitner [at] seerose.net
*******************************************************************************/
#include "trice.h"
#include "trice_test.h"

// SetTriceBuffer sets the internal triceBuffer pointer to buf. 
// This function is called from Go for test setup.
void SetTriceBuffer( uint8_t* buf  ){
    triceBuffer = buf;
}

// SetTriceFraming sets the TriceFraming to n. 
// This function is called from Go for test setup.
void SetTriceFraming( int n ){
    triceFraming = n;
}

// TriceCode performs trice code sequence n and returns the amount of into triceBuffer written bytes.
// This function is called from Go for tests.
int TriceCode( int n ){
    //char* s;
    switch( n ){
        case 0: TRICE32_1( id(0x2222), "rd:TRICE32_1 line %d (%%d)\n", -1 ); return triceBufferDepth;
        case 1: TRICE32_1( Id(0x2222), "rd:TRICE32_1 line %d (%%d)\n", -1 ); return triceBufferDepth;
        case 2: TRICE32_1( ID(0x2222), "rd:TRICE32_1 line %d (%%d)\n", -1 ); return triceBufferDepth;
        // case 1: TRICE64( Id( 52183), "rd:TRICE64 %d, %d\n", 1, 2 );         return triceBufferDepth; 
        // case 2: s = "AAAAAAAAAAAA"; TRICE_S( Id( 43140), "sig:%s\n", s );   return triceBufferDepth; 
    }
    return 0;
}
