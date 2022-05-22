/*! \file trice_test.c
\brief wrapper for tests
\author thomas.hoehenleitner [at] seerose.net
*******************************************************************************/

#include <stdint.h>

void SetTriceBuffer( uint8_t* buf );
void SetTriceFraming( int n );
int TriceCode( int n );

extern int triceBufferDepth;
extern uint8_t* triceBuffer;
extern int triceFraming;
