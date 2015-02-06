//
//  Filename:	PhotonMap.cpp
//	Created:	22:03:2012   20:13

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"Photonmap.h"
#include	"Lightmap.h"

#include	"DirectLightCalculator.h"

#ifdef WIN32
    #include	<windows.h>
    #include	<GL/gl.h>
#else
    #include    <OpenGL/OpenGL.h>
#endif


/*
=========================================================================================

			CODE

=========================================================================================
*/

// ** cPhotonMap::Create
void cPhotonMap::Create( cLightmapper *_lightmapper, int _width, int _height, int _maxDepth, int _finalGatherSamples, int _finalGatherRadius, bool _interpolate )
{
	width				= _width;
	height				= _height;
	maxDepth			= _maxDepth;
	lightmapper			= _lightmapper;
	finalGatherSamples	= _finalGatherSamples;
    finalGatherRadius   = _finalGatherRadius;
	interpolate			= _interpolate;
	cells				= new sPhotonMapCell[width * height];
}

// ** cPhotonMap::Calculate
void cPhotonMap::Calculate( const sLight *light )
{
	extern double random0to1();

	const int photonCount	= (light->area * 100) * (light->area * 100) * light->intensity * 1000;
	float	  photonEnergy	= light->intensity / photonCount;
	sVector3  L				= sVector3( light->x, light->y, light->z );
	sColor	  C				= sColor( light->r, light->g, light->b );

	for( int i = 0; i < photonCount; i++ ) {
		sVector3 dir = sVector3( random0to1() * 2.0 - 1.0, random0to1() * 2.0 - 1.0, random0to1() * 2.0 - 1.0 );
		dir.normalize();

		TracePhoton( L, dir, C, light->intensity, 0 );
	}
}

// ** cPhotonMap::StorePhoton
void cPhotonMap::StorePhoton( const sColor& color, float u, float v )
{
	int x = u * width;
	int y = v * height;

	sPhotonMapCell& cell = cells[y * width + x];
	bool first = cell.color.r == 0.0f && cell.color.g == 0.0f && cell.color.b == 0.0f;
	cell.color	 += color;
	cell.photons++;
//	cell.photons = 1;
	if( !first ) {
	//	cell.color = cell.color * 0.5f;
	}

	totalPhotons++;
}

// ** cPhotonMap::TracePhoton
void cPhotonMap::TracePhoton( const sVector3& position, const sVector3& direction, const sColor& color, float energy, int depth )
{
	if( depth > maxDepth || energy < 0.05f ) {
		return;
	}

	const float attRadius = 10.0f;

	sRayTraceResult hit;
	if( raytracer->TraceRay( position, position + direction * attRadius, &hit ) < 0 ) {
		return;
	}

	if( depth > 0 ) {
	//	const sLMFace& hitFace	= lightmapper->GetFace( hit.mesh, hit.triangle );
	//	cPhotonMap *photons		= ( cPhotonMap* )hitFace.lightmap->GetUserData();
		cPhotonMap *photons = lightmapper->GetPhotonMap( hit.lightmap );

		float attenuation	= cDirectLightCalculator::CalculateLightAttenuation( position, hit.point, energy, attRadius );
		float intensity		= cDirectLightCalculator::CalculateLambertIntensity( -direction, hit.normal ) * attenuation;

		photons->StorePhoton( hit.color * color * intensity, hit.u, hit.v );

		energy *= intensity;
	}

	sVector3 nextRay  = cLightmapper::GetRandomDirection( hit.point, hit.normal );
	TracePhoton( hit.point, nextRay, hit.color * color, energy, depth + 1 );
}

