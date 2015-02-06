//
//  Filename:	DirectLightCalculator.h
//	Created:	22:03:2012   17:06

#ifndef		__DirectLightCalculator_H__
#define		__DirectLightCalculator_H__

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"LightmapCalculator.h"

/*
=========================================================================================

			CODE

=========================================================================================
*/

// ** class cDirectLightCalculator
class cDirectLightCalculator : public cLightmapCalculator {
protected:

	bool			isShadowJitter;

protected:

	sColor			CalculateDirectColor( const sVector3& position, const sVector3& normal, const sLight& light );
	void			CalculateFace( cLightmap *lm, int faceIndex, const sLMFace& face, const sLight *light );
	bool			IsFaceLit( const sLMFace& face, const sLight *light );

public:

					cDirectLightCalculator( const cLightmapper *lightmapper, IRayTracer *raytracer ) : cLightmapCalculator( lightmapper, raytracer ), isShadowJitter( true ) {}

	virtual void	Calculate( cLightmap *lm, const sLight *light, int *rowsProcessed = NULL );

	static float	CalculateLightAttenuation( const sVector3& a, const sVector3& b, float intensity, float radius );
	static float	CalculateLambertIntensity( const sVector3& a, const sVector3& b );
};

#endif	/*	!__LightCalculator_H__	*/