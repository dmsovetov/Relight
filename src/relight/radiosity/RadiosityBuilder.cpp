/**************************************************************************

 The MIT License (MIT)

 Copyright (c) 2015 Dmitry Sovetov

 https://github.com/dmsovetov

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

 **************************************************************************/

#include "../BuildCheck.h"

#include "RadiosityBuilder.h"
#include "../scene/Scene.h"
#include "../scene/Mesh.h"
#include "../rt/Tracer.h"
#include "../Lightmap.h"

namespace relight {

// ** RadiosityBuilder::RadiosityBuilder
RadiosityBuilder::RadiosityBuilder( Scene* scene ) : m_scene( scene ), m_formFactorThreshold( 0.1f ), m_maxFormFactors( 0 ), m_totalFormFactors( 0 )
{

}

// ** RadiosityBuilder::build
Radiosity RadiosityBuilder::build( f32 formFactorThreshold, s32 maxFormFactors )
{
	m_maxFormFactors	  = maxFormFactors;
	m_formFactorThreshold = formFactorThreshold;
	m_totalFormFactors	  = 0;

	// ** Create patches
	Radiosity radiosity;
	createPatches( radiosity );

	// ** Compute form factors.
	computeFormFactors( radiosity );

	return radiosity;
}

// ** RadiosityBuilder::createPatches
void RadiosityBuilder::createPatches( Radiosity& radiosity )
{
	for( s32 i = 0, n = m_scene->meshCount(); i < n; i++ ) {
		// ** Get the mesh by index.
		const Mesh* mesh = m_scene->mesh( i );

		// ** Get the radiance map from mesh.
		Radiancemap* map = mesh->radiancemap();
		DC_BREAK_IF( map == NULL );

		// ** Create a patch for each map pixel.
		s32 w = map->width();
		s32 h = map->height();

		// ** Register the patches.
		Radiosity::Patches patches;

		// ** Initialize each patch.
		for( s32 y = 0; y < h; y++ ) {
			for( s32 x = 0; x < w; x++ ) {
				// ** Get the corresponding map pixel.
				const Lumel& lumel = map->lumel( x, y );

				// ** Skip empty pixels.
				if( !lumel ) {
					continue;
				}

				patches.push_back( Radiosity::Patch( mesh, &lumel ) );
			}
		}

		// ** Add mesh patches to radiosity.
		radiosity.addPatches( mesh, patches );
	}
}

// ** RadiosityBuilder::refineFormFactors
void RadiosityBuilder::refineFormFactors( Radiosity::Patch& sample )
{
	if( sample.m_ff.empty() ) {
		return;
	}

	if( m_maxFormFactors ) {
		while( sample.m_ff.size() > m_maxFormFactors ) {
			sample.m_ff.erase( sample.m_ff.begin() + rand() % sample.m_ff.size() );
		}
	}

	// ** Normalize weights
	f32 weightSum = 0.0f;

	for( int i = 0, n = sample.m_ff.size(); i < n; i++ ) {
		weightSum += sample.m_ff[i].m_weight;
	}

	for( int i = 0, n = sample.m_ff.size(); i < n; i++ ) {
		sample.m_ff[i].m_weight /= weightSum;
	}

	m_totalFormFactors += sample.m_ff.size();
}

// ** RadiosityBuilder::ComputeFormFactors
void RadiosityBuilder::computeFormFactors( Radiosity& radiosity )
{
	s32 n = radiosity.patchCount();

	for( s32 i = 0; i < n; i++ ) {
		Radiosity::Patch& patch = radiosity.patch( i );

		for( s32 j = i + 1; j < n; j++ ) {
			distanceFormFactor( m_scene, patch, radiosity.patch( j ), m_formFactorThreshold );
		}

		refineFormFactors( patch );
	}
}

// ** RadiosityBuilder::distanceFormFactor
void RadiosityBuilder::distanceFormFactor( Scene* scene, Radiosity::Patch& receiver, Radiosity::Patch& sender, f32 threshold )
{
	const Vec3& sPos = sender.m_lumel->m_position;
	const Vec3& rPos = receiver.m_lumel->m_position;
	const Vec3& sN	 = sender.m_lumel->m_normal;
	const Vec3& rN	 = receiver.m_lumel->m_normal;

	Vec3 dir = sPos - rPos;

	// ** Intensity
	f32 r		= dir.normalize();
	f32 cosBA	= dir * rN;
	f32 cosAB   = -(dir * sN);

	if( r == 0.0f || cosAB < 0.0f || cosBA < 0.0f ) {
		return;
	}

	f32 wBA = ((cosBA * cosAB) / (Pi * r * r));
	f32 wAB = ((cosAB * cosBA) / (Pi * r * r));

	if( wBA < threshold && wAB < threshold ) {
		return;
	}

	// ** Trace scene
	rt::Hit hit = scene->tracer()->traceSegment( rPos, sPos );
	if( hit ) {
		return;
	}

	// ** Push a form-factor
	if( wBA >= threshold ) receiver.m_ff.push_back( Radiosity::FormFactor( &sender, wBA ) );
	if( wAB >= threshold ) sender.m_ff.push_back( Radiosity::FormFactor( &receiver, wAB ) );
}

// ** RadiosityBuilder::distanceSqAreaFormFactor
void RadiosityBuilder::distanceSqAreaFormFactor( Scene* scene, Radiosity::Patch& receiver, Radiosity::Patch& sender, f32 threshold )
{
	const Vec3& sPos = sender.m_lumel->m_position;
	const Vec3& rPos = receiver.m_lumel->m_position;
	const Vec3& sN	 = sender.m_lumel->m_normal;
	const Vec3& rN	 = receiver.m_lumel->m_normal;

	Vec3 dir = sPos - rPos;
	f32  r   = dir.normalize();

	if( r == 0.0f ) {
		return;
	}

	f32 Ai =   dir * rN;
	f32 Aj = -(dir * sN);

	f32 wBA = (Ai * Aj) / (Pi * r * r);
	f32 wAB = (Ai * Aj) / (Pi * r * r);

	if( wBA < threshold && wAB < threshold ) {
		return;
	}

	// ** Trace scene
	rt::Hit hit = scene->tracer()->traceSegment( rPos, sPos );
	if( hit ) {
		return;
	}

	// ** Push a form-factor
	if( wBA >= threshold ) receiver.m_ff.push_back( Radiosity::FormFactor( &sender, wBA ) );
	if( wAB >= threshold ) sender.m_ff.push_back( Radiosity::FormFactor( &receiver, wAB ) );
}

} // namespace relight