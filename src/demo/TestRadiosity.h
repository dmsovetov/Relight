//
//  Filename:	TestRadiosity.h
//	Created:	01:06:2012   17:56

#ifndef		__TestRadiosity_H__
#define		__TestRadiosity_H__

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"Test.h"
#include	"Radiosity.h"

/*
=========================================================================================

			CODE

=========================================================================================
*/

class cLightmapper;

// ** class cTestRadiosity
class cTestRadiosity : public cTest {
private:

	cLightmapper			*lightmapper;
	cLightmap				*indirect;
	cRadiositySolver		*solver;

	const sLight			*lights;
	int						totalLights;

	// ** GPU textures
	float					*indirectPixels, *directPixels;
	unsigned int			texIndirect, texDirect;
	TextureImage			diffuse;

	// ** Settings
	bool					useLinearFiltration;
	bool					usePrevResults;
	bool					renderDirect, renderIndirect, renderSample;
	bool					blurTexture, interpolate;
	bool					solve;

	// ** Debug draw
	const sRadiositySample	*nearest;

private:

	void					FindNearestSample( void );
	void					BlurLightmap( void );

public:

							cTestRadiosity( const sLight *_lights, int _totalLights )
								: nearest( NULL ), lights( _lights ), totalLights( _totalLights ) {}

	// ** cTest
	virtual void			KeyPressed( int key );
	virtual void			Create( IRayTracer *model, const Model_OBJ& mesh );
	virtual void			Update( void );
	virtual void			Render( Model_OBJ& mesh );
};

#endif	/*	!__TestRadiosity_H__	*/