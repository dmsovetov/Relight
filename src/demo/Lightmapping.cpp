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

#include "Lightmapping.h"
#include "FbxLoader.h"

#include <OpenGL/gl.h>

// ** Lightmapping::Lightmapping
Lightmapping::Lightmapping( renderer::Hal* hal ) : m_hal( hal )
{
    using namespace uscene;

    m_meshVertexLayout = m_hal->createVertexDeclaration( "P3:N:C4:T0:T1", sizeof( SceneVertex ) );

    m_assets = Assets::parse( "Assets/assets" );
    m_scene  = Scene::parse( m_assets, "Assets/Crypt/Scenes/Test.scene" );

    m_relightScene = relight::Scene::create();
    m_relightScene->begin();
    for( int i = 0; i < m_scene->sceneObjectCount(); i++ ) {
        SceneObject* sceneObject = m_scene->sceneObject( i );
        Transform*   transform   = sceneObject->transform();
        Mesh*        mesh        = sceneObject->mesh();
        Renderer*    renderer    = sceneObject->renderer();

        const float* position   = transform->position();
        const char*  fileName   = mesh->asset()->fileName().c_str();
        SceneMesh*   sceneMesh  = findMesh( mesh->asset() );

        if( renderer->materials().size() ) {
            sceneMesh->m_diffuse = createTextureFromAsset( renderer->materials()[0]->texture( uscene::Material::Diffuse ) );
        }

        sceneObject->setUserData( sceneMesh );
        m_relightScene->addMesh( sceneMesh->m_mesh, affineTransform( transform ) );

        printf( "%s (%s) : %f %f %f\n", sceneObject->name().c_str(), fileName, position[0], position[1], position[2] );
    }
    m_relightScene->end();
}

// ** Lightmapping::affineTransform
relight::Matrix4 Lightmapping::affineTransform( const uscene::Transform *transform )
{
    relight::Vec3 position = relight::Vec3( transform->position()[0], transform->position()[1], transform->position()[2] );
    relight::Vec3 scale    = relight::Vec3( transform->scale()[0], transform->scale()[1], transform->scale()[2] );
    relight::Quat rotation = relight::Quat( transform->rotation()[0], transform->rotation()[1], transform->rotation()[2], transform->rotation()[3] );

    return relight::Matrix4::affineTransform( position, rotation, scale );
}

// ** Lightmapping::createTextureFromAsset
renderer::Texture* Lightmapping::createTextureFromAsset( const uscene::Asset* asset )
{
    if( m_textures.count( asset ) ) {
        return m_textures[asset];
    }

    printf( "Loading %s...\n", asset->fileName().c_str() );

    relight::Texture* image = relight::Texture::createFromFile( asset->fileName().c_str() );
    if( !image ) {
        return NULL;
    }

    renderer::Texture2D* texture = m_hal->createTexture2D( image->width(), image->height(), renderer::PixelRgb32F );
    texture->setData( 0, ( void* )&image->pixels()[0].r );
    delete image;

    m_textures[asset] = texture;

    return texture;
}

// ** Lightmapping::findMesh
SceneMesh* Lightmapping::findMesh( const uscene::Asset* asset )
{
    if( m_meshes.count( asset ) ) {
        return &m_meshes[asset];
    }

    const uscene::MeshAsset* meshAsset = static_cast<const uscene::MeshAsset*>( asset );

    fbx::FbxLoader loader;
    if( !loader.load( asset->fileName().c_str() ) ) {
        return NULL;
    }

    relight::VertexBuffer vertices;
    relight::IndexBuffer  indices;
    relight::Matrix4      transform = relight::Matrix4::scale( meshAsset->scale(), meshAsset->scale(), meshAsset->scale() );

    for( int i = 0; i < loader.vertexBuffer().size(); i++ ) {
        const fbx::Vertex& fbxVertex = loader.vertexBuffer()[i];
        relight::Vertex    v;

        v.m_position = relight::Vec3( fbxVertex.position[0], fbxVertex.position[1], fbxVertex.position[2] );
        v.m_normal   = relight::Vec3( fbxVertex.normal[0], fbxVertex.normal[1], fbxVertex.normal[2] );
        v.m_color    = relight::Color( 1, 1, 1 );

        for( int j = 0; j < relight::Vertex::TotalUvLayers; j++ ) {
            v.m_uv[j] = relight::Uv( fbxVertex.uv[j][0], fbxVertex.uv[j][1] );
        }

        v.m_position = transform * v.m_position;
        v.m_normal   = transform * v.m_normal;
        v.m_normal.normalize();

        vertices.push_back( v );
    }

    for( int i = 0; i < loader.indexBuffer().size(); i++ ) {
        indices.push_back( loader.indexBuffer()[i] );
    }

    SceneMesh sceneMesh;

    // ** Create Relight mesh
    sceneMesh.m_mesh = relight::Mesh::create();
    sceneMesh.m_mesh->addFaces( vertices, indices );

    // ** Create HAL vertex & index buffers
    createBuffersFromMesh( sceneMesh );

    m_meshes[asset] = sceneMesh;

    return &m_meshes[asset];
}

