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

#include "Light.h"
#include "Mesh.h"
#include "../rt/Tracer.h"

namespace relight {

// ------------------------------------------------------------ Light ------------------------------------------------------------ //

// ** Light::Light
Light::Light( void ) : m_intensity( 0.0f ), m_castsShadow( false ), m_cutoff( NULL ), m_attenuation( NULL ), m_influence( NULL ), m_vertexGenerator( NULL ), m_photonEmitter( NULL )
{

}

Light::~Light( void )
{
    delete m_cutoff;
    delete m_attenuation;
    delete m_photonEmitter;
    delete m_influence;
}

// ** Light::cutoff
LightCutoff* Light::cutoff( void ) const
{
    return m_cutoff;
}

// ** Light::setCutoff
void Light::setCutoff( LightCutoff* value )
{
    delete m_cutoff;
    m_cutoff = value;
}

// ** Light::influence
LightInfluence* Light::influence( void ) const
{
    return m_influence;
}

// ** Light::setInfluence
void Light::setInfluence( LightInfluence* value )
{
    delete m_influence;
    m_influence = value;
}

// ** Light::vertexGenerator
LightVertexGenerator* Light::vertexGenerator( void ) const
{
    return m_vertexGenerator;
}

// ** Light::setVertexGenerator
void Light::setVertexGenerator( LightVertexGenerator* value )
{
    delete m_vertexGenerator;
    m_vertexGenerator = value;
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

// ** Light::createPointLight
Light* Light::createPointLight( const Vec3& position, float radius, const Color& color, float intensity, bool castsShadow )
{
    Light* light = new Light;

    light->setInfluence( new LightInfluence( light ) );
    light->setAttenuation( new LinearLightAttenuation( light, radius ) );
    light->setPhotonEmitter( new PhotonEmitter( light ) );
    light->setCutoff( new LightCutoff( light ) );
    light->setCastsShadow( castsShadow );
    light->setPosition( position );
    light->setColor( color );
    light->setIntensity( intensity );
    
    return light;
}

// ** Light::createSpotLight
Light* Light::createSpotLight( const Vec3& position, const Vec3& direction, float cutoff, float radius, const Color& color, float intensity, bool castsShadow )
{
    Light* light = new Light;

    light->setInfluence( new LightInfluence( light ) );
    light->setAttenuation( new LinearLightAttenuation( light, radius ) );
    light->setPhotonEmitter( new PhotonEmitter( light ) );
    light->setCutoff( new LightSpotCutoff( light, direction, cutoff, 1.0f ) );
    light->setCastsShadow( castsShadow );
    light->setPosition( position );
    light->setColor( color );
    light->setIntensity( intensity );
    
    return light;
}

// ** Light::createDirectionalLight
Light* Light::createDirectionalLight( const Vec3& direction, const Color& color, float intensity, bool castsShadow )
{
    Light* light = new Light;

    light->setInfluence( new DirectionalLightInfluence( light, direction ) );
    light->setPhotonEmitter( new PhotonEmitter( light ) );
    light->setCastsShadow( castsShadow );
    light->setColor( color );
    light->setIntensity( intensity );

    return light;
}

// ** Light::create
Light* Light::createAreaLight( const Mesh* mesh, const Vec3& position, const Color& color, float intensity, bool castsShadow )
{
    Light* light = new Light;

    light->setInfluence( new LightInfluence( light ) );
    light->setAttenuation( new LinearLightAttenuation( light, mesh->bounds().volume() ) );
    light->setPhotonEmitter( new PhotonEmitter( light ) );
    light->setCutoff( new LightCutoff( light ) );
//    light->setVertexGenerator( new FaceLightVertexGenerator( mesh, true, 3 ) );
//  light->setVertexGenerator( new FaceLightVertexGenerator( mesh, false, 0 ) );
  light->setVertexGenerator( new FaceLightVertexGenerator( mesh, true, 0 ) );
//  light->setVertexGenerator( new LightVertexGenerator( mesh ) );
    light->setCastsShadow( castsShadow );
    light->setPosition( position );
    light->setColor( color );
    light->setIntensity( intensity );
    
    light->vertexGenerator()->generate();
    
    return light;
}

// ---------------------------------------------------- LightVertexGenerator ------------------------------------------------------ //

// ** LightVertexGenerator::LightVertexGenerator
LightVertexGenerator::LightVertexGenerator( const Mesh* mesh ) : m_mesh( mesh )
{
    
}

// ** LightVertexGenerator::vertexCount
int LightVertexGenerator::vertexCount( void ) const
{
    return ( int )m_vertices.size();
}

// ** LightVertexGenerator::vertices
const LightVertexBuffer& LightVertexGenerator::vertices( void ) const
{
    return m_vertices;
}

// ** LightVertexGenerator::clear
void LightVertexGenerator::clear( void )
{
    m_vertices.clear();
}

// ** LightVertexGenerator::generate
void LightVertexGenerator::generate( void )
{
    clear();

    for( int i = 0; i < m_mesh->vertexCount(); i++ ) {
        push( m_mesh->vertex( i ) ) ;
    }
}

// ** LightVertexGenerator::push
void LightVertexGenerator::push( const Vertex& vertex )
{
     LightVertex lightVertex;

     lightVertex.m_position = vertex.m_position;
     lightVertex.m_normal   = vertex.m_normal;

     m_vertices.push_back( lightVertex );
}

// ------------------------------------------------- FaceLightVertexGenerator --------------------------------------------------- //

// ** FaceLightVertexGenerator::FaceLightVertexGenerator
FaceLightVertexGenerator::FaceLightVertexGenerator( const Mesh* mesh, bool excludeVertices, int maxSubdivisions )
    : LightVertexGenerator( mesh ), m_excludeVertices( excludeVertices ), m_maxSubdivisions( maxSubdivisions )
{

}

// ** FaceLightVertexGenerator::generate
void FaceLightVertexGenerator::generate( void )
{
    if( !m_excludeVertices ) {
        LightVertexGenerator::generate();
    }
    
    for( int i = 0; i < m_mesh->faceCount(); i++ ) {
        generateFromTriangle( m_mesh->face( i ), 0 );
    }
}

// ** FaceLightVertexGenerator::generateFromTriangle
void FaceLightVertexGenerator::generateFromTriangle( const Triangle& triangle, int subdivision )
{
    // ** Generate light vertex from triangle centroid
    push( triangle.centroid() );

    // ** The maximum subdivisions exceeded
    if( subdivision >= m_maxSubdivisions ) {
        return;
    }

    // ** Tesselate a triangle
    Triangle center, corners[3];
    triangle.tesselate( center, corners );

    // ** Process corner triangles
    for( int i = 0; i < 3; i++ ) {
        generateFromTriangle( corners[i], subdivision + 1 );
    }
}

// -------------------------------------------------------- PhotonEmitter --------------------------------------------------------- //

// ** PhotonEmitter::PhotonEmitter
PhotonEmitter::PhotonEmitter( const Light* light ) : m_light( light )
{

}

// ** PhotonEmitter::photonCount
int PhotonEmitter::photonCount( void ) const
{
    return m_light->intensity() * 25000;
}

// ** PhotonEmitter::emit
Vec3 PhotonEmitter::emit( void ) const
{
    return Vec3::randomDirection();

}

// ------------------------------------------------------- LightInfluence --------------------------------------------------------- //

// ** LightInfluence::LightInfluence
LightInfluence::LightInfluence( const Light* light ) : m_light( light )
{

}

// ** LightInfluence::calculate
float LightInfluence::calculate( rt::ITracer* tracer, const Vec3& light, const Vec3& point, const Vec3& normal, float& distance ) const
{
    Vec3 direction = light - point;
    distance       = direction.normalize();

    // ** Calculate Lambert's cosine law intensity
    float intensity = lambert( direction, normal );
    if( intensity <= 0.001f ) {
        return 0.0f;
    }

    // ** Cast shadow to point
    if( m_light->castsShadow() ) {
        intensity *= tracer->test( point, light ) ? 0.0f : 1.0f;
    }

    return intensity;
}

// ** LightInfluence::lambert
float LightInfluence::lambert( const Vec3& direction, const Vec3& normal )
{
    float dp = direction * normal;
    return dp < 0.0f ? 0.0f : dp;
}

// --------------------------------------------------- DirectionalLightInfluence -------------------------------------------------- //

// ** DirectionalLightInfluence::DirectionalLightInfluence
DirectionalLightInfluence::DirectionalLightInfluence( const Light* light, const Vec3& direction ) : LightInfluence( light ), m_direction( direction )
{

}

// ** DirectionalLightInfluence::calculate
float DirectionalLightInfluence::calculate( rt::ITracer* tracer, const Vec3& light, const Vec3& point, const Vec3& normal, float& distance ) const
{
    float intensity = lambert( -m_direction, normal );

    if( intensity <= 0.001f ) {
        return 0.0f;
    }

    // ** Cast shadow to point
    if( m_light->castsShadow() ) {
        intensity *= tracer->test( point, point - m_direction * 1000 ) ? 0.0f : 1.0f;
    }

    return intensity;
}

// --------------------------------------------------------- LightCutoff ---------------------------------------------------------- //

// ** LightCutoff::LightCutoff
LightCutoff::LightCutoff( const Light* light ) : m_light( light )
{

}

// ** LightCutoff::calculate
float LightCutoff::calculate( const Vec3& point ) const
{
    return 1.0f;
}

// ** LightCutoff::cutoffForDirection
float LightCutoff::cutoffForDirection( const Vec3& direction ) const
{
    return 1.0f;
}

// ------------------------------------------------------- LightSpotCutoff -------------------------------------------------------- //

// ** LightSpotCutoff::LightSpotCutoff
LightSpotCutoff::LightSpotCutoff( const Light* light, const Vec3& direction, float cutoff, float exponent ) : LightCutoff( light ), m_direction( direction ), m_cutoff( cutoff ), m_exponent( exponent )
{

}

// ** LightSpotCutoff::calculate
float LightSpotCutoff::calculate( const Vec3& point ) const
{
    Vec3 dir = point - m_light->position();
    dir.normalize();

    return cutoffForDirection( dir );
}

// ** LightSpotCutoff::cutoffForDirection
float LightSpotCutoff::cutoffForDirection( const Vec3& direction ) const
{
    float value = direction * m_direction;

    if( value <= m_cutoff ) {
        return 0.0f;
    }

    value = (1.0 - (1.0 - value) * 1.0/(1.0 - m_cutoff));

    if( fabs( 1.0f - m_exponent ) > 0.01f ) {
        value = powf( value, m_exponent );
    }

    return value;
}

// ------------------------------------------------------- LightAttenuation ------------------------------------------------------- //

// ** LightAttenuation::LightAttenuation
LightAttenuation::LightAttenuation( const Light* light ) : m_light( light )
{

}

// ---------------------------------------------------- LinearLightAttenuation ---------------------------------------------------- //

// ** LinearLightAttenuation::LinearLightAttenuation
LinearLightAttenuation::LinearLightAttenuation( const Light* light, float radius ) : LightAttenuation( light ), m_radius( radius )
{

}

// ** LinearLightAttenuation::calculate
float LinearLightAttenuation::calculate( float distance ) const
{
    return max( 1.0f - (distance / m_radius), 0.0f );
}

} // namespace relight