// ** cPhotonMap::InterpolateFacePhotons
void cPhotonMap::InterpolateFacePhotons( cLightmap *lm, const sTriangle& face )
{
/*
	static float _maxArea = 0.0f;
	static float _maxUVArea = 0.0f;
	static float _maxTxArea = 0.0f;

	float area	 = cLightmapper::TriangleArea( face.A, face.B, face.C );
	float uvArea = (face.uvMax.x - face.uvMin.x) * (face.uvMax.y - face.uvMin.y);
	float txArea = cLightmapper::TriangleArea(	sVector2( face.tA.x * lm->GetWidth(), face.tA.y * lm->GetHeight() ),
												sVector2( face.tB.x * lm->GetWidth(), face.tB.y * lm->GetHeight() ),
												sVector2( face.tC.x * lm->GetWidth(), face.tC.y * lm->GetHeight() ) );

	if( area > _maxArea ) {
		_maxArea = area;
		printf( "Max area: %2.8f / %2.8f / %2.2f\n", _maxArea, _maxUVArea, _maxTxArea );
	}
	if( uvArea > _maxUVArea ) {
		_maxUVArea = uvArea;
		printf( "Max area: %2.8f / %2.8f / %2.2f\n", _maxArea, _maxUVArea, _maxTxArea );
	}
	if( txArea > _maxTxArea ) {
		_maxTxArea = txArea;
		printf( "Max area: %2.8f / %2.8f / %2.2f\n", _maxArea, _maxUVArea, _maxTxArea );
	}
*/
	float area = cLightmapper::TriangleArea(sVector2( face.tA.x * lm->GetWidth(), face.tA.y * lm->GetHeight() ),
											sVector2( face.tB.x * lm->GetWidth(), face.tB.y * lm->GetHeight() ),
											sVector2( face.tC.x * lm->GetWidth(), face.tC.y * lm->GetHeight() ) );

	const float areaThreshold = 500;

	if( area > areaThreshold ) {
		GeatherTriangulatedFacePhotons( lm, face );
		return;
	}

	// ** Geather vertex color
	sColor cA = FinalGeather( face.A, face.nA, true );
	sColor cB = FinalGeather( face.B, face.nB, true );
	sColor cC = FinalGeather( face.C, face.nC, true );

	int	w		= lm->GetWidth();
	int h		= lm->GetHeight();
	int uStart	= face.uvMin.x * w;
	int uEnd	= face.uvMax.x * w;
	int vStart	= face.uvMin.y * h;
	int vEnd	= face.uvMax.y * h;
	sLumel *lumels = lm->GetLumels();

	for( int v = vStart; v <= vEnd; v++ ) {
		for( int u = uStart; u <= uEnd; u++ ) {
			sLumel *lumel = &lumels[w * v + u];

			float _u = (u + 0.5f) / float( w );
			float _v = (v + 0.5f) / float( h );

			float bu, bv;
			if( !cLightmapper::IsUVInside( face.tA, face.tB, face.tC, _u, _v, &bu, &bv ) ) {
				continue;
			}

			lumel->color = cLightmapper::TriangleInterpolate( cA, cB, cC, bu, bv );
		}
	}
}

// ** cPhotonMap::GeatherTriangulatedFacePhotons
void cPhotonMap::GeatherTriangulatedFacePhotons( cLightmap *lm, const sTriangle& face )
{
	sTriangle subdiv[4];

	Triangulate4( face, subdiv );
	for( int i = 0; i < 4; i++ ) {
		InterpolateFacePhotons( lm, subdiv[i] );
	}
}

// ** cPhotonMap::GeatherFacePhotons
void cPhotonMap::GeatherFacePhotons( cLightmap *lm, const sTriangle& face )
{
	int	w		= lm->GetWidth();
	int h		= lm->GetHeight();
	int uStart	= face.uvMin.x * w;
	int uEnd	= face.uvMax.x * w;
	int vStart	= face.uvMin.y * h;
	int vEnd	= face.uvMax.y * h;
	sLumel *lumels = lm->GetLumels();

	for( int v = vStart; v <= vEnd; v++ ) {
		for( int u = uStart; u <= uEnd; u++ ) {
			sLumel *lumel = &lumels[w * v + u];

			float _u = (u + 0.5f) / float( w );
			float _v = (v + 0.5f) / float( h );

			float bu, bv;
			if( !cLightmapper::IsUVInside( face.tA, face.tB, face.tC, _u, _v, &bu, &bv ) ) {
				continue;
			}

			lumel->color = FinalGeather( lumel->position, lumel->normal, false );
		}
	}
}

// ** cPhotonMap::Triangulate4
void cPhotonMap::Triangulate4( const sTriangle& face, sTriangle result[4] ) const
{
	// ** Source triangle
	const sVector3& A = face.A;
	const sVector3& B = face.B;
	const sVector3& C = face.C;

	const sVector3& nA = face.nA;
	const sVector3& nB = face.nB;
	const sVector3& nC = face.nC;

	const sVector2& tA = face.tA;
	const sVector2& tB = face.tB;
	const sVector2& tC = face.tC;

	// ** Tesselation points
	sVector3 xA = (A + B) * 0.5f;
	sVector3 xB = (B + C) * 0.5f;
	sVector3 xC = (C + A) * 0.5f;

	sVector3 xnA = (nA + nB) * 0.5f; xnA.normalize();
	sVector3 xnB = (nB + nC) * 0.5f; xnB.normalize();
	sVector3 xnC = (nC + nA) * 0.5f; xnC.normalize();

	sVector2 xtA = (tA + tB) * 0.5f;
	sVector2 xtB = (tB + tC) * 0.5f;
	sVector2 xtC = (tC + tA) * 0.5f;

	// ** Triangles
	sTriangle ta, tb, tc, td;

	// ** Triangle A
	ta.A =  A; ta.nA = nA; ta.tA = tA;
	ta.B = xA; ta.nB = xnA; ta.tB = xtA;
	ta.C = xC; ta.nC = xnC; ta.tC = xtC;
	cLightmapper::CalculateUVRect( ta.tA, ta.tB, ta.tC, ta.uvMin, ta.uvMax );

	// ** Triangle B
	tb.A = xA; tb.nA = xnA; tb.tA = xtA;
	tb.B =  B; tb.nB = nB; tb.tB = tB;
	tb.C = xB; tb.nC = xnB; tb.tC = xtB;
	cLightmapper::CalculateUVRect( tb.tA, tb.tB, tb.tC, tb.uvMin, tb.uvMax );

	// ** Triangle C
	tc.A = xC; tc.nA = xnC; tc.tA = xtC;
	tc.B = xB; tc.nB = xnB; tc.tB = xtB;
	tc.C =  C; tc.nC = nC; tc.tC = tC;
	cLightmapper::CalculateUVRect( tc.tA, tc.tB, tc.tC, tc.uvMin, tc.uvMax );

	// ** Triangle D
	td.A = xA; td.nA = xnA; td.tA = xtA;
	td.B = xB; td.nB = xnB; td.tB = xtB;
	td.C = xC; td.nC = xnC; td.tC = xtC;
	cLightmapper::CalculateUVRect( td.tA, td.tB, td.tC, td.uvMin, td.uvMax );

	result[0] = ta;
	result[1] = tb;
	result[2] = tc;
	result[3] = td;
}

