//
//  Filename:	Test.h
//	Created:	2:6:2012   17:39

#ifndef		__DC_Test_H__
#define		__DC_Test_H__

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include <float.h>

/*
=========================================================================================

			CODE

=========================================================================================
*/

class Model_OBJ;
class IRayTracer;
struct sLight;

// ** class cTest
class cTest {
public:

	virtual			~cTest( void ) {}

	virtual void	KeyPressed( int key ) {}
	virtual void	Create( void ) {}
	virtual void	Update( void ) {}
	virtual void	Render( void ) {}
};

#endif	/*	!__DC_Test_H__	*/