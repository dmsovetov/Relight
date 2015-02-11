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

#include "BuildCheck.h"

#include "Light.h"

namespace relight {

// ** Light::Light
Light::Light( LightType type ) : m_type( type ), m_intensity( 0.0f ), m_castsShadow( false ), m_attenuation( NULL ), m_photonEmitter( NULL )
{

}

Light::~Light( void )
{
    delete m_attenuation;
    delete m_photonEmitter;
}

// ** Light::type
LightType Light::type( void ) const
{
    return m_type;
}

// ** Light::attenuation
LightAttenuation* Light::attenuation( void ) const
{
    return m_attenuation;
}

// ** Light::setAttenuation
void Light::setAttenuation( LightAttenuation* value )
{
    delete m_attenuation;
    m_attenuation = value;
}

// ** Light::photonEmitter
PhotonEmitter* Light::photonEmitter( void ) const
{
    return m_photonEmitter;
}

// ** Light::setPhotonEmitter
void Light::setPhotonEmitter( PhotonEmitter* value )
{
    delete m_photonEmitter;
    m_photonEmitter = value;
}

// ** Light::position
const Vec3& Light::position( void ) const
{
    return m_position;
}

// ** Light::setPosition
void Light::setPosition( const Vec3& value )
{
    m_position = value;
}

// ** Light::color
const Color& Light::color( void ) const
{
    return m_color;
}

// ** Light::setColor
void Light::setColor( const Color& value )
{
    m_color = value;
}

// ** Light::intensity
float Light::intensity( void ) const
{
    return m_intensity;
}

// ** Light::setIntensity
void Light::setIntensity( float value )
{
    m_intensity = value;
}

// ** Light::castsShadow
bool Light::castsShadow( void ) const
{
    return m_castsShadow;
}

// ** Light::setCastsShadow
void Light::setCastsShadow( bool value )
{
    m_castsShadow = value;
}

// ---------------------------------------------------------- PointLight ---------------------------------------------------------- //

// ** PointLight::PointLight
PointLight::PointLight( void ) : Light( LightPoint )
{

}

// ** PointLight::setRadius
void PointLight::setRadius( float value )
{
    m_radius = value;
}

// ** PointLight::radius
float PointLight::radius( void ) const
{
    return m_radius;
}

// ** PointLight::create
PointLight* PointLight::create( const Vec3& position, float radius, const Color& color, float intensity, bool castsShadow )
{
    PointLight* light = new PointLight;

    light->setAttenuation( new LinearLightAttenuation );
    light->setPhotonEmitter( new PhotonEmitter( light ) );
    light->setCastsShadow( castsShadow );
    light->setPosition( position );
    light->setColor( color );
    light->setIntensity( intensity );
    light->setRadius( radius );
    
    return light;
}

// -------------------------------------------------------- PhotonEmitter --------------------------------------------------------- //

// ** PhotonEmitter::PhotonEmitter
PhotonEmitter::PhotonEmitter( const Light* light ) : m_light( light )
{

}

// ** PhotonEmitter::photonCount
int PhotonEmitter::photonCount( void ) const
{
    float area = 0.05f;
    return (area * 100) * (area * 100) * m_light->intensity() * 1000;
}

// ** PhotonEmitter::emit
Vec3 PhotonEmitter::emit( void ) const
{
    return Vec3::randomDirection();
}

// ---------------------------------------------------- LinearLightAttenuation ---------------------------------------------------- //

// ** LinearLightAttenuation::calculate
float LinearLightAttenuation::calculate( const Light* light, float distance ) const
{
    float att = 1.0f;

    if( light->type() == LightPoint ) {
        att = 1.0f - (distance / static_cast<const PointLight*>( light )->radius());
    }


    return att < 0.0f ? 0.0f : att;
}

} // namespace relight