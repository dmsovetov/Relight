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

#include "Photons.h"
#include "DirectLight.h"

#include "../Scene.h"
#include "../Light.h"
#include "../Mesh.h"
#include "../Lightmap.h"
#include "../rt/Tracer.h"

namespace relight {

namespace bake {

// ** Photons::Photons
Photons::Photons( const Scene* scene, int passCount, int maxDepth, float energyThreshold, float reflectionRadius )
    : Baker( scene ), m_passCount( passCount ), m_maxDepth( maxDepth ), m_energyThreshold( energyThreshold ), m_reflectionRadius( reflectionRadius )
{

}

// ** Photons::bake
RelightStatus Photons::bake( void )
{
    for( int j = 0; j < m_passCount; j++ ) {
        for( int i = 0, n = m_scene->lightCount(); i < n; i++ ) {
            const Light* light = m_scene->light( i );

            if( !light->photonEmitter() ) {
                continue;
            }

            emitPhotons( light );
        }

        printf( "Tracing photons %d%%\n", int( float( j ) / m_passCount * 100 ) );
    }

    return RelightSuccess;
}

// ** Photons::emitPhotons
void Photons::emitPhotons( const Light* light )
{
    PhotonEmitter*  emitter = light->photonEmitter();

    for( int i = 0, n = emitter->photonCount(); i < n; i++ ) {
        trace( light->position(), emitter->emit(), light->color(), light->intensity(), 0 );
    }
}

// ** Photons::trace
void Photons::trace( const Vec3& position, const Vec3& direction, const Color& color, float energy, int depth )
{
    // ** Maximum depth or energy threshold exceeded
    if( depth > m_maxDepth || energy < m_energyThreshold ) {
        return;
    }

    rt::Hit hit;

    // ** The photon didn't hit anything
    if( !m_scene->tracer()->traceSegment( position, position + direction * m_reflectionRadius, &hit ) ) {
        return;
    }

    // ** Store photons with depth more that 0, because zero depth means that it's a direct light.
    if( depth > 0 ) {
        float attenuation = std::max( 1.0f - (position - hit.m_point).length() / m_reflectionRadius, 0.0f );
        float intensity   = DirectLight::lambert( -direction, hit.m_normal );

        store( hit.m_instance->photonmap(), hit.m_color * color * intensity, hit.m_uv );

        energy *= intensity;
    }

    // ** Keep tracing
    trace( hit.m_point, Vec3::randomHemisphereDirection( hit.m_point, hit.m_normal ), hit.m_color * color, energy, depth + 1 );
}

// ** Photons::store
void Photons::store( Photonmap* photonmap, const Color& color, const Uv& uv )
{
    if( !photonmap ) {
        return;
    }
    
    Lumel& lumel = photonmap->lumel( uv );
    if( !lumel ) {
        return;
    }

    lumel.m_color += color;
    lumel.m_photons++;
}

} // namespace bake

} // namespace relight