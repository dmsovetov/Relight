//
//  Filename:	LightmapCalculator.cpp
//	Created:	22:03:2012   16:09

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"LightmapCalculator.h"
#include	"Lightmap.h"

/*
=========================================================================================

			CODE

=========================================================================================
*/

// ** cLightmapCalculator::GetShadowTerm
float cLightmapCalculator::GetShadowTerm( const sVector3& position, const sVector3& light, float lightRadius, int jitter ) const
{
	if( !raytracer ) {
		return 1.0f;
	}

	extern double random0to1( void );

	float shadow = 0.0f;

	for( int k = 0; k < jitter; k++ ) {
		// ** Jitter light vector
		sVector3 end = light;
		if( jitter > 1 ) {
			end = sVector3( light.x + (random0to1() * 2 - 1) * lightRadius, light.y + (random0to1() * 2 - 1) * lightRadius, light.z + (random0to1() * 2 - 1) * lightRadius );
		}
		sVector3 dir = end - position;

		// ** Trace scene
	//	sVector3 start = position + dir * 0.01;
		sRayTraceResult result;
		if( raytracer->TraceRay( position, end, &result ) >= 0 ) {
			shadow++;
		}
	}

	return shadow / jitter;
}

// ** cLightmapCalculator::GetFirstRow
int cLightmapCalculator::GetFirstRow( void ) const
{
	row = startRow;
	return row;
}

// ** cLightmapCalculator::GetNextRow
int cLightmapCalculator::GetNextRow( void ) const
{
	row += rowStride;
	return row;
}

// ** cLightmapCalculator::GetFirstFace
int cLightmapCalculator::GetFirstFace( void ) const
{
	face = startFace;
	return face;
}

// ** cLightmapCalculator::GetNextFace
int cLightmapCalculator::GetNextFace( void ) const
{
	face += faceStride;
	return face;
}