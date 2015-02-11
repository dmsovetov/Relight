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
#include "../scene/Scene.h"
#include "../scene/Light.h"
#include "../Lightmap.h"

namespace relight {

namespace bake {

// ** DirectLight::DirectLight
DirectLight::DirectLight( const Scene* scene, Progress* progress ) : Baker( scene, progress )
{

}

// ** DirectLight::bakeLumel
void DirectLight::bakeLumel( Lumel& lumel )
{
    for( int l = 0, n = m_scene->lightCount(); l < n; l++ ) {
        const Light* light = m_scene->light( l );
        Color        influence;

        switch( light->type() ) {
        case LightPoint:    influence = lightFromSource( lumel.m_position, lumel.m_normal, static_cast<const PointLight*>( light ) );
                            break;
        default:            assert( false );
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

// ** DirectLight::lightFromSource
Color DirectLight::lightFromSource( const Vec3& position, const Vec3& normal, const PointLight* light ) const
{
    Vec3  dir           = light->position() - position;
    float distance      = dir.normalize();
    float attenuation   = 1.0f;

    // ** Calculate light attenuation
    if( const LightAttenuation* att = light->attenuation() ) {
        attenuation = att->calculate( light, distance );
    }

    // ** Calculate influence intensity
    float intensity = lambert( dir, normal ) * attenuation;
    if( intensity <= 0.001f ) {
        return Color( 0.0f, 0.0f, 0.0f );
    }

    // ** Calculate shadow
    if( light->castsShadow() ) {
        intensity *= m_scene->tracer()->traceSegment( position, light->position() ) ? 0.0f : 1.0f;
    }

    return light->color() * intensity;
}

} // namespace bake

} // namespace relight
