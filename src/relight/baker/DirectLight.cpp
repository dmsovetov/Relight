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

#include "DirectLight.h"
#include "../scene/Mesh.h"
#include "../scene/Scene.h"
#include "../scene/Light.h"
#include "../Lightmap.h"

namespace relight {

namespace bake {

// ** DirectLight::DirectLight
DirectLight::DirectLight( const Scene* scene, Progress* progress, BakeIterator* iterator ) : Baker( scene, progress, iterator )
{

}

// ** DirectLight::bakeLumel
void DirectLight::bakeLumel( Lumel& lumel )
{
    for( int l = 0, n = m_scene->lightCount(); l < n; l++ ) {
        const Light* light = m_scene->light( l );
        Rgb          influence;

        if( light->vertexGenerator() ) {
            influence = lightFromPointSet( lumel, light );
        } else {
            influence = lightFromPoint( lumel, light );
        }

        lumel.m_color += influence;
    }
}

// ** DirectLight::lightFromPoint
Rgb DirectLight::lightFromPoint( const Lumel& lumel, const Light* light ) const
{
    float influence = influenceFromPoint( lumel, light->position(), light );

    if( influence > 0.0f ) {
        return light->color() * light->intensity() * influence;
    }

    return Rgb( 0, 0, 0 );
}

// ** DirectLight::lightFromPointSet
Rgb DirectLight::lightFromPointSet( const Lumel& lumel, const Light* light ) const
{
    LightVertexGenerator* vertexGenerator = light->vertexGenerator();

    // ** No light vertices generated - just exit
    if( vertexGenerator->vertexCount() == 0 ) {
        return Rgb( 0, 0, 0 );
    }

    const LightVertexBuffer& vertices = vertexGenerator->vertices();
    Rgb                      color    = Rgb( 0, 0, 0 );

    for( int i = 0, n = vertexGenerator->vertexCount(); i < n; i++ ) {
        const LightVertex&  vertex    = vertices[i];
        float               influence = influenceFromPoint( lumel, vertex.m_position + light->position(), light );

        // ** We have a non-zero light influence - add a light color to final result
        if( influence > 0.0f ) {
            color += light->color() * light->intensity() * influence;
        }
    }

    return color / vertexGenerator->vertexCount();
}

// ** DirectLight::influenceFromPoint
float DirectLight::influenceFromPoint( const Lumel& lumel, const Vec3& point, const Light* light ) const
{
    float inf       = 0.0f;
    float att       = 1.0f;
    float cut       = 1.0f;
    float distance  = 0.0f;

    // ** Calculate light influence.
    if( const LightInfluence* influence = light->influence() ) {
        inf = influence->calculate( m_scene->tracer(), point, lumel.m_position, lumel.m_normal, distance );
    }

    // ** Calculate light cutoff.
    if( const LightCutoff* cutoff = light->cutoff() ) {
        cut = cutoff->calculate( lumel.m_position );
    }

    // ** Calculate light attenuation
    if( const LightAttenuation* attenuation = light->attenuation() ) {
        att = attenuation->calculate( distance );
    }

    // ** Return final influence
    return inf * att * cut;
}

} // namespace bake

} // namespace relight
