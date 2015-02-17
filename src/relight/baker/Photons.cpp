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

#include "../scene/Scene.h"
#include "../scene/Light.h"
#include "../scene/Mesh.h"
#include "../Lightmap.h"
#include "../rt/Tracer.h"

namespace relight {

namespace bake {

// ** Photons::Photons
Photons::Photons( const Scene* scene, int passCount, int maxDepth, float energyThreshold, float maxDistance )
    : m_scene( scene ), m_passCount( passCount ), m_maxDepth( maxDepth ), m_energyThreshold( energyThreshold ), m_maxDistance( maxDistance ), m_photonCount( 0 )
{

}

// ** Photons::emit
RelightStatus Photons::emit( void )
{
    for( int j = 0; j < m_passCount; j++ ) {
        for( int i = 0, n = m_scene->lightCount(); i < n; i++ ) {
            const Light* light = m_scene->light( i );

            if( !light->photonEmitter() ) {
                continue;
            }

            emitPhotons( light );
        }
    }

    printf( "%d photons stored\n", m_photonCount );

    return RelightSuccess;
}

// ** Photons::emitPhotons
void Photons::emitPhotons( const Light* light )
{
    PhotonEmitter* emitter   = light->photonEmitter();
    Vec3           direction;
    Vec3           position;

    for( int i = 0, n = emitter->photonCount(); i < n; i++ ) {
        // ** Emit photon
        emitter->emit( m_scene, position, direction );

        // ** Calculate light cutoff
        float cut = 1.0f;

        if( const LightCutoff* cutoff = light->cutoff() ) {
            cut = cutoff->cutoffForDirection( direction );
        }

        if( cut <= 0.0f ) {
            continue;
        }

        // ** Trace photon
        trace( light->attenuation(), position, direction, light->color() * light->intensity() * cut, 0 );
    }
}

// ** Photons::trace
void Photons::trace( const LightAttenuation* attenuation, const Vec3& position, const Vec3& direction, const Color& color, int depth )
{
    // ** Maximum depth or energy threshold exceeded
    if( depth > m_maxDepth || color.luminance() < m_energyThreshold ) {
        return;
    }

    rt::Hit hit;

    // ** The photon didn't hit anything
    if( !m_scene->tracer()->traceSegment( position, position + direction * m_maxDistance, &hit ) ) {
        return;
    }

    // ** Energy attenuation after a photon has passed the traced segment
    float att = 1.0f;
    if( attenuation ) {
        attenuation->calculate( (position - hit.m_point).length() );
    }

    // ** Energy after reflection
    float influence = LightInfluence::lambert( -direction, hit.m_normal ) * att;

    // ** Final photon color
    Color hitColor = color * hit.m_color * influence;

    // ** Store photon energy
    store( hit.m_mesh->photonmap(), hitColor, hit.m_uv );

    // ** Keep tracing
    trace( attenuation, hit.m_point, Vec3::randomHemisphereDirection( hit.m_point, hit.m_normal ), hitColor, depth + 1 );
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

    m_photonCount++;
}

} // namespace bake

} // namespace relight