//
//  Filename:	Radiosity.h
//	Created:	01:06:2012   11:11

#ifndef		__Radiosity_H__
#define		__Radiosity_H__

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	<vector>
#include	"Relight.h"

/*
=========================================================================================

			CODE

=========================================================================================
*/

class cLightmap;
struct sLumel;
class IRayTracer;

// ** struct sFormFactor
struct sFormFactor {
	unsigned short	sample;
	float			weight;

					sFormFactor( void ) {}
					sFormFactor( unsigned short _sample, float _weight )
						: sample( _sample ), weight( _weight ) {}
};


typedef std::vector< sFormFactor >		tFormFactorVector;

// ** struct sRadiositySample
struct sRadiositySample {
    relight::Vec3       position;
	relight::Vec3       normal;
    relight::Color		injected;
	relight::Color      diffuse;
	relight::Color      indirect;
	tFormFactorVector	formFactors;
	bool				isValid;

	sLumel				*indirectLumel;

						sRadiositySample( void )
							: isValid( false ), indirectLumel( NULL ), injected( 0.0f, 0.0f, 0.0f ), diffuse( 0.0f, 0.0f, 0.0f ), indirect( 0.0f, 0.0f, 0.0f ) {}
};

typedef std::vector<sRadiositySample>	tRadiositySamples;

// ** class cRadiositySolver
class cRadiositySolver {
private:

	sRadiositySample		*samples;
	int						totalSamples;
	IRayTracer				*model;

	cLightmap				*indirect;

	int						totalFormFactors;
	bool					interpolate;

private:

	void					RefineFormFactors( sRadiositySample& sample, int maxFormFactors );

	void					DistanceFormFactor( unsigned short receiverIndex, unsigned short senderIndex );
	void					DistanceSqAreaFormFactor( unsigned short receiverIndex, unsigned short senderIndex );

	void					InterpolateSample( sRadiositySample& sample, unsigned short a, unsigned short b );

public:

							cRadiositySolver( IRayTracer *model );

	void					Create( cLightmap *indirect );

	// ** Realtime
	void					Solve( void );
	void					Clear( void );
	void					InjectPointLight( float x, float y, float z, float r, float g, float b, float intensity, float radius );
	void					InjectIndirect( void );

	void					UpdateDirectTexture( unsigned char *pixels, int width, int height );
	void					UpdateIndirectTexture( unsigned char *pixels, int width, int height );
	void					UpdateDirectTexture( float *pixels, int width, int height );
	void					UpdateIndirectTexture( float *pixels, int width, int height );

	int						GetTotalSamples( void ) const;
	const sRadiositySample&	GetSample( int index ) const;
	void					SetInterpolate( bool value ) { interpolate = value; }
	bool					IsInterpolate( void ) const { return interpolate; }

	// ** Precompute
	int						ComputeFormFactors( int maxFormFactors = 0 );
	void					Save( const char *fileName );
	bool					Load( const char *fileName );
};

#endif	/*	!__Radiosity_H__	*/