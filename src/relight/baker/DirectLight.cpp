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
        Color        influence;

        if( light->vertexGenerator() ) {
            influence = lightFromPointSet( lumel, light );
        } else {
            influence = lightFromPoint( lumel, light );
        }

        lumel.m_color += influence;
    }
}

// ** DirectLight::lightFromPoint
Color DirectLight::lightFromPoint( const Lumel& lumel, const Light* light ) const
{
    float influence = influenceFromPoint( lumel, light->position(), light->influence(), light->cutoff(), light->attenuation(), light->castsShadow() );

    if( influence > 0.0f ) {
        return light->color() * light->intensity() * influence;
    }

    return Color( 0, 0, 0 );
}

// ** DirectLight::lightFromPointSet
Color DirectLight::lightFromPointSet( const Lumel& lumel, const Light* light ) const
{
    LightVertexGenerator* vertexGenerator = light->vertexGenerator();

    // ** No light vertices generated - just exit
    if( vertexGenerator->vertexCount() == 0 ) {
        return Color( 0, 0, 0 );
    }

    const LightVertexBuffer& vertices = vertexGenerator->vertices();
    Color                    color = Color( 0, 0, 0 );

    for( int i = 0, n = vertexGenerator->vertexCount(); i < n; i++ ) {
        const LightVertex&  vertex    = vertices[i];
        float               influence = influenceFromPoint( lumel, vertex.m_position + light->position(), light->influence(), light->cutoff(), light->attenuation(), light->castsShadow() );

        // ** We have a non-zero light influence - add a light color to final result
        if( influence > 0.0f ) {
            color += light->color() * light->intensity() * influence;
        }
    }

    return color / vertexGenerator->vertexCount();
}

// ** DirectLight::influenceFromPoint
float DirectLight::influenceFromPoint( const Lumel& lumel, const Vec3& point, LightInfluence* influence, LightCutoff* cutoff, LightAttenuation* attenuation, bool castsShadow ) const
{
    float inf       = 0.0f;
    float att       = 1.0f;
    float cut       = 1.0f;
    float distance  = 0.0f;

    // ** Calculate light influence.
    if( influence ) {
        inf = influence->calculate( m_scene->tracer(), point, lumel.m_position, lumel.m_normal, distance );
    }

    // ** Calculate light cutoff.
    if( cutoff ) {
        cut = cutoff->calculate( lumel.m_position );
    }

    // ** Calculate light attenuation
    if( attenuation ) {
        att = attenuation->calculate( distance );
    }

    // ** Calculate final influence
    inf = inf * att * cut;
    if( inf <= 0.001f ) {
        return 0.0f;
    }
    
    return inf;
}

} // namespace bake

} // namespace relight
