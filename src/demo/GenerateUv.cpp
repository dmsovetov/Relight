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

#include <math/Mesh.h>
#include <math/DCEL.h>

#ifdef WIN32
	#include <Windows.h>
	#include <gl/GL.h>
#endif

// ** GenerateUv::GenerateUv
GenerateUv::GenerateUv( renderer::Hal* hal ) : m_hal( hal )
{
    m_meshVertexLayout = m_hal->createVertexDeclaration( "P3:N:T0:T1", sizeof( SceneVertex ) );

	m_simpleScene = scene::Scene::create();
	m_simpleScene->setRenderer( new scene::Renderer( hal ) );

	scene::MeshPtr mesh = createMeshFromFile( "Assets/models/Rock03.fbx" );

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

//	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	m_simpleScene->update( 0.1f );
	m_simpleScene->render( f32( window->width() ) / window->height() );

    m_hal->present();
}

// ** GenerateUv::createMeshFromFile
scene::MeshPtr GenerateUv::createMeshFromFile( CString fileName )
{
    fbx::FbxLoader loader;

    if( !loader.load( fileName ) ) {
        return scene::MeshPtr();
    }

//	fbx::FbxLoader::FbxMeshIndexer::VertexBuffer& vertices = loader.vertexBuffer();
//	fbx::FbxLoader::FbxMeshIndexer::IndexBuffer&  indices  = loader.indexBuffer();

	typedef math::TriMesh<SceneVertex> SceneMesh;
	typedef math::MeshIndexer<SceneVertex, SceneVertexCompare> SceneMeshIndexer;

	// ** Convert loaded mesh to scene mesh
	SceneMesh::Vertices vertices;
	SceneMesh::Indices  indices;

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

		vertices.push_back( sv );
	}

	for( int i = 0, n = loader.indexBuffer().size(); i < n; i++ ) {
		indices.push_back( loader.indexBuffer()[i] );
	}

	// ** Process the mesh.
	SceneMesh		 sceneMesh( vertices, indices );
	SceneMeshIndexer sceneMeshIndexer;

	for( int i = 0; i < sceneMesh.faceCount(); i++ ) {
		SceneMesh::Face face	= sceneMesh.face( i );
		math::Vec3      normal	= face.normal();
		float			nx		= fabs( normal.x );
		float			ny		= fabs( normal.y );
		float			nz		= fabs( normal.z );
		math::Vec3		axis;

		math::Plane plane( axis );
		math::Vec3  projected;

		for( int j = 0; j < 3; j++ ) {
			SceneVertex v = face[j];

			if( nx > ny && nx > nz ) {
				axis		= math::Vec3( 1, 0, 0 );
				projected	= plane * v.position;
				v.position = math::Vec3( projected.y, 0, projected.z );
			}
			else if( ny > nx && ny > nz ) {
				axis		= math::Vec3( 0, 1, 0 );
				projected	= plane * v.position;
				v.position = math::Vec3( projected.x, 0, projected.z );
			}
			else if( nz > ny && nz > nx ) {
				axis		= math::Vec3( 0, 0, 1 );
				projected	= plane * v.position;
				v.position = math::Vec3( projected.x, 0, projected.y );
			}

			sceneMeshIndexer += v;
		}
	}

	vertices = sceneMeshIndexer.vertexBuffer();
	indices  = sceneMeshIndexer.indexBuffer();

	// ** Create buffers
    renderer::VertexBuffer* vertexBuffer = m_hal->createVertexBuffer( m_meshVertexLayout, vertices.size(), false );
	renderer::IndexBuffer*	indexBuffer  = m_hal->createIndexBuffer( indices.size(), false );

	// ** Upload data
    memcpy( vertexBuffer->lock(), &vertices[0], vertices.size() * sizeof( vertices[0] ) );
    vertexBuffer->unlock();

    memcpy( indexBuffer->lock(), &indices[0], indices.size() * sizeof( indices[0] ) );
    indexBuffer->unlock();

	// ** Create mesh
	scene::MeshPtr mesh = scene::Mesh::create();
	mesh->addChunk( vertexBuffer, indexBuffer );

	return mesh;
}