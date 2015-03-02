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

// ** Lightmap size
const int k_LightmapMaxSize = 128;
const int k_LightmapMinSize = 16;

// ** Background workers
const int k_Workers      = 8;

// ** Debug flags
const bool k_DebugShowStatic = false;

// ** Blue sky color
const relight::Rgb k_BlueSky        = relight::Rgb( 0.52734375f, 0.8046875f, 0.91796875f );

// ** Black sky
const relight::Rgb k_BlackSky       = relight::Rgb( 0, 0, 0 );

// ** Sun light color
const relight::Rgb k_SunColor           = relight::Rgb( 0.75f, 0.74609375f, 0.67578125f );
const float        k_SunColorIntensity  = 1.5f;

// ** Dark blue light
const relight::Rgb  k_DarkLightColor        = relight::Rgb( 0.0737915039f, 0.166036054f, 0.395522416f );
const float         k_DarkLightIntensity    = 1.5f;

// ** Scene ambient color
const relight::Rgb k_AmbientColor = relight::Rgb( 0.34f, 0.34f, 0.34f );

// ** Indirect light settings
const relight::IndirectLightSettings k_IndirectLight = relight::IndirectLightSettings::fast( /*k_BlueSky*/k_BlackSky, k_AmbientColor );

unsigned int nextPowerOf2(unsigned int n)
{
    unsigned count = 0;

    /* First n in the below condition is for the case where n is 0*/
    if (n && !(n&(n-1)))
        return n;

    while( n != 0)
    {
        n  >>= 1;
        count += 1;
    }
    
    return 1<<count;
}

// ** Lightmapping::Lightmapping
Lightmapping::Lightmapping( renderer::Hal* hal ) : m_hal( hal )
{
    using namespace uscene;

    m_meshVertexLayout = m_hal->createVertexDeclaration( "P3:N:T0:T1", sizeof( SceneVertex ) );

    m_assets = Assets::parse( "Assets/assets" );
//    m_scene  = Scene::parse( m_assets, "Assets/Crypt/Scenes/Simple.scene" );
    m_scene  = Scene::parse( m_assets, "Assets/Crypt/Demo/NoTerrain.scene" );

    if( !m_scene ) {
        printf( "Failed to create scene\n" );
        return;
    }

    m_relight      = relight::Relight::create();
    m_relightScene = m_relight->createScene();
    m_relightScene->begin();

    // ** Add directional light
    relight::Vec3 direction = relight::Vec3::normalize( relight::Vec3( 0, 2, 0 ) - relight::Vec3( 1.50f, 4.50f,  1.50f ) );
    m_relightScene->addLight( relight::Light::createDirectionalLight( direction, k_DarkLightColor, k_DarkLightIntensity, true ) );

    int totalLightmapPixels = 0;

    float maxArea = -FLT_MAX;
    float scale = 170.0f;

    // ** Add objects
    for( int i = 0; i < m_scene->sceneObjectCount(); i++ ) {
        SceneObject*        sceneObject = m_scene->sceneObject( i );
        Transform*          transform   = sceneObject->transform();
        Mesh*               mesh        = sceneObject->mesh();
        Renderer*           renderer    = sceneObject->renderer();

        if( !mesh ) {
            continue;
        }

        bool addToRelight = sceneObject->isStatic();
        bool isSolid      = false;

        if( renderer->materials()[0]->shader() == "diffuse" ) {
            m_solidRenderList.push_back( sceneObject );
            isSolid = true;
        }
        else if( renderer->materials()[0]->shader() == "additive" ) {
            m_additiveRenderList.push_back( sceneObject );
            addToRelight = false;
        }
        else {
            m_transparentRenderList.push_back( sceneObject );
        }

        SceneMeshInstance* instance = new SceneMeshInstance;

        sceneObject->setUserData( instance );

        instance->m_transform = affineTransform( transform );
        instance->m_lightmap  = NULL;
        instance->m_mesh      = findMesh( mesh->asset(), renderer, isSolid );
        instance->m_dirty     = false;

        if( !addToRelight ) {
            continue;
        }

        maxArea = std::max( maxArea, instance->m_mesh->m_mesh->area() );
        float area = instance->m_mesh->m_mesh->area();
        int   size = nextPowerOf2( ceil( k_LightmapMinSize + (k_LightmapMaxSize - k_LightmapMinSize) * (area / 170.0f) ) );

        totalLightmapPixels += size * size;

        instance->m_lm = m_relight->createLightmap( size, size );
        instance->m_pm = m_relight->createPhotonmap( size, size );

        relight::Mesh* m = m_relightScene->addMesh( instance->m_mesh->m_mesh, instance->m_transform, instance->m_mesh->m_material );
        m->setUserData( instance );

        instance->m_lm->addMesh( m );
        instance->m_pm->addMesh( m );
    }
    m_relightScene->end();

    printf( "%d instances added to relight scene, maximum mesh area %2.4f (%d lightmap pixels used)\n", m_relightScene->meshCount(), maxArea, totalLightmapPixels );

    printf( "Emitting photons...\n" );
    m_relight->emitPhotons( m_relightScene, k_IndirectLight );
    printf( "Done!\n" );

    struct Bake : public relight::Job {
    public:

        virtual void execute( relight::JobData* data ) {
            data->m_relight->bakeDirectLight( data->m_scene, data->m_mesh, data->m_worker, data->m_iterator );
            data->m_relight->bakeIndirectLight( data->m_scene, data->m_mesh, data->m_worker, k_IndirectLight, data->m_iterator );
        }
    };

//    typedef relight::Worker LmWorker;
    typedef ThreadWorker LmWorker;

    // ** Bake scene
    m_rootWorker = new LmWorker;
    for( int i = 0; i < k_Workers; i++ ) {
        m_relightWorkers.push_back( new LmWorker );
    }
    m_relight->bake( m_relightScene, new Bake, m_rootWorker, m_relightWorkers );
}

// ** Lightmapping::affineTransform
relight::Matrix4 Lightmapping::affineTransform( const uscene::Transform *transform )
{
    if( !transform ) {
        return relight::Matrix4();
    }

    relight::Vec3 position = relight::Vec3( transform->position()[0], transform->position()[1], transform->position()[2] );
    relight::Vec3 scale    = relight::Vec3( transform->scale()[0], transform->scale()[1], transform->scale()[2] );
    relight::Quat rotation = relight::Quat( transform->rotation()[0], transform->rotation()[1], transform->rotation()[2], transform->rotation()[3] );

    return affineTransform( transform->parent() ) * relight::Matrix4::affineTransform( position, rotation, scale ) * relight::Matrix4::scale( -1, 1, 1 );
}

// ** Lightmapping::createTexture
renderer::Texture* Lightmapping::createTexture( const relight::Texture* texture )
{
    if( m_textures.count( texture ) ) {
        return m_textures[texture];
    }

    renderer::Texture2D* texture2d = m_hal->createTexture2D( texture->width(), texture->height(), texture->channels() == 3 ? renderer::PixelRgb8 : renderer::PixelRgba8 );
    texture2d->setData( 0, texture->pixels() );

    m_textures[texture] = texture2d;

    return texture2d;
}

// ** Lightmapping::findTexture
relight::Texture* Lightmapping::findTexture( const uscene::Asset* asset, bool solid )
{
    if( m_relightTextures.count( asset ) ) {
        return m_relightTextures[asset];
    }

    relight::Texture* texture = relight::Texture::createFromFile( asset->fileName().c_str() );

    if( solid && texture->channels() == 4 ) {
        texture->convertToRgb();
    }

    if( texture ) {
        m_relightTextures[asset] = texture;
    }

    return texture;
}

// ** Lightmapping::findMesh
SceneMesh* Lightmapping::findMesh( const uscene::Asset* asset, const uscene::Renderer* renderer, bool solid )
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

    // ** Create texture
    if( renderer->materials().size() ) {
        uscene::Material* material  = renderer->materials()[0];
        const float*      diffuse   = material->color( uscene::Material::Diffuse );
        relight::Texture* texture   = findTexture( material->texture( uscene::Material::Diffuse ), solid );
        sceneMesh.m_material        = texture ? new relight::TexturedMaterial( texture, diffuse ) : new relight::Material( diffuse );
        sceneMesh.m_diffuse         = createTexture( texture );
    }

    // ** Create Relight mesh
    sceneMesh.m_mesh = relight::Mesh::create();
    sceneMesh.m_mesh->addFaces( vertices, indices );

    // ** Create HAL vertex & index buffers
    createBuffersFromMesh( sceneMesh );

    m_meshes[asset] = sceneMesh;

    return &m_meshes[asset];
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
    }
    mesh.m_vertexBuffer->unlock();

    mesh.m_indexBuffer = m_hal->createIndexBuffer( mesh.m_mesh->indexCount(), false );
    memcpy( mesh.m_indexBuffer->lock(), mesh.m_mesh->indexBuffer(), mesh.m_mesh->indexCount() * sizeof( relight::Index ) );
    mesh.m_indexBuffer->unlock();
}

// ** Lightmapping::handleUpdate
void Lightmapping::handleUpdate( platform::Window* window )
{
    if( !m_scene ) {
        return;
    }
    
    using namespace relight;

    static float rotation = 0.0;

    Matrix4 proj = Matrix4::perspective( 60.0f, window->width() / float( window->height() ), 0.01f, 1000.0f );
    Matrix4 view = Matrix4::lookAt( Vec3( 3, 3, 3 ), Vec3( 0, 0, 0 ), Vec3( 0, 1, 0 ) );

    uscene::SceneSettings* settings = m_scene->settings();
    const float*           ambient  = settings->ambient();
    const float*           fog      = settings->fogColor();
    dreemchest::Rgba       ambientColor( ambient[0], ambient[1], ambient[2], 1.0f );
    dreemchest::Rgba       fogColor( fog[0], fog[1], fog[2], 1.0f );
//    dreemchest::Rgba       clearColor( k_BlueSky.r, k_BlueSky.g, k_BlueSky.b );

    if( !m_hal->clear( fogColor ) ) {
        return;
    }

//    m_hal->setTransform( renderer::TransformView, view.m );
    m_hal->setTransform( renderer::TransformProjection, proj.m );

    glTranslatef( 0, -3, -6 );
    glRotatef( rotation, 0, 1, 0 );
    rotation += 0.1f;

    m_hal->setFog( renderer::FogExp2, settings->fogDensity() * 7, fogColor, 0, 300 );

    {
        renderObjects( m_solidRenderList );
    }

    {
        m_hal->setAlphaTest( renderer::Greater, 0.5f );
        renderObjects( m_transparentRenderList );
        m_hal->setAlphaTest( renderer::CompareDisabled );
    }

    {
        m_hal->setDepthTest( false, renderer::Less );
        m_hal->setBlendFactors( renderer::BlendOne, renderer::BlendOne );

        renderObjects( m_additiveRenderList );

        m_hal->setDepthTest( true, renderer::Less );
        m_hal->setBlendFactors( renderer::BlendDisabled, renderer::BlendDisabled );
    }

    m_hal->present();
}

// ** Lightmapping::renderObjects
void Lightmapping::renderObjects( const uscene::SceneObjectArray& objects )
{
    for( int i = 0; i < objects.size(); i++ ) {
        uscene::SceneObject* object   = objects[i];
        SceneMeshInstance*   instance = reinterpret_cast<SceneMeshInstance*>( object->userData() );

        if( !instance ) {
            continue;
        }

        glPushMatrix();
        glMultMatrixf( instance->m_transform.m );

        const relight::Rgb& diffuse = instance->m_mesh->m_material->color();

        if( object->isStatic() ) {
            m_hal->setColorModulation( diffuse.r, diffuse.g, diffuse.b, 1.0f );
        } else {
            const float* ambient = m_scene->settings()->ambient();
            m_hal->setColorModulation( diffuse.r * ambient[0], diffuse.g * ambient[1], diffuse.b * ambient[2], 1.0f );
        }

        if( k_DebugShowStatic ) {
            if( object->isStatic() ) {
                m_hal->setColorModulation( 1, 0, 1, 1 );
            } else {
                m_hal->setColorModulation( 1, 1, 0, 1 );
            }
            m_hal->setTexture( 0, NULL );
            m_hal->setTexture( 1, NULL );
        } else {
            m_hal->setTexture( 0, instance->m_mesh->m_diffuse );
            m_hal->setTexture( 1, instance->m_lightmap );
        }

        m_hal->setVertexBuffer( instance->m_mesh->m_vertexBuffer );
        m_hal->renderIndexed( renderer::PrimTriangles, instance->m_mesh->m_indexBuffer, 0, instance->m_mesh->m_indexBuffer->size() );

        glPopMatrix();

        if( instance->m_dirty ) {
            relight::Lightmap* lm = instance->m_lm;

            float* pixels = lm->toRgb32F();
            instance->m_lightmap = m_hal->createTexture2D( lm->width(), lm->height(), renderer::PixelRgb32F );
            instance->m_lightmap->setData( 0, pixels );
            delete[]pixels;

            instance->m_dirty = false;
        }
    }

    m_hal->setVertexBuffer( NULL );
}

// ** ThreadWorker::worker
void* ThreadWorker::worker( void* userData )
{
    relight::JobData* data = reinterpret_cast<relight::JobData*>( userData );
    data->m_job->execute( data );
    delete data;
}

// ** ThreadWorker::ThreadWorker
ThreadWorker::ThreadWorker( void )
{
}

// ** ThreadWorker::push
void ThreadWorker::push( relight::Job* job, relight::JobData* data )
{
    data->m_worker = this;
    pthread_create( &m_thread, NULL, worker, ( void* )data );
}

// ** ThreadWorker::wait
void ThreadWorker::wait( void )
{
    pthread_join( m_thread, NULL );
}

// ** ThreadWorker::notify
void ThreadWorker::notify( const relight::Mesh* instance, int step, int stepCount )
{
    reinterpret_cast<SceneMeshInstance*>( instance->userData() )->m_dirty = true;
}