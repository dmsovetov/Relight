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

#include "IndirectLight.h"
#include "../Lightmap.h"
#include "../scene/Scene.h"
#include "../scene/Mesh.h"
#include "../rt/Tracer.h"

namespace relight {

namespace bake {

// ** IndirectLight::IndirectLight
IndirectLight::IndirectLight( const Scene* scene, Progress* progress, BakeIterator* iterator, int samples, float maxDistance, int radius, const Rgb& skyColor, const Rgb& ambientColor )
    : Baker( scene, progress, iterator ), m_samples( samples ), m_maxDistance( maxDistance ), m_radius( radius ), m_skyColor( skyColor ), m_ambientColor( ambientColor )
{

}

// ** IndirectLight::bakeLumel
void IndirectLight::bakeLumel( Lumel& lumel )
{
    Rgb          gathered( 0, 0, 0 );
    rt::ITracer* tracer = m_scene->tracer();

    for( int k = 0; k < m_samples; k++ ) {
        Vec3    dir       = Vec3::randomHemisphereDirection( lumel.m_normal );

        float   influence = max2( lumel.m_normal * dir, 0.0f );
		DC_BREAK_IF( influence > 1.0f );

        rt::Hit hit       = tracer->traceSegment( lumel.m_position, lumel.m_position + dir * m_maxDistance, rt::HitUv | rt::HitNormal );

        if( !hit ) {
            gathered += m_skyColor * influence + m_ambientColor;
            continue;
        }

		if( dir * hit.m_normal >= 0.0f ) {
			continue;
		}

        if( const Photonmap* photons = hit.m_mesh->photonmap() ) {
            gathered += photons->lumel( hit.m_uv ).m_gathered * influence + m_ambientColor;
        }
    }

    lumel.m_color += gathered / static_cast<float>( m_samples );
}

} // namespace bake

} // namespace relight