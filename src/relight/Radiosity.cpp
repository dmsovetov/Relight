//
//  Filename:	Radiosity.cpp
//	Created:	01:06:2012   12:08

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"Radiosity.h"
#include	"Lightmap.h"
#include	"DirectLightCalculator.h"

#define		DUAL_FF				(1)
#define		FF_THRESHOLD		(0.01f)
#define		FF_PI				(3.1415926535897932384626433832795f)

/*
=========================================================================================

			CODE

=========================================================================================
*/

// ** cRadiositySolver::cRadiositySolver
cRadiositySolver::cRadiositySolver( IRayTracer *_model )
{
	model			 = _model;
	interpolate		 = true;
}

// ** cRadiositySolver::Create
void cRadiositySolver::Create( cLightmap *indirect )
{
	this->indirect = indirect;

	int w = indirect->GetWidth();
	int h = indirect->GetHeight();
	sLumel *lumels = indirect->GetLumels();

	samples		 = new sRadiositySample[w * h];
	totalSamples = w * h;

	for( int j = 0; j < h; j++ ) {
		for( int i = 0; i < w; i++ ) {
			sLumel& ind = lumels[j * w + i];

			if( !ind.IsValid() ) {
				continue;
			}

			sRadiositySample& sample = samples[j * w + i];

			sample.position			= ind.position;
			sample.diffuse			= ind.color;
			sample.normal			= ind.normal;
			sample.isValid			= true;
			sample.indirectLumel	= &ind;
		}
	}
}

// ** cRadiositySolver::InterpolateSample
void cRadiositySolver::InterpolateSample( sRadiositySample& sample, unsigned short a, unsigned short b )
{
	if( !sample.isValid ) {
		return;
	}

	const sRadiositySample& sampleA = samples[a];
	const sRadiositySample& sampleB = samples[b];

	if( !sampleA.isValid )		sample.indirect = sampleB.indirect;
	else if( !sampleB.isValid ) sample.indirect = sampleA.indirect;
	else						sample.indirect = (sampleA.indirect + sampleB.indirect) * 0.5f;

	sample.indirectLumel->color = sample.indirect;
}

// ** cRadiositySolver::Solve
void cRadiositySolver::Solve( void )
{
	if( interpolate ) {
		int w = indirect->GetWidth();
		int h = indirect->GetHeight();

		for( int i = 0; i < totalSamples; i++ ) {
			samples[i].indirect = sColor( 0.0f, 0.0f, 0.0f );
		}

		for( int y = 0; y < h - 1; y += 2 ) {
			for( int x = 0; x < w - 1; x += 2 ) {
				sRadiositySample& sample = samples[y * w + x];
				if( !sample.isValid ) {
					continue;
				}

				for( int i = 0, n = sample.formFactors.size(); i < n; i++ ) {
					sFormFactor&			formFactor	= sample.formFactors[i];
					const sRadiositySample& other		= samples[formFactor.sample];

					sample.indirect.r += sample.diffuse.r * other.injected.r * formFactor.weight;
					sample.indirect.g += sample.diffuse.g * other.injected.g * formFactor.weight;
					sample.indirect.b += sample.diffuse.b * other.injected.b * formFactor.weight;
				}

				sample.indirectLumel->color = sample.indirect;
			}
		}

		// ** Interpolate
		for( int y = 1; y < h - 2; y += 2 ) {
			for( int x = 1; x < w - 2; x += 2 ) {
				InterpolateSample( samples[(y - 1) * w + x], (y - 1) * w + x - 1, (y - 1) * w + x + 1 );
				InterpolateSample( samples[(y + 1) * w + x], (y + 1) * w + x - 1, (y + 1) * w + x + 1 );

				InterpolateSample( samples[y * w + (x - 1)], (y + 1) * w + (x - 1), (y - 1) * w + (x - 1) );
				InterpolateSample( samples[y * w + (x + 1)], (y + 1) * w + (x + 1), (y - 1) * w + (x + 1) );

				InterpolateSample( samples[y * w + x], (y + 1) * w + x, (y - 1) * w + x );
			}
		}
	} else {
		for( int i = 0; i < totalSamples; i++ ) {
			sRadiositySample& sample = samples[i];
			if( !sample.isValid ) {
				continue;
			}

			for( int i = 0, n = sample.formFactors.size(); i < n; i++ ) {
				sFormFactor&			formFactor	= sample.formFactors[i];
				const sRadiositySample& other		= samples[formFactor.sample];

				sample.indirect.r += sample.diffuse.r * other.injected.r * formFactor.weight;
				sample.indirect.g += sample.diffuse.g * other.injected.g * formFactor.weight;
				sample.indirect.b += sample.diffuse.b * other.injected.b * formFactor.weight;
			}

			sample.indirectLumel->color = sample.indirect;
		}
	}
}

