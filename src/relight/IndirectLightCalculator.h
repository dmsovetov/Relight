//
//  Filename:	IndirectLightCalculator.h
//	Created:	22:03:2012   17:20

#ifndef		__IndirectLightCalculator_H__
#define		__IndirectLightCalculator_H__

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"DirectLightCalculator.h"

/*
=========================================================================================

			CODE

=========================================================================================
*/

// ** class cIndirectLightCalculator
class cIndirectLightCalculator : public cDirectLightCalculator {
private:

	int				indirectSamples;
	int				indirectMaxDepth;

private:

	sColor			CalculateIndirectColor( const sLight& light, const sVector3& position, const sVector3& direction, const sVector3& normal, int depth = 0 );

public:

					cIndirectLightCalculator( const cLightmapper *lightmapper, IRayTracer *raytracer )
						: cDirectLightCalculator( lightmapper, raytracer ), indirectSamples( 16 ), indirectMaxDepth( 25 ) { isShadowJitter = false; }

	// ** cLightCalculator
	virtual void	Calculate( cLightmap *lm, const sLight *light, int *rowsProcessed = NULL );

	// ** cIndirectLightCalculator
	void			SetTotalIndirectSamples( int value ) { indirectSamples = value; }
	int				GetTotalIndirectSamples( void ) const { return indirectSamples; }
	void			SetIndirectMaxDepth( int value ) { indirectMaxDepth = value; }
	int				GetIndirectMaxDepth( void ) const { return indirectMaxDepth; }
};

#endif	/*	!__LightCalculator_H__	*/