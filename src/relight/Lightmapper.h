//
//  Filename:	Lightmapper.h
//	Created:	28:02:2012   10:40

#ifndef		__Lightmapper_H__
#define		__Lightmapper_H__

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"PathTracer.h"
#include	<pthread.h>

/*
=========================================================================================

			CODE

=========================================================================================
*/

class cLightmapper;
class cLightmap;
class cLightmapCalculator;
	class cDirectLightCalculator;
	class cIndirectLightCalculator;

class cPhotonMap;

// ** struct sLight
struct sLight {
	float		x, y, z;
	float		r, g, b;
	float		intensity;
	float		radius;
	float		area;
	int			jitter;
	bool		castShadow;
};

// ** struct sLMVertex
struct sLMVertex {
	sVector3	position;
	sVector3	normal;
	sColor		color;
	sVector2	uv;
};

// ** sLMFace
struct sLMFace {
	int			lightmap;
	sLMVertex	vertices[3];
	sVector3	normal, centroid;
	const sMesh	*mesh;

	sVector2	uvMin, uvMax;
};

// ** struct sMeshVertex
struct sMeshVertex {
	enum { eElementCount = 11 };

	union {
		struct 
		{
			float		position[3];
			float		normal[3];
			float		color[3];
			float		uv[2];
		};
		float v[eElementCount];
	};

	int		lightmapIndex;

	// ** sMeshVertex
	bool operator < ( const sMeshVertex& other ) const {
		for( int i = 0; i < eElementCount; ++i ) {
			if( v[i] != other.v[i] ) {
				return v[i] < other.v[i]; 
			}
		}

		return false;
	}
};

// ** sMesh
struct sMesh {
    sMeshVertex		*vertices;
    int				*indices;
    int				totalVertices;
    int				totalFaces;

    sMesh( void )
    : vertices( NULL ), indices( NULL ), totalVertices( 0 ), totalFaces( 0 ) {}
};

// ** struct sLightmapCalculation
struct sLightmapCalculation {
	pthread_t				thread;
	cLightmapper			*lightmapper;
	cLightmap				*lightmap;
	cLightmapCalculator		*calculator;
	int						rowsProcessed;
	bool					isReady;

							sLightmapCalculation( void ) : isReady( false ), rowsProcessed( 0 ) {}
};

// class cLightmapper
class cLightmapper {
friend class cLightmap;
private:

	std::vector< sLight >		lights;
	std::vector< sLMFace >		faces;
	std::vector< cLightmap* >	lightmaps;
	std::vector< cPhotonMap* >	photonMaps;
	IRayTracer					*raytracer;
	bool						copyColorFromVertex;

private:

	static void*			Calculate( void *userData );

public:

							cLightmapper( void ) : raytracer( NULL ), copyColorFromVertex( false ) {}

	cLightmap*				CreateLightmap( void );
	cPhotonMap*				CreatePhotonMap( void );
	cLightmapCalculator*	CreateDirectLightCalculator( void ) const;
	cLightmapCalculator*	CreateIndirectLightCalculator( int samples, int maxDepth ) const;
	void					AddLight( const sLight& light );
	void					AddFace( int lightmap, const sMesh *mesh, int a, int b, int c, const sMeshVertex *vertices );
	void					SetRayTracer( IRayTracer *tracer ) { raytracer = tracer; }
	void					SetCopyColorFromVertex( bool value ) { copyColorFromVertex = value; }
	bool					IsCopyColorFromVertex( void ) const { return copyColorFromVertex; }
	sLightmapCalculation*	CalculateDirectLight( cLightmap *lm, int row, int stride );
	sLightmapCalculation*	CalculateIndirectLight( cLightmap *lm, int samples, int maxDepth, int row, int stride );
	const sLMFace&			GetFace( int index ) const { return faces[index]; }
	cPhotonMap*				GetPhotonMap( int index ) const { return photonMaps[index]; }
//	const sLMFace&			GetFace( const sMesh *mesh, int index ) const;
	int						GetTotalFaces( void ) const { return faces.size(); }

	static sVector3			GetRandomDirection( const sVector3& point, const sVector3& normal );
	static sVector3			TriangleInterpolate( const sVector3& a, const sVector3& b, const sVector3& c, float u, float v );
	static sVector2			TriangleInterpolate( const sVector2& a, const sVector2& b, const sVector2& c, float u, float v );
	static sColor			TriangleInterpolate( const sColor& a, const sColor& b, const sColor& c, float u, float v );
	static float			TriangleArea( const sVector2& A, const sVector2& B, const sVector2& C );
	static sVector2			CalculateBarycentric( const sVector3& a, const sVector3& b, const sVector3& c, const sVector3& point );
//	static bool				IsUVInside( const sLMFace& face, double u, double v, double *bu, double *bv );
	static bool				IsUVInside( const sVector2& A, const sVector2& B, const sVector2& C, float u, float v, float *bu = NULL, float *bv = NULL );
	static bool				IsInsideTriangle( const sVector2& A, const sVector2& B, const sVector2& C, const sVector2& point );
	static void				CalculateUVRect( const sVector2& A, const sVector2& B, const sVector2& C, sVector2& min, sVector2& max );
};

#endif	/*	!__Lightmapper_H__	*/