// ** cRadiositySolver::Clear
void cRadiositySolver::Clear( void )
{
	for( int i = 0; i < totalSamples; i++ ) {
		sRadiositySample& sample = samples[i];
		if( !sample.isValid ) {
			continue;
		}

		sample.indirect = sColor( 0.0f, 0.0f, 0.0f );
		sample.injected = sColor( 0.0f, 0.0f, 0.0f );
	}
}

// ** cRadiositySolver::InjectPointLight
void cRadiositySolver::InjectPointLight( float x, float y, float z, float r, float g, float b, float intensity, float radius )
{
	sVector3 L  = sVector3( x, y, z );
	sColor	 Lc = sColor( r, g, b );

	for( int i = 0; i < totalSamples; i++ ) {
		sRadiositySample& sample = samples[i];
		if( !sample.isValid ) {
			continue;
		}

		// ** Light intensity
		sVector3 dir		= L - sample.position; dir.normalize();
		float intensity		= cDirectLightCalculator::CalculateLambertIntensity( sample.normal, dir );
		float attenuation	= cDirectLightCalculator::CalculateLightAttenuation( sample.position, L, intensity, radius );

		intensity = intensity * attenuation;
		if( intensity < 0.01f ) {
			continue;
		}

		// ** Trace scene
		sRayTraceResult result;
		if( model->TraceRay( sample.position + sample.normal * 0.01, L, &result ) >= 0 ) {
			continue;
		}

		sample.injected	+= Lc * sample.diffuse * intensity;
	}
}

// ** cRadiositySolver::InjectIndirect
void cRadiositySolver::InjectIndirect( void )
{
	for( int i = 0; i < totalSamples; i++ ) {
		sRadiositySample& sample = samples[i];
		if( !sample.isValid ) {
			continue;
		}

		sample.injected = sample.injected + sample.indirectLumel->color;
	}
}

// ** cRadiositySolver::UpdateDirectTexture
void cRadiositySolver::UpdateDirectTexture( unsigned char *pixels, int width, int height )
{
	for( int j = 0; j < height; j++ ) {
		for( int i = 0; i < width; i++ ) {
			unsigned char			*pixel = &pixels[j * width * 3 + i * 3];
			const sRadiositySample&	sample = samples[j * width + i];

			if( !sample.isValid ) {
				continue;
			}

			pixel[0] = sample.injected.r * 255.0f;
			pixel[1] = sample.injected.g * 255.0f;
			pixel[2] = sample.injected.b * 255.0f;
		}
	}
}

// ** cRadiositySolver::UpdateIndirectTexture
void cRadiositySolver::UpdateIndirectTexture( unsigned char *pixels, int width, int height )
{
	for( int j = 0; j < height; j++ ) {
		for( int i = 0; i < width; i++ ) {
			unsigned char			*pixel = &pixels[j * width * 3 + i * 3];
			const sRadiositySample&	sample = samples[j * width + i];

			if( !sample.isValid ) {
				continue;
			}

			pixel[0] = sample.indirectLumel->color.r * 255.0f;
			pixel[1] = sample.indirectLumel->color.g * 255.0f;
			pixel[2] = sample.indirectLumel->color.b * 255.0f;
		}
	}
}