// ** cPhotonMap::FinalGeather
sColor cPhotonMap::FinalGeather( const sVector3& point, const sVector3& normal, bool useCache ) const
{
	sColor geathered( 0, 0, 0 );
	const float maxGeatherDistance = 50;

	for( int k = 0; k < finalGatherSamples; k++ ) {
		sVector3 dir = cLightmapper::GetRandomDirection( point, normal );
		sRayTraceResult hit;

		if( raytracer->TraceRay( point, point + dir * maxGeatherDistance, &hit ) < 0 ) {
			continue;
		}

		cPhotonMap *photons = lightmapper->GetPhotonMap( hit.lightmap );
		geathered += photons->GeatherPhotons( hit.u * photons->width, hit.v * photons->height, finalGatherRadius );
	}

	geathered = geathered / finalGatherSamples;

	return geathered;
}

// ** cPhotonMap::ApplyTo
void cPhotonMap::ApplyTo( cLightmap *lm, cLightmapper *lightmapper )
{
	printf( "Geathering photons...\n" );

	for( int i = 0, n = lightmapper->GetTotalFaces(); i < n; i++ ) {
		if( interpolate ) {
			InterpolateFacePhotons( lm, lightmapper->GetFace( i ) );
		} else {
			GeatherFacePhotons( lm, lightmapper->GetFace( i ) );
		}
		printf( "Face %d / %d\r", i, n );
	}
}

// ** cPhotonMap::GeatherPhotons
sColor cPhotonMap::GeatherPhotons( int x, int y, int radius ) const
{
	if( x < 0 || y < 0 || x >= width || y >= height ) {
		return sColor( 0, 0, 0 );
	}

	sColor color;
	int photons = 0;

	for( int j = y - radius; j <= y + radius; j++ ) {
		for( int i = x - radius; i <= x + radius; i++ ) {
			if( i < 0 || j < 0 || i >= width || j >= height ) {
				continue;
			}

			const sPhotonMapCell& cell = cells[j * width + i];
			float distance = sqrtf( (x - i) * (x - i) + (y - j) * (y - j) );
			if( distance > radius ) {
				continue;
			}

			color   += cell.color /*/ cell.photons*/;
			photons += cell.photons;
		}
	}

    if( photons == 0 ) {
        return sColor( 0.0f, 0.0f, 0.0f );
    }

//	const sPhotonMapCell& cell = cells[y * width + x];
//	color = cell.color / cell.photons;
	
	color = color / photons /*/ (radius * radius * 3.14159f)*/;

	return color;
}

// ** cPhotonMap::Debug_Save
void cPhotonMap::Debug_Save( const char *fileName ) const
{
	char buffer[256];

	FILE	*file;
	int		image_size = 0;

	sprintf( buffer, "%s-%d.tga", fileName, width );

	unsigned char tga_header_a[12]   = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char tga_info_header[6] = { 0, 0, 0, 0, 0, 0 };

	file = fopen( buffer, "wb" );
	if( !file ) {
		return;
	}

	fwrite( tga_header_a, 1, sizeof( tga_header_a ), file );

	int channels = 3;

	tga_info_header[0] = width  % 256;
	tga_info_header[1] = width  / 256;
	tga_info_header[2] = height % 256;
	tga_info_header[3] = height / 256;
	tga_info_header[4] = channels * 8;
	tga_info_header[5] = 0;

	fwrite( tga_info_header, 1, sizeof( tga_info_header ), file );
	image_size = width * height * channels;

	for( int j = 0; j < height; j++ ) {
		for( int i = 0; i < width; i++ ) {
			const sPhotonMapCell& cell = cells[j*width + i];

			unsigned char r = std::min( cell.color.b / cell.photons * 255.0, 255.0 );
			unsigned char g = std::min( cell.color.g / cell.photons * 255.0, 255.0 );
			unsigned char b = std::min( cell.color.r / cell.photons * 255.0, 255.0 );

		//	unsigned char pixel[] = { r > 255 ? 255 : r, g > 255 ? 255 : g, b > 255 ? 255 : b };
            unsigned char pixel[] = { r, g, b };
			fwrite( pixel, sizeof( pixel ), 1, file );
		}
	}

	fclose( file );
}
