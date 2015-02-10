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
	virtual void	Create( IRayTracer *model, const Model_OBJ& mesh ) {}
	virtual void	Update( void ) {}
	virtual void	Render( Model_OBJ& mesh, Model_OBJ& light ) {}
};

#endif	/*	!__DC_Test_H__	*/