// ** cRadiositySolver::UpdateDirectTexture
void cRadiositySolver::UpdateDirectTexture( float *pixels, int width, int height )
{
	for( int j = 0; j < height; j++ ) {
		for( int i = 0; i < width; i++ ) {
			float					*pixel = &pixels[j * width * 3 + i * 3];
			const sRadiositySample&	sample = samples[j * width + i];

			if( !sample.isValid ) {
				continue;
			}

			pixel[0] = sample.injected.r;
			pixel[1] = sample.injected.g;
			pixel[2] = sample.injected.b;
		}
	}
}

// ** cRadiositySolver::UpdateIndirectTexture
void cRadiositySolver::UpdateIndirectTexture( float *pixels, int width, int height )
{
	indirect->Expand();

	const sLumel *lumels = indirect->GetLumels();

	for( int j = 0; j < height; j++ ) {
		for( int i = 0; i < width; i++ ) {
			const sLumel& lumel = lumels[j * width + i];
			float *pixel		= &pixels[j * width * 3 + i * 3];

			pixel[0] = lumel.color.r;
			pixel[1] = lumel.color.g;
			pixel[2] = lumel.color.b;
		}
	}
}

// ** cRadiositySolver::RefineFormFactors
void cRadiositySolver::RefineFormFactors( sRadiositySample& sample, int maxFormFactors )
{
	if( sample.formFactors.empty() ) {
		return;
	}

	if( maxFormFactors ) {
		while( sample.formFactors.size() > maxFormFactors ) {
			sample.formFactors.erase( sample.formFactors.begin() + rand() % sample.formFactors.size() );
		}
	}

	// ** Normalize weights
	float weightSum = 0.0f;
	for( int i = 0, n = sample.formFactors.size(); i < n; i++ ) {
		weightSum += sample.formFactors[i].weight;
	}

	for( int i = 0, n = sample.formFactors.size(); i < n; i++ ) {
		sample.formFactors[i].weight /= weightSum;
	}

	totalFormFactors += sample.formFactors.size();
}

// ** cRadiositySolver::ComputeFormFactors
int cRadiositySolver::ComputeFormFactors( int maxFormFactors )
{
//	if( Load( "output/radiosity" ) ) {
//		return 0;
//	}

	totalFormFactors = 0;

	for( int i = 0; i < totalSamples; i++ ) {
		sRadiositySample& sample = samples[i];
		if( !sample.isValid ) {
			continue;
		}

		int j = 0;
	#if DUAL_FF
		j = i;
	#endif
		for( ; j < totalSamples; j++ ) {
			if( i == j ) {
				continue;
			}

			DistanceFormFactor( i, j );
		//	DistanceSqAreaFormFactor( i, j );
		}

		RefineFormFactors( sample, maxFormFactors );

		printf( "Form factor %d%%\r", int( float( i ) / totalSamples * 100 ) );
	}

	Save( "output/radiosity" );

	return totalFormFactors;
}

// ** cRadiositySolver::DistanceFormFactor
void cRadiositySolver::DistanceFormFactor( unsigned short receiverIndex, unsigned short senderIndex )
{
	sRadiositySample& receiver	= samples[receiverIndex];
	sRadiositySample& sender	= samples[senderIndex];
	if( !sender.isValid ) {
		return;
	}

	sVector3 dir = sender.position - receiver.position;

	// ** Intensity
	float r		= dir.normalize();
	float cosBA	=   dir * receiver.normal;
	float cosAB = -(dir * sender.normal);

	if( cosAB < 0.0f || cosBA < 0.0f ) return;

	float wBA = ((cosBA * cosAB) / (FF_PI * r * r));
	float wAB = ((cosAB * cosBA) / (FF_PI * r * r));

	if( wBA < FF_THRESHOLD && wAB < FF_THRESHOLD ) {
		return;
	}

	// ** Trace scene
	sRayTraceResult result;
	if( model->TraceRay( receiver.position, sender.position, &result ) >= 0 ) {
		return;
	}

	if( wBA >= FF_THRESHOLD ) receiver.formFactors.push_back( sFormFactor( senderIndex, wBA ) );
	if( wAB >= FF_THRESHOLD ) sender.formFactors.push_back( sFormFactor( receiverIndex, wAB ) );
}

