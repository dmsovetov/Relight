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

#include "Scene.h"
#include "Mesh.h"
#include "../Lightmap.h"
#include "../rt/Embree.h"

namespace relight {

// ** Scene::Scene
Scene::Scene( void ) : m_state( StateInitial ), m_tracer( NULL )
{

}

// ** Scene::create
Scene* Scene::create( void )
{
    Scene* scene = new Scene;
    return scene;
}

// ** Scene::tracer
rt::ITracer* Scene::tracer( void ) const
{
    return m_tracer;
}

// ** Scene::createLightmap
Lightmap* Scene::createLightmap( int width, int height ) const
{
    return new Lightmap( width, height );
}

// ** Scene::createPhotonmap
Photonmap* Scene::createPhotonmap( int width, int height ) const
{
    return new Photonmap( width, height );
}

// ** Scene::bounds
const Bounds& Scene::bounds( void ) const
{
    return m_bounds;
}

// ** Scene::lightCount
int Scene::lightCount( void ) const
{
    return ( int )m_lights.size();
}

// ** Scene::light
const Light* Scene::light( int index ) const
{
    assert( index >= 0 && index < lightCount() );
    return m_lights[index];
}

// ** Scene::meshCount
int Scene::meshCount( void ) const
{
    return ( int )m_meshes.size();
}

// ** Scene::mesh
const Mesh* Scene::mesh( int index ) const
{
    assert( index >= 0 && index < meshCount() );
    return m_meshes[index];
}

// ** Scene::updateBounds
void Scene::updateBounds( void )
{
    m_bounds = Bounds();

    for( int i = 0; i < meshCount(); i++ ) {
        m_bounds += mesh( i )->bounds();
    }
}

// ** Scene::addLight
RelightStatus Scene::addLight( const Light* light )
{
    m_lights.push_back( light );
}

// ** Scene::addMesh
Mesh* Scene::addMesh( const Mesh* mesh, const Matrix4& transform, const Material* material )
{
    Mesh* transformed = mesh->transformed( transform );
    if( material ) {
        transformed->overrideMaterial( material );
    }
    m_meshes.push_back( transformed );
    updateBounds();
    return transformed;
}

// ** Scene::begin
RelightStatus Scene::begin( void )
{
    if( m_state != StateInitial ) {
        return RelightInvalidCall;
    }

    m_state = StateAddingObjects;

    return RelightSuccess;
}

// ** Scene::end
RelightStatus Scene::end( void )
{
    if( m_state != StateAddingObjects ) {
        return RelightInvalidCall;
    }

    // ** Create a tracer
    m_tracer = new rt::Embree;
    m_tracer->begin();

    for( int i = 0, n = ( int )m_meshes.size(); i < n; i++ ) {
        m_tracer->addMesh( m_meshes[i] );
    }

    m_tracer->end();

    // ** Switch scene state
    m_state = StateReadyToBake;

    return RelightSuccess;
}

} // namespace relight