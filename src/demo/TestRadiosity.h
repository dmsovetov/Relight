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

typedef struct												// Create A Structure
{
    unsigned char	*imageData;										// Image Data (Up To 32 Bits)
    unsigned int	bpp;											// Image Color Depth In Bits Per Pixel.
    unsigned int	width;											// Image Width
    unsigned int	height;											// Image Height
    unsigned int	texID;											// Texture ID Used To Select A Texture
} TextureImage;

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
    float                   rotation;

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
	virtual void			Render( void );
};

#endif	/*	!__TestRadiosity_H__	*/