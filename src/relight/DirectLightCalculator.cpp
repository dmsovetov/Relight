//
//  Filename:	DirectLightCalculator.cpp
//	Created:	22:03:2012   17:08

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"DirectLightCalculator.h"
#include	"Lightmap.h"
#include	"Lightmapper.h"

/*
=========================================================================================

			CODE

=========================================================================================
*/

// ** cDirectLightCalculator::Calculate
void cDirectLightCalculator::Calculate( cLightmap *lm, const sLight *light, int *rowsProcessed )
{
/*
	for( int i = GetFirstFace(), n = lightmapper->GetTotalFaces(); i < n; i = GetNextFace() ) {
		CalculateFace( lm, i, lightmapper->GetFace( i ), light );
		printf( "Direct %d / %d\r", i, n );
	}
*/

	int		width   = lm->GetWidth();
	int		height  = lm->GetHeight();
	sLumel	*lumels = lm->GetLumels();

	for( int j = GetFirstRow(); j < height; j = GetNextRow() ) {
		for( int i = 0; i < width; i++ ) {
			sLumel& lumel = lumels[j*width + i];

			if( !lumel.IsValid() ) {
				continue;
			}

			lumel.color += CalculateDirectColor( lumel.position, lumel.normal, *light );
		}

		printf( "Direct %d \ %d\r", j, height );
	}

}

// ** cDirectLightCalculator::IsFaceLit
bool cDirectLightCalculator::IsFaceLit( const sLMFace &face, const sLight *light )
{
//	sVector3 N = (face.vertices[0].normal + face.vertices[1].normal + face.vertices[2].normal) * (1.0f / 3.0f);
//	sVector3 C = (face.vertices[0].position + face.vertices[1].position + face.vertices[2].position) * (1.0f / 3.0f);

	sVector3 dir = sVector3( light->x, light->y, light->z ) - face.centroid; dir.normalize();

	if( (dir * face.normal) < 0.0f ) {
		return false;
	}

	return true;
}

// ** cDirectLightCalculator::CalculateFace
void cDirectLightCalculator::CalculateFace( cLightmap *lm, int faceIndex, const sLMFace& face, const sLight *light )
{
	if( face.lightmap != lm->GetIndex() ) {
		return;
	}

	if( !IsFaceLit( face, light ) ) {
		return;
	}

	int		width   = lm->GetWidth();
	int		height  = lm->GetHeight();
	sLumel	*lumels = lm->GetLumels();

	int uStart	= face.uvMin.x * width;
	int uEnd	= face.uvMax.x * width;
	int vStart	= face.uvMin.y * height;
	int vEnd	= face.uvMax.y * height;

	for( int v = vStart; v <= vEnd; v++ ) {
		for( int u = uStart; u <= uEnd; u++ ) {
			if( v >= width ) break;

			sLumel *lumel = &lumels[width * v + u];

			if( !lumel->IsValid() || lumel->faceIndex != faceIndex ) {
				continue;
			}

		//	double bu, bv;
		//	if( !cLightmapper::IsUVInside( face, u / float( width ), v / float( height ), &bu, &bv ) ) {
		//		continue;
		//	}

			if( lumel->color == sColor( 255, 0, 255 ) ) {
				lumel->color = sColor( 0, 0, 0 );
			}
			lumel->color += CalculateDirectColor( lumel->position, lumel->normal, *light );
		}
	}
}

// ** cDirectLightCalculator::CalculateLightAttenuation
float cDirectLightCalculator::CalculateLightAttenuation( const sVector3& a, const sVector3& b, float intensity, float radius )
{
//	float distance = (a - b).length();
//	float k = distance / radius + 1;
//	return intensity / (k * k);

	// ** Linear
	float distance	= (a - b).length();
	float att		= 1.0f - (distance / radius);

	return att < 0.0f ? 0.0f : att;

// ** Quadric
//	float distance	= (a - b).length();
//	float att		= 1.0f - (distance / radius);

//	return att < 0.0f ? 0.0f : att * att;
}

// ** cDirectLightCalculator::CalculateLambertIntensity
float cDirectLightCalculator::CalculateLambertIntensity( const sVector3& a, const sVector3& b )
{
	float dp = a * b;
	return dp < 0.0f ? 0.0f : dp;
}

// ** cDirectLightCalculator::CalculateDirectColor
sColor cDirectLightCalculator::CalculateDirectColor( const sVector3& position, const sVector3& normal, const sLight& light )
{
	sVector3 L				= sVector3( light.x, light.y, light.z );
	sVector3 dir			= L - position; dir.normalize();
	float	 attenuation	= CalculateLightAttenuation( position, L, light.intensity, light.radius );
	float	 intensity		= CalculateLambertIntensity( dir, normal ) * attenuation;
	float	 shadow			= 0.0f;

	// ** Shadow
	if( light.castShadow && intensity >= 0.001f ) {
		int jitter = isShadowJitter ? light.jitter : 1;
		intensity *= 1.0f - GetShadowTerm( position, L, light.area, jitter );
	}

	const sColor diffuseColor( 1.0, 1.0, 1.0 );

	return sColor( light.r, light.g, light.b ) * diffuseColor * intensity;
}