// ** cRadiositySolver::DistanceSqAreaFormFactor
void cRadiositySolver::DistanceSqAreaFormFactor( unsigned short receiverIndex, unsigned short senderIndex )
{
	sRadiositySample& receiver	= samples[receiverIndex];
	sRadiositySample& sender	= samples[senderIndex];
	if( !sender.isValid ) {
		return;
	}

	sVector3 dir = sender.position - receiver.position;
	float r = dir.normalize();

	if( !r ) return;

	float Ai =   dir * receiver.normal;
	float Aj = -(dir * sender.normal);

	float wBA = (Ai * Aj) / (3.141592653f * r * r);
	float wAB = (Ai * Aj) / (3.141592653f * r * r);
	if( wBA < FF_THRESHOLD && wAB < FF_THRESHOLD ) {
		return;
	}

	// ** Trace scene
	sRayTraceResult result;
	if( model->TraceRay( receiver.position, sender.position, &result ) >= 0 ) {
		return;
	}

	if( wBA >= FF_THRESHOLD ) receiver.formFactors.push_back( sFormFactor( senderIndex, wBA ) );
	if( wAB >= FF_THRESHOLD ) sender.formFactors.push_back( sFormFactor( receiverIndex, wAB ) );

/*
	float Ai =   dir * receiver.normal;
	float Aj = -(dir * sender.normal);

	if( Ai <= 0.0f || Aj <= 0.0f ) {
		return;
	}

	float weight = (Ai * Aj) / 3.14159265358979323846f * (r * r);

	// ** Trace scene
	sVector3 start = receiver.position + dir * 0.01;
	sRayTraceResult result;
	if( model->TraceRay( start, sender.position, &result ) >= 0 ) {
		return;
	}

	if( weight > 0.01f ) {
		receiver.formFactors.push_back( sFormFactor( senderIndex, weight ) );
	}*/
}

// ** cRadiositySolver::GetTotalSamples
int cRadiositySolver::GetTotalSamples( void ) const {
	return totalSamples;
}

// ** cRadiositySolver::GetSample
const sRadiositySample& cRadiositySolver::GetSample( int index ) const {
	return samples[index];
}

// ** cRadiositySolver::Save
void cRadiositySolver::Save( const char *fileName )
{
	FILE *file = fopen( fileName, "wt" );

	for( int i = 0; i < totalSamples; i++ ) {
		sRadiositySample& sample = samples[i];
		if( !sample.isValid ) {
			continue;
		}

		int count = sample.formFactors.size();

		fwrite( &i, sizeof( int ), 1, file );
		fwrite( &count, sizeof( int ), 1, file );
		for( int j = 0; j < count; j++ ) {
			fwrite( &sample.formFactors[j].sample, sizeof( unsigned short ), 1, file );
			fwrite( &sample.formFactors[j].weight, sizeof( float ), 1, file );
		}
	}

	fclose( file );
}

// ** cRadiositySolver::Load
bool cRadiositySolver::Load( const char *fileName )
{
	FILE *file = fopen( fileName, "rt" );
	if( !file ) {
		return false;
	}

	while( !feof( file ) ) {
		int count, index;

		fread( &index, sizeof( int ), 1, file );
		fread( &count, sizeof( int ), 1, file );

		for( int j = 0; j < count; j++ ) {
			sFormFactor ff;

			fread( &ff.sample, sizeof( unsigned short ), 1, file );
			fread( &ff.weight, sizeof( float ), 1, file );

			samples[index].formFactors.push_back( ff );
		}
	}

	fclose( file );
	return true;
}