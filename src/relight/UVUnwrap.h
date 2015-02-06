//
//  Filename:	UVUnwrap.h
//	Created:	22:02:2012   17:24

#ifndef		__UVUnwrap_H__
#define		__UVUnwrap_H__

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cassert>
#include <algorithm>
#include <float.h>

#ifdef HAVE_OPEN_NL
    #include <NL/nl.h>
#endif

#include <OpenGL/gl.h>
#include <GLUT/GLUT.h>

#define		_USE_MATH_DEFINES
#undef		min
#undef		max

#define		lmTYPE	float
#define     DWORD   unsigned int

#ifndef WIN32
    inline DWORD timeGetTime( void ) { return 0; }
#endif

/*
=========================================================================================

			CODE

=========================================================================================
*/

struct sMesh;
struct sMeshVertex;

template <class T> inline T nl_min(T x, T y) { return x < y ? x : y ; }
template <class T> inline T nl_max(T x, T y) { return x > y ? x : y ; }

// ** struct sVector3
struct sVector3 {
	lmTYPE	x, y, z;
			sVector3( void ) {}
			sVector3( lmTYPE _x, lmTYPE _y, lmTYPE _z ) : x( _x ), y( _y ), z( _z ) {}
			sVector3( const float v[3] ) : x( v[0] ), y( v[1] ), z( v[2] ) {}

    lmTYPE length() const { 
        return sqrt(x*x + y*y + z*z) ;
    }
    lmTYPE normalize() {
        lmTYPE l = length();
		if( l ) {
			x /= l ; y /= l ; z /= l;
		}
		return l;
    }

	sVector3 	operator - ( void ) const { return sVector3( -x, -y, -z ); }
	bool		operator == ( const sVector3& other ) const { return x == other.x && y == other.y && z == other.z; }
};

// ** struct sVector3
struct sVector2 {
	lmTYPE	x, y;
			sVector2( void ) {}
			sVector2( lmTYPE _x, lmTYPE _y ) : x( _x ), y( _y ) {}
			sVector2( const float v[2] ) : x( v[0] ), y( v[1] ) {}

    lmTYPE lengthSq() const { 
        return x*x + y*y;
    }
    lmTYPE length() const { 
        return sqrt( lengthSq() );
    }
    lmTYPE normalize() {
        lmTYPE l = length();
        x /= l ; y /= l;
		return l;
    }
};

typedef struct												// Create A Structure
{
	unsigned char	*imageData;										// Image Data (Up To 32 Bits)
	unsigned int	bpp;											// Image Color Depth In Bits Per Pixel.
	unsigned int	width;											// Image Width
	unsigned int	height;											// Image Height
	unsigned int	texID;											// Texture ID Used To Select A Texture
} TextureImage;

// I/O

inline std::ostream& operator<<(std::ostream& out, const sVector2& v) {
    return out << v.x << " " << v.y ;
}

inline std::ostream& operator<<(std::ostream& out, const sVector3& v) {
    return out << v.x << " " << v.y << " " << v.z ;
}

inline std::istream& operator>>(std::istream& in, sVector2& v) {
    return in >> v.x >> v.y ;
}

inline std::istream& operator>>(std::istream& in, sVector3& v) {
    return in >> v.x >> v.y >> v.z ;
}

/* dot product */
inline double operator*(const sVector3& v1, const sVector3& v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z ;
}

/* dot product */
inline double operator*(const sVector2& v1, const sVector2& v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

/* dot product */
inline bool operator == (const sVector2& v1, const sVector2& v2) {
    return v1.x == v2.x && v1.y == v2.y;
}

/* cross product */
inline sVector3 operator^(const sVector3& v1, const sVector3& v2) {
    return sVector3(
        v1.y*v2.z - v2.y*v1.z,
        v1.z*v2.x - v2.z*v1.x,
        v1.x*v2.y - v2.x*v1.y
    ) ;
}

inline sVector3 operator+(const sVector3& v1, const sVector3& v2) {
    return sVector3(
        v1.x + v2.x,
        v1.y + v2.y,
        v1.z + v2.z
    ) ;
}

inline sVector3 operator*(const sVector3& v1, double scalar) {
    return sVector3(
        v1.x * scalar,
        v1.y * scalar,
        v1.z * scalar
    ) ;
}

inline sVector3 operator-(const sVector3& v1, const sVector3& v2) {
    return sVector3(
        v1.x - v2.x,
        v1.y - v2.y,
        v1.z - v2.z
    ) ;
}

inline sVector2 operator+(const sVector2& v1, const sVector2& v2) {
    return sVector2(
        v1.x + v2.x,
        v1.y + v2.y
    ) ;
}

inline sVector2 operator-(const sVector2& v1, const sVector2& v2) {
    return sVector2(
        v1.x - v2.x,
        v1.y - v2.y
    ) ;
}

inline sVector2 operator*(const sVector2& v1, float scalar) {
    return sVector2(
        v1.x * scalar,
        v1.y * scalar
    ) ;
}

inline sVector2 operator/(const sVector2& v1, float scalar) {
    return sVector2(
        v1.x / scalar,
        v1.y / scalar
    ) ;
}

// ** struct sVertex
struct sVertex {
    sVector3	pos;
    sVector2	uv;
    bool		locked;
    int			id;

				sVertex( void ) : locked( false ), id( -1 ) {}
				sVertex( const sVector3& p, const sVector2& t ) : pos( p ), uv(t), locked( false) , id( -1 ) {}
};

// ** sFacet
struct sFacet : public std::vector<int> {
public:
};

// ** class cLSCMMesh
class cLSCMMesh {
friend class cUVUnwrap;
private:

    std::vector< sVertex >	vertex;
    std::vector< sFacet >	facet;
    bool					in_facet;

public:

							cLSCMMesh( void );

    sVertex*				AddVertex( void );
    sVertex*				AddVertex( const sVector3& p, const sVector2& t );

    void					BeginFacet( void );
    void					EndFacet( void );

    void					AddVertexToFacet( unsigned int i );
    void					Clear( void );

    void					Load( const std::string& file_name );
    void					Save( const std::string& file_name );
};

#ifdef HAVE_OPEN_NL
// ** class cUVUnwrap
class cUVUnwrap {
private:

    cLSCMMesh	*mesh;

protected:

	void		SetupForLSCM( void );
    void		SetupFaceForLSCM( const sFacet& facet );
	void		SetupLSCMRelations( const sVertex& v0, const sVertex& v1, const sVertex& v2 );
	static void	ProjectTriangle( const sVector3& p0, const sVector3& p1, const sVector3& p2, sVector2& z0, sVector2& z1, sVector2& z2 );

    void		SolverToMesh( void );
    void		MeshToSolver( void );
    void		Project( void );

public:

    void		Unwrap( cLSCMMesh *mesh );
};
#endif

#if 0

// ** class cSimpleUnwrap
class cSimpleUnwrap {

	typedef OpenMesh::TriMesh_ArrayKernelT<>	tTriangleMesh;
	typedef tTriangleMesh::FaceHandle			tFace;
	typedef tTriangleMesh::VertexHandle			tVertex;
	typedef std::vector< tVertex >				tVertexHandles;
	typedef std::vector< tFace >				tFaceHandles;

private:

	// ** struct sIntersection
	struct sIntersection {
		int			solutions;
		sVector2	a, b;

					sIntersection( void ) : solutions( 0 ) {}
		void		InsertSolution( const sVector2& v ) {
			if( solutions == 0 ) {
				a = v;
				solutions++;
			}
			else if( solutions == 1 ) {
				b = v;
				solutions++;
			}
		}
	};

private:

	tTriangleMesh				triMesh;
	tFaceHandles				processed;
	tVertexHandles				vertices;
	tFaceHandles				faces;
	sMesh						*mesh;

	float						r, g, b;

	std::vector< sMesh* >		islands;

private:

	void						UnwrapFace( sMeshVertex *vertices, const tFace& face, const int *adj );
	bool						IsPointInsideTriangles( const sMeshVertex *vertices, const sVector2& point ) const;
	bool						IsFaceProcessed( const tFace& face ) const;
	void						NormalizeUV( void );

	void						BeginIsland( void );

public:

	sIntersection				CircleToCircleIntersection( const sVector2& circlePosition1, float radius1, const sVector2& circlePosition2, float radius2 ) const;
	void						Unwrap( sMesh *mesh );
};

#endif

#endif	/*	!__UVUnwrap_H__	*/