//
//  Filename:	PhotonMap.h
//	Created:	22:03:2012   20:10

#ifndef		__PhotonMap_H__
#define		__PhotonMap_H__

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"Lightmapper.h"

/*
=========================================================================================

			CODE

=========================================================================================
*/

// ** class cPhotonMap
class cPhotonMap {

	// ** struct sPhotonMapCell
	struct sPhotonMapCell {
		sColor			color;
		int				photons;

						sPhotonMapCell( void ) : photons( 0.0f ) {}
	};

	// ** struct sTriangle
	struct sTriangle {
		sVector3		A, B, C;
		sVector3		nA, nB, nC;
		sVector2		tA, tB, tC;
		sVector2		uvMin, uvMax;

						sTriangle( void ) {}
						sTriangle( const sLMFace& face ) {
							A = face.vertices[0].position;
							B = face.vertices[1].position;
							C = face.vertices[2].position;

							nA = face.vertices[0].normal;
							nB = face.vertices[1].normal;
							nC = face.vertices[2].normal;

							tA = face.vertices[0].uv;
							tB = face.vertices[1].uv;
							tC = face.vertices[2].uv;

							uvMin = face.uvMin;
							uvMax = face.uvMax;
						}
	};

private:

	sPhotonMapCell		*cells;
	cLightmapper		*lightmapper;
	int					width, height;
	int					totalPhotons;
	int					maxDepth;
	IRayTracer			*raytracer;
	int					finalGatherSamples;
    int                 finalGatherRadius;
	bool				interpolate;

//	mutable tIrradianceCache	irradianceCache;

private:

	void				StorePhoton( const sColor& color, float u, float v );
	void				TracePhoton( const sVector3& position, const sVector3& direction, const sColor& color, float energy, int depth );
	sColor				GeatherPhotons( int x, int y, int radius ) const;
	void				InterpolateFacePhotons( cLightmap *lm, const sTriangle& face );
	void				GeatherTriangulatedFacePhotons( cLightmap *lm, const sTriangle& face );
	void				GeatherFacePhotons( cLightmap *lm, const sTriangle& face );
	sColor				FinalGeather( const sVector3& point, const sVector3& normal, bool useCache ) const;
	void				Triangulate4( const sTriangle& face, sTriangle result[4] ) const;

//	bool				GetCachedValue( const sVector3& point, sColor& value ) const;

public:

						cPhotonMap( IRayTracer *_raytracer )
							: raytracer( _raytracer ), totalPhotons( 0 ), maxDepth( 1 ), finalGatherSamples( 8 ) {}

	void				Create( cLightmapper *lightmapper, int width, int height, int maxDepth, int finalGatherSamples, int finalGatherRadius, bool interpolate );
	void				Calculate( const sLight *light );
	int					GetTotalPhotons( void ) const { return totalPhotons; }
	void				ApplyTo( cLightmap *lm, cLightmapper *lightmapper );

	void				Debug_Save( const char *fileName ) const;
};

#endif	/*	!__PhotonMap_H__	*/