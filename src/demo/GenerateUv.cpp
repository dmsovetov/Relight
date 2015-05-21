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

#include "GenerateUv.h"
#include "FbxLoader.h"

#ifdef WIN32
	#include <Windows.h>
	#include <gl/GL.h>
#endif

// ** GenerateUv::GenerateUv
GenerateUv::GenerateUv( renderer::Hal* hal ) : m_hal( hal )//, m_cov( 2, 2 )
{
    m_meshVertexLayout = m_hal->createVertexDeclaration( "P3:N:T0:T1", sizeof( SceneVertex ) );

	m_simpleScene = scene::Scene::create();
	m_simpleScene->setRenderer( new scene::Renderer( hal ) );

//	scene::MeshPtr mesh = createMeshFromFile( "Assets/models/dungeon/Column7.fbx" );
	scene::MeshPtr mesh = createMeshFromFile( "Assets/models/Stair01_c.fbx" );

	scene::SceneObjectPtr object = scene::SceneObject::create(); 
	object->attach<scene::Transform>();
	object->attach<scene::MeshRenderer>( mesh );

	m_simpleScene->add( object );
}

// ** GenerateUv::handleUpdate
void GenerateUv::handleUpdate( platform::Window* window )
{
    if( !m_hal->clear() ) {
        return;
    }

	const bool kShowMesh  = true;
    const bool kShowSeams = true;
    const bool kShowFaces = true;

	m_simpleScene->update( 0.1f );
	if( kShowMesh ) {
		m_simpleScene->render( f32( window->width() ) / window->height() );
	}

    SceneTriMesh::Dcel dcel = m_loadedTriMesh->dcel();

    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf( m_simpleScene->camera()->proj( f32( window->width() ) / window->height() ).m );

    glMatrixMode( GL_MODELVIEW );
    glLoadMatrixf( m_simpleScene->camera()->view().m );

    glDepthFunc( GL_LEQUAL );
    glLineWidth( 3.0f );
    for( int i = 0; i < dcel.edgeCount(); i++ )
    {
		if( !kShowMesh ) {
			continue;
		}

        const SceneTriMesh::Dcel::Edge* edge = dcel.edge( i );

        glBegin( GL_LINES );
            if( edge->isBoundary() )
                glColor3f( 0, 1, 0 );
            else
                glColor3f( 1, 0, 0 );
            glVertex3fv( &m_loadedVertices[edge->m_vertex].position.x );
            glVertex3fv( &m_loadedVertices[edge->m_next->m_vertex].position.x );
        glEnd();
    }

    m_hal->present();
}

// ** GenerateUv::createMeshFromFile
scene::MeshPtr GenerateUv::createMeshFromFile( CString fileName )
{
    fbx::FbxLoader loader;

    if( !loader.load( fileName ) ) {
        return scene::MeshPtr();
    }

	// ** Convert loaded mesh to scene mesh
	for( int i = 0, n = loader.vertexBuffer().size(); i < n; i++ ) {
        const fbx::Vertex& rv = loader.vertexBuffer()[i];
        SceneVertex        sv;

        sv.position.x  = rv.position[0]; sv.position.y  = rv.position[1]; sv.position.z  = rv.position[2];
        sv.normal.x    = rv.normal[0];   sv.normal.y    = rv.normal[1];   sv.normal.z    = rv.normal[2];

        sv.uv[0].x	   = rv.uv[0][0];    sv.uv[0].y		= rv.uv[0][1];
        sv.uv[1].x	   = rv.uv[1][0];    sv.uv[1].y		= rv.uv[1][1];

		sv.position.x *= 0.01f;
		sv.position.y *= 0.01f;
		sv.position.z *= 0.01f;

		m_loadedVertices.push_back( sv );
	}

	for( int i = 0, n = loader.indexBuffer().size(); i < n; i++ ) {
		m_loadedIndices.push_back( loader.indexBuffer()[i] );
	}

	// ** Process the mesh.
    m_loadedTriMesh = new SceneTriMesh( m_loadedVertices, m_loadedIndices );
	SceneMeshIndexer sceneMeshIndexer;

	// ** Create buffers
    renderer::VertexBuffer* vertexBuffer = m_hal->createVertexBuffer( m_meshVertexLayout, m_loadedVertices.size(), false );
    renderer::IndexBuffer*	indexBuffer  = m_hal->createIndexBuffer( m_loadedIndices.size(), false );

	// ** Upload data
    memcpy( vertexBuffer->lock(), &m_loadedVertices[0], m_loadedVertices.size() * sizeof( m_loadedVertices[0] ) );
    vertexBuffer->unlock();

    memcpy( indexBuffer->lock(), &m_loadedIndices[0], m_loadedIndices.size() * sizeof( m_loadedIndices[0] ) );
    indexBuffer->unlock();

	// ** Create mesh
	scene::MeshPtr mesh = scene::Mesh::create();
	mesh->addChunk( vertexBuffer, indexBuffer );

	return mesh;
}