// ** Lightmapping::createGroundPlane
void Lightmapping::createGroundPlane( int size, relight::VertexBuffer& vertexBuffer, relight::IndexBuffer& indexBuffer )
{
    using namespace relight;

    // ** Vertices
    Vertex a;
    a.m_position                = Vec3( -size / 2, 0, -size / 2 );
    a.m_normal                  = Vec3( 0, 1, 0 );
    a.m_color                   = Color( 1, 1, 1 );
    a.m_uv[Vertex::Lightmap]    = Uv( 0, 0 );
    a.m_uv[Vertex::Diffuse]     = Uv( 0, 0 );
    vertexBuffer.push_back( a );

    Vertex b;
    b.m_position                = Vec3(  size / 2, 0, -size / 2 );
    b.m_normal                  = Vec3( 0, 1, 0 );
    b.m_color                   = Color( 1, 1, 1 );
    b.m_uv[Vertex::Lightmap]    = Uv( 0.99f, 0 );
    b.m_uv[Vertex::Diffuse]     = Uv( size, 0 );
    vertexBuffer.push_back( b );

    Vertex c;
    c.m_position                = Vec3(  size / 2, 0,  size / 2 );
    c.m_normal                  = Vec3( 0, 1, 0 );
    c.m_color                   = Color( 1, 1, 1 );
    c.m_uv[Vertex::Lightmap]    = Uv( 0.99f, 0.99f );
    c.m_uv[Vertex::Diffuse]     = Uv( size, size );
    vertexBuffer.push_back( c );

    Vertex d;
    d.m_position                = Vec3( -size / 2, 0,  size / 2 );
    d.m_normal                  = Vec3( 0, 1, 0 );
    d.m_color                   = Color( 1, 1, 1 );
    d.m_uv[Vertex::Lightmap]    = Uv( 0, 0.99f );
    d.m_uv[Vertex::Diffuse]     = Uv( 0, size );
    vertexBuffer.push_back( d );

    // ** Indices
    indexBuffer.push_back( 2 );
    indexBuffer.push_back( 1 );
    indexBuffer.push_back( 0 );

    indexBuffer.push_back( 3 );
    indexBuffer.push_back( 2 );
    indexBuffer.push_back( 0 );
}

// ** Lightmapping::createBuffersFromMesh
void Lightmapping::createBuffersFromMesh( SceneMesh& mesh )
{
    mesh.m_vertexBuffer = m_hal->createVertexBuffer( m_meshVertexLayout, mesh.m_mesh->vertexCount(), false );
    SceneVertex* vertices = ( SceneVertex* )mesh.m_vertexBuffer->lock();
    for( int i = 0; i < mesh.m_mesh->vertexCount(); i++ ) {
        const relight::Vertex& rv = mesh.m_mesh->vertex( i );
        SceneVertex&           sv = vertices[i];

        sv.x  = rv.m_position.x; sv.y  = rv.m_position.y; sv.z  = rv.m_position.z;
        sv.nx = rv.m_normal.x;   sv.ny = rv.m_normal.y;   sv.nz = rv.m_normal.z;
        sv.u0 = rv.m_uv[0].x;    sv.v0 = rv.m_uv[0].y;
        sv.u1 = rv.m_uv[1].x;    sv.v1 = rv.m_uv[1].y;
        sv.r  = sv.g = sv.b = sv.a = 255;
    }
    mesh.m_vertexBuffer->unlock();

    mesh.m_indexBuffer = m_hal->createIndexBuffer( mesh.m_mesh->indexCount(), false );
    memcpy( mesh.m_indexBuffer->lock(), mesh.m_mesh->indexBuffer(), mesh.m_mesh->indexCount() * sizeof( relight::Index ) );
    mesh.m_indexBuffer->unlock();
}

// ** Lightmapping::handleUpdate
void Lightmapping::handleUpdate( platform::Window* window )
{
    using namespace relight;

    static float rotation = 0.0;

    Matrix4 proj = Matrix4::perspective( 60.0f, window->width() / float( window->height() ), 0.01f, 1000.0f );
    Matrix4 view = Matrix4::lookAt( Vec3( 3, 3, 3 ), Vec3( 0, 0, 0 ), Vec3( 0, 1, 0 ) );

    m_hal->clear( Rgba( 0.3f, 0.3f, 0.3f ) );

//    m_hal->setTransform( renderer::TransformView, view.m );
    m_hal->setTransform( renderer::TransformProjection, proj.m );

    glTranslatef( 0, -2, -5 );
    glRotatef( rotation, 0, 1, 0 );
    rotation += 0.1f;

    for( int i = 0; i < m_scene->sceneObjectCount(); i++ ) {
        uscene::SceneObject* object     = m_scene->sceneObject( i );
        SceneMesh*           mesh       = reinterpret_cast<SceneMesh*>( object->userData() );
   //     Matrix4              transform  = view * affineTransform( object->transform() );

    //    m_hal->setTransform( renderer::TransformModel, transform.m );

        glPushMatrix();
        glMultMatrixf( affineTransform( object->transform() ).m );

        m_hal->setTexture( 0, mesh->m_diffuse );
        m_hal->setVertexBuffer( mesh->m_vertexBuffer );
        m_hal->renderIndexed( renderer::PrimTriangles, mesh->m_indexBuffer, 0, mesh->m_indexBuffer->size() );

        glPopMatrix();
    }

    m_hal->setVertexBuffer( NULL );

    glBegin( GL_LINES );
        glColor3f( 1, 0, 0 ); glVertex3f( 0, 0, 0 ); glVertex3f( 1, 0, 0 );
        glColor3f( 0, 1, 0 ); glVertex3f( 0, 0, 0 ); glVertex3f( 0, 1, 0 );
        glColor3f( 0, 0, 1 ); glVertex3f( 0, 0, 0 ); glVertex3f( 0, 0, 1 );
    glEnd();

    m_hal->present();
}