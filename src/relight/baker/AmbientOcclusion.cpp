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

#include "AmbientOcclusion.h"
#include "../rt/Tracer.h"
#include "../scene/Scene.h"
#include "../Lightmap.h"

namespace relight {

namespace bake {

// ** AmbientOcclusion::AmbientOcclusion
AmbientOcclusion::AmbientOcclusion( const Scene* scene, Progress* progress, int samples, float occludedFraction, float maxDistance )
    : Baker( scene, progress ), m_samples( samples ), m_occludedFraction( occludedFraction ), m_maxDistance( maxDistance )
{

}

// ** AmbientOcclusion::bakeLumel
void AmbientOcclusion::bakeLumel( Lumel& lumel )
{
    rt::ITracer* tracer   = m_scene->tracer();
    int          occluded = 0;

    for( int i = 0; i < m_samples; i++ ) {
        Vec3 dir = Vec3::randomHemisphereDirection( lumel.m_position, lumel.m_normal );

        if( tracer->traceSegment( lumel.m_position, lumel.m_position + dir * m_maxDistance ) ) {
            occluded++;
        }
    }

    float ao = 1.0f - occluded / (m_samples * m_occludedFraction);
    lumel.m_color = Color( ao, ao, ao );
}

} // namespace bake

} // namespace relight
