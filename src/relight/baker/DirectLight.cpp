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
#include "../rt/Tracer.h"
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

        switch( light->type() ) {
        case PointLightType:    influence = lightFromPoint( lumel.m_position, lumel.m_normal, static_cast<const PointLight*>( light ) );
                                break;
        case MeshLightType:     influence = lightFromMesh( lumel.m_position, lumel.m_normal, static_cast<const MeshLight*>( light ) );
                                break;
        default:                assert( false );
        }

        lumel.m_color += influence;
    }
}

// ** DirectLight::lambert
float DirectLight::lambert( const Vec3& direction, const Vec3& normal )
{
    float dp = direction * normal;
    return dp < 0.0f ? 0.0f : dp;
}

// ** DirectLight::lightFromPoint
Color DirectLight::lightFromPoint( const Vec3& position, const Vec3& normal, const PointLight* light ) const
{
    Vec3  direction;
    float influence = influenceFromPoint( direction, position, normal, light->position(), light->attenuation(), light->castsShadow() );

    if( influence > 0.0f ) {
        return light->color() * light->intensity() * influence;
    }

    return Color( 0, 0, 0 );
}

// ** DirectLight::lightFromMesh
Color DirectLight::lightFromMesh( const Vec3& position, const Vec3& normal, const MeshLight* light ) const
{
    const Mesh*           mesh            = light->mesh();
    LightVertexGenerator* vertexGenerator = light->vertexGenerator();

    // ** No light vertices generated - generate them now
    if( vertexGenerator->vertexCount() == 0 ) {
        vertexGenerator->generate();
        printf( "%d light vertices generated\n", vertexGenerator->vertexCount() );
    }

    // ** No light vertices generated - just exit
    if( vertexGenerator->vertexCount() == 0 ) {
        return Color( 0, 0, 0 );
    }

    const LightVertexBuffer& vertices = vertexGenerator->vertices();
    Color                    color = Color( 0, 0, 0 );
    Vec3                     direction;

    for( int i = 0, n = vertexGenerator->vertexCount(); i < n; i++ ) {
        const LightVertex&  vertex    = vertices[i];
        float               influence = influenceFromPoint( direction, position, normal, vertex.m_position + light->position(), light->attenuation(), light->castsShadow() );

        if( light->isHemisphere() ) {
            // ** Multiply an indluence by a dot product with vertex normal and direction point
            influence *= max( direction * vertex.m_normal, 0.0f );
        }

        // ** We have a non-zero light influence - add a light color to final result
        if( influence > 0.0f ) {
            color += light->color() * light->intensity() * influence;
        }
    }

    return color / vertexGenerator->vertexCount();

}

// ** DirectLight::influenceFromPoint
float DirectLight::influenceFromPoint( Vec3& direction, const Vec3& position, const Vec3& normal, const Vec3& point, LightAttenuation* attenuation, bool castsShadow ) const
{
    direction       = point - position;
    float distance  = direction.normalize();
    float att       = 1.0f;

    // ** Calculate light attenuation
    if( attenuation ) {
        att = attenuation->calculate( distance );
    }

    // ** Calculate influence intensity
    float influence = lambert( direction, normal ) * att;
    if( influence <= 0.001f ) {
        return 0.0f;
    }

    // ** Calculate shadow
    if( castsShadow ) {
        influence *= m_scene->tracer()->test( position, point ) ? 0.0f : 1.0f;
    }

    return influence;
}

} // namespace bake

} // namespace relight
