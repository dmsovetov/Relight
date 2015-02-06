//
//  Filename:	InirectLightCalculator.cpp
//	Created:	22:03:2012   17:22

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"IndirectLightCalculator.h"
#include	"Lightmap.h"

/*
=========================================================================================

			CODE

=========================================================================================
*/

// ** cIndirectLightCalculator::Calculate
void cIndirectLightCalculator::Calculate( cLightmap *lm, const sLight *light, int *rowsProcessed )
{
	int		width   = lm->GetWidth();
	int		height  = lm->GetHeight();
	sLumel	*lumels = lm->GetLumels();

	for( int j = GetFirstRow(); j < height; j = GetNextRow() ) {
		for( int i = 0; i < width; i++ ) {
			sLumel& lumel = lumels[j*width + i];
			sColor	indirect( 0, 0, 0 ); 

			if( !lumel.IsValid() ) {
				continue;
			}
			
			for( int k = 0; k < indirectSamples; k++ ) {
				sVector3 dir = cLightmapper::GetRandomDirection( lumel.position, lumel.normal );
				float	 dp  = CalculateLambertIntensity( lumel.normal, dir );
				indirect += CalculateIndirectColor( *light, lumel.position, dir, lumel.normal ) * dp * (1.0 / indirectSamples);
			}

			lumel.color += indirect;
		}

		printf( "Indirect %d \ %d\r", j, height );
	//	if( rowsProcessed ) *rowsProcessed++;
	}
}

// ** cIndirectLightCalculator::CalculateIndirectColor
sColor cIndirectLightCalculator::CalculateIndirectColor( const sLight& light, const sVector3& position, const sVector3& direction, const sVector3& _normal, int depth )
{
	float		radius = (light.radius * pow( 1.0f - float( depth ) / 5, 2.0f ) );
	sVector3	start  = position + _normal * 0.01;
	sVector3	end	   = position + direction * radius;

	if( !raytracer || depth > indirectMaxDepth ) {
		return sColor( 0.0, 0.0, 0.0 );
	}

	sRayTraceResult hit;
	if( raytracer->TraceRay( start, end, &hit ) < 0 ) {
		return sColor( 0.0, 0.0, 0.0 );
	}

	// ** Calculate direct color for this point
	sColor result = CalculateDirectColor( hit.point, hit.normal, light );

	// ** Shoot the ray and sample incoming flux
	sVector3 nextRay  = cLightmapper::GetRandomDirection( hit.point, hit.normal );
	float attenuation = CalculateLightAttenuation( position, hit.point, light.intensity, light.radius );
	float intensity	  = CalculateLambertIntensity( nextRay, _normal ) * attenuation;

	if( intensity > 0.01f ) {
		result += CalculateIndirectColor( light, hit.point, nextRay, hit.normal, depth + 1 ) * intensity;
	}

	return result;
}