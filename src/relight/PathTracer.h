//
//  Filename:	PathTracer.h
//	Created:	22:02:2012   17:18

#ifndef		__PathTracer_H__
#define		__PathTracer_H__

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"UVUnwrap.h"

/*
=========================================================================================

			CODE

=========================================================================================
*/

struct sMesh;

// ** struct sColor
struct sColor {
	lmTYPE			r, g, b, a;

					sColor( void ) : r( 0.0 ), g( 0.0 ), b( 0.0 ), a( 1.0 ) {}
					sColor( const sColor& other ) : r( other.r ), g( other.g ), b( other.b ), a( other.a ) {}
					sColor( lmTYPE _r, lmTYPE _g, lmTYPE _b, lmTYPE _a = 1.0f ) : r( _r ), g( _g ), b( _b ), a( _a ) {}
					sColor( const lmTYPE *rgb ) : r( rgb[0] ), g( rgb[1] ), b( rgb[2] ), a( 1.0 ) {}

	sColor			operator * ( lmTYPE scalar ) const { return sColor( r * scalar, g * scalar, b * scalar, a * scalar ); }
	sColor			operator / ( lmTYPE scalar ) const { return sColor( r / scalar, g / scalar, b / scalar, a / scalar ); }
	sColor			operator * ( const sColor& other ) const { return sColor( r * other.r, g * other.g, b * other.b, a * other.a ); }
	sColor			operator + ( const sColor& other ) const { return sColor( r + other.r, g + other.g, b + other.b, 1.0f ); }
	const sColor&	operator += ( const sColor& other ) { r += other.r; g += other.g; b += other.b; a = 1.0f; return *this; }
	bool			operator == ( const sColor& other ) { return other.r == r && other.g == g && other.b == b; }
};

// ** struct sMaterial
struct sMaterial {
	sColor		diffuse;
	sColor		specular;
	sColor		emissive;
	double		shininess;

				sMaterial( void ) : shininess( 0.0 ) {}
};

// ** srtuct sRayTraceResult
struct sRayTraceResult {
	sVector3			point;
	sVector3			normal;
	sColor				color;
	int					lightmap;
	float				u, v;

    // ?? Bullet deprecated
    void				*object;
    int					triangle;
    sMesh				*mesh;
};

// ** class IRayTracer
class IRayTracer {
public:

	virtual int		TraceRay( const sVector3& start, const sVector3& end, sRayTraceResult *result = 0 ) = 0;
	virtual int		GetRayCount( void ) = 0;
	virtual void	AddMesh( sMesh& mesh ) = 0;
	virtual void	Create( void ) = 0;
};
/*
// ** class cPathTracer
class cPathTracer {
private:

	bool		isDirectLighting;
	float		lightIntensityScale;
	int			maxDepth;
	IRayTracer	*tracer;

private:

	sColor		TraceObject( const double *point, const double *normal, const sColor& diffuse, double weight, double kw, int depth );
	sColor		TraceLight( const double *point, const double *normal, const sColor& diffuse, double weight, double kw, int depth, double probKillInv );

	void		GetDirectionUsingCosWeightedLambertian( const double *point, const double *normal, double *direction );
	sColor		SampleDirectLighting( const double *point, const double *normal, const double *ray, int object );

public:

	sColor		TracePixel( const double *point, const double *ray, double weight = 1.0, int depth = 0 );
	int			TraceScene( const double *eye, float *image, int width, int height );

	void		SetTracer( IRayTracer *_tracer ) { tracer = _tracer; }
	IRayTracer*	GetTracer( void ) const { return tracer; }
	void		SetDirectLighting( bool value ) { isDirectLighting = value; }
	bool		IsDirectLighting( void ) const { return isDirectLighting; }
	void		SetMaxDepth( int value ) { maxDepth = value; }
	int			GetMaxDepth( void ) const { return maxDepth; }
	void		SetLightIntensityScale( float value ) { lightIntensityScale = value; }
	float		GetLightIntensityScale( void ) const { return lightIntensityScale; }
};*/

#endif	/*	!__PathTracer_H__	*/