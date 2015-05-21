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

#ifdef WIN32
	#include <Windows.h>
	#include <gl/GL.h>
#endif

#define GLSL( ... ) #__VA_ARGS__

#define USE_BAKED		(0)
#define USE_HDR			(1)

#define BAKE_INDIRECT	(0)
#define GENERATE_UV		(1)

// ** Lightmap size
const int k_LightmapMaxSize = 128;
const int k_LightmapMinSize = 128;

// ** Background workers
const int k_Workers      = 8;

// ** Debug flags
const bool k_DebugShowStatic = false;

// ** Blue sky color
//const relight::Rgb k_BlueSky        = relight::Rgb( 0.52734375f, 0.8046875f, 0.91796875f );

// ** Black sky
const Rgb k_BlackSky       = Rgb( 0, 0, 0 );

// ** Sun light color
//const relight::Rgb k_SunColor           = relight::Rgb( 0.75f, 0.74609375f, 0.67578125f );
//const float        k_SunColorIntensity  = 1.5f;

// ** Dark blue light
//const relight::Rgb  k_DarkLightColor        = relight::Rgb( 0.0737915039f, 0.166036054f, 0.395522416f );
//const float         k_DarkLightIntensity    = 1.5f;

// ** Scene ambient color
const Rgb k_AmbientColor = Rgb( /*0.34f, 0.34f, 0.34f*/0, 0, 0 );

// ** Indirect light settings
const relight::IndirectLightSettings k_IndirectLight = relight::IndirectLightSettings::fast( k_BlackSky, k_AmbientColor, 100, 500 );

// ** Lightmapping::Lightmapping
Lightmapping::Lightmapping( renderer::Hal* hal ) : m_hal( hal )
{
    using namespace uscene;

	// ** Create the vertex layout
    m_meshVertexLayout = m_hal->createVertexDeclaration( "P3:N:T0:T1", sizeof( SceneVertex ) );

    m_assets = Assets::parse( "Assets/assets" );
//    m_scene  = Scene::parse( m_assets, "Assets/Crypt/Scenes/Simple.scene" );
//    m_scene  = Scene::parse( m_assets, "Assets/Crypt/Demo/NoTerrain.scene" );
//	m_scene = Scene::parse( m_assets, "Assets/Demo/Demo7.scene" );
//	m_scene = Scene::parse( m_assets, "Assets/Test.scene" );
	m_scene = Scene::parse( m_assets, "Assets/scenes/Crypt.scene" );

    if( !m_scene ) {
        printf( "Failed to create scene\n" );
        return;
    }

	// *******************************************

	m_simpleScene = scene::Scene::create();
	m_renderer    = new scene::Renderer( m_hal );

	// *******************************************

    m_relight      = relight::Relight::create();
    m_relightScene = m_relight->createScene();
    m_relightScene->begin();

    int totalLightmapPixels = 0;

    float maxArea = -FLT_MAX;
    float scale = 170.0f;

    // ** Add objects
    for( int i = 0; i < m_scene->sceneObjectCount(); i++ ) {
        SceneObject*        sceneObject = m_scene->sceneObject( i );
        Transform*          transform   = sceneObject->transform();
        Mesh*               mesh        = sceneObject->mesh();
        Renderer*           renderer    = sceneObject->renderer();
		Light*				light		= sceneObject->light();

		Vec3 position, scale;
		Quat rotation;

		extractTransform( transform, position, rotation, scale );

		// **********************************************************************************************

		scene::SceneObjectPtr object = scene::SceneObject::create();
		object->attach<scene::Transform>( position, rotation, scale );
		m_simpleScene->add( object );

		sceneObject->setUserData( static_cast<void*>( object.get() ) );

		// **********************************************************************************************

		if( light ) {
			switch( light->type() ) {
			case 2:			m_relightScene->addLight( relight::Light::createPointLight( affineTransform( transform ) * Vec3(), light->range(), light->color(), light->intensity() ) );
										break;

			case 1:	m_relightScene->addLight( relight::Light::createDirectionalLight( rotation.rotate( Vec3(0, 0, 1) ), light->color(), light->intensity(), true ) );
										break;
			}
			
			continue;
		}

        if( !mesh ) {
            continue;
        }

        bool addToRelight = sceneObject->isStatic();
        bool isSolid      = false;
        std::string shader = renderer->materials()[0]->shader();

        if( shader == "diffuse" || shader == "specular" ) {
        //    m_solidRenderList.push_back( sceneObject );
            isSolid = true;
        }
        else if( shader == "additive" ) {
        //    m_additiveRenderList.push_back( sceneObject );
            addToRelight = false;
        }
        else {
        //    m_transparentRenderList.push_back( sceneObject );
        }

        SceneMeshInstance* instance = new SceneMeshInstance;
        instance->m_transform = affineTransform( transform );
        instance->m_lightmap  = NULL;
        instance->m_mesh      = findMesh( mesh->asset(), renderer, isSolid );
        instance->m_dirty     = false;
		object->setUserData<SceneMeshInstance>( instance );

		// **************************************************************************************

		scene::MaterialPtr material = scene::Material::create();
		material->setColor( scene::Material::Diffuse, instance->m_mesh->m_diffuseColor );
		material->setColor( scene::Material::Tint, instance->m_mesh->m_tintColor );
		material->setTexture( scene::Material::Diffuse, instance->m_mesh->m_diffuse );

        if( shader == "diffuse" || shader == "specular" ) {
			material->setShader( scene::Material::Solid );
        }
        else if( shader == "additive" ) {
			material->setShader( scene::Material::Additive );
        }
        else {
			material->setShader( scene::Material::Transparent );
        }

		scene::MeshPtr sceneMesh = scene::Mesh::create();
		sceneMesh->addChunk( instance->m_mesh->m_vertexBuffer, instance->m_mesh->m_indexBuffer );
		StrongPtr<scene::MeshRenderer> meshRenderer = object->attach<scene::MeshRenderer>( sceneMesh );
		meshRenderer->setMaterial( 0, material );

		// **************************************************************************************

        if( !addToRelight ) {
            continue;
        }

        #if USE_BAKED
            instance->m_lightmap = loadLightmapFromFile( "lightmaps/" + std::to_string( sceneObject->objectId() ) + ".raw" );
        #else   
            float area = instance->m_mesh->m_mesh->area();

			if( isNaN( area ) )
			{
				instance->m_mesh->m_mesh->area();
			}

            relight::Mesh* m = m_relightScene->addMesh( instance->m_mesh->m_mesh, instance->m_transform, instance->m_mesh->m_material );
            m->setUserData( instance );

		    maxArea	   = max2( maxArea, instance->m_mesh->m_mesh->area() );
            int   size = min2( k_LightmapMaxSize, ( int )nextPowerOf2( ceil( k_LightmapMinSize + (k_LightmapMaxSize - k_LightmapMinSize) * (area / 170.0f) ) ) );
			DC_BREAK_IF( size < k_LightmapMinSize || size > k_LightmapMaxSize );

            totalLightmapPixels += size * size;

            instance->m_lm = m_relight->createLightmap( size, size );
            instance->m_pm = m_relight->createPhotonmap( size, size );

            instance->m_lm->addMesh( m );
            instance->m_pm->addMesh( m );

		// **********************************************************************************************
            #if USE_HDR
            instance->m_lightmap = m_hal->createTexture2D( instance->m_lm->width(), instance->m_lm->height(), renderer::PixelRgb32F );
            #else
            instance->m_lightmap = m_hal->createTexture2D( instance->m_lm->width(), instance->m_lm->height(), renderer::PixelRgb8 );
            #endif
        #endif
            meshRenderer->setLightmap( instance->m_lightmap );
		// **********************************************************************************************
    }
    m_relightScene->end();

	// ** Link the transforms
	for( int i = 0; i < m_scene->sceneObjectCount(); i++ ) {
		// ** Get Unity scene object
		uscene::SceneObject* uSceneObject = m_scene->sceneObject( i );

		// ** Get the parent transform.
		uscene::Transform* uTransform = uSceneObject->transform();
		
		if( !uTransform->parent() ) {
			continue;
		}

		// ** Get the scene object
		scene::SceneObject* sceneObject  = reinterpret_cast<scene::SceneObject*>( uSceneObject->userData() );
		assert( sceneObject != NULL );

		// ** Get the parent scene object.
		uscene::SceneObject* uParentSceneObject = uTransform->parent()->sceneObject();
		scene::SceneObject*	parentSceneObject   = reinterpret_cast<scene::SceneObject*>( uParentSceneObject->userData() );
		assert( parentSceneObject != NULL );

		// ** Link the transforms.
		sceneObject->get<scene::Transform>()->setParent( parentSceneObject->get<scene::Transform>().get() );
	}

	printf( "%d instances added to relight scene, maximum mesh area %2.4f (%d lightmap pixels used, %d mb used)\n", m_relightScene->meshCount(), maxArea, totalLightmapPixels, totalLightmapPixels * sizeof( relight::Lumel ) / 1024 / 1024 );

#if !USE_BAKED
	#if BAKE_INDIRECT
		printf( "Emitting photons...\n" );
		m_relight->emitPhotons( m_relightScene, k_IndirectLight );
		printf( "Done!\n" );
	#endif

    struct Bake : public relight::Job {
    public:

		Bake( const relight::IndirectLightSettings& indirect )
			: m_indirect( indirect ) {}

        virtual void execute( relight::JobData* data ) {
            data->m_relight->bakeDirectLight( data->m_scene, data->m_mesh, data->m_worker, data->m_iterator );
		#if BAKE_INDIRECT
			data->m_relight->bakeIndirectLight( data->m_scene, data->m_mesh, data->m_worker, m_indirect, data->m_iterator );
		#endif
        }

	private:

		relight::IndirectLightSettings m_indirect;
    };

    typedef ThreadWorker LmWorker;

    // ** Bake scene
    m_rootWorker = new LmWorker;
    for( int i = 0; i < k_Workers; i++ ) {
        m_relightWorkers.push_back( new LmWorker );
    }

	const Rgb kSkyColor( 0.86f, 0.93f, 1.0f );
	m_relight->bake( m_relightScene, new Bake( relight::IndirectLightSettings::production( m_scene->settings()->ambient()/*kSkyColor*/, Rgb(0, 0, 0), 100, 500 ) ), m_rootWorker, m_relightWorkers );
#endif
}

renderer::Texture2D* Lightmapping::loadLightmapFromFile( const std::string& fileName )
{
    printf( "Loading lightmap %s\n", fileName.c_str() );

    int width = 0, height = 0;

    FILE* file = fopen( fileName.c_str(), "rb" );

    fread( &width,  1, sizeof( int ), file );
    fread( &height, 1, sizeof( int ), file );

    float* hdr = new float[width * height * 3];
    fread( hdr, sizeof( float ), width * height * 3, file );
    fclose( file );

    unsigned char* ldr = new unsigned char[width * height * 3];
    for( int y = 0; y < height; y++ ) {
        for( int x = 0; x < width; x++ ) {
            Rgb				hdrPixel = &hdr[y * width * 3 + x * 3];
            unsigned char*	ldrPixel = &ldr[y * width * 3 + x * 3];

            ldrPixel[0] = ( unsigned char )min2( 255.0f, hdrPixel.r * 255.0f * 0.5f );
            ldrPixel[1] = ( unsigned char )min2( 255.0f, hdrPixel.g * 255.0f * 0.5f );
            ldrPixel[2] = ( unsigned char )min2( 255.0f, hdrPixel.b * 255.0f * 0.5f );
        }
    }

    renderer::Texture2D* texture = m_hal->createTexture2D( width, height, renderer::PixelRgb8 );
    texture->setData( 0, ldr );
    delete[]hdr;
    delete[]ldr;

    return texture;
}

// ** Lightmapping::extractTransform
void Lightmapping::extractTransform( const uscene::Transform* transform, Vec3& position, Quat& rotation, Vec3& scale ) const
{
	position = Vec3( -transform->position()[0], transform->position()[1], transform->position()[2] );
	scale    = Vec3(  transform->scale()[0],	 transform->scale()[1],	   transform->scale()[2] );
	rotation = Quat( -transform->rotation()[0], transform->rotation()[1], transform->rotation()[2], -transform->rotation()[3] );
}

// ** Lightmapping::affineTransform
Matrix4 Lightmapping::affineTransform( const uscene::Transform *transform )
{
    if( !transform ) {
        return Matrix4();
    }

	Vec3 position, scale;
	Quat rotation;

	extractTransform( transform, position, rotation, scale );

	return affineTransform( transform->parent() ) * Matrix4::translation( position ) * Matrix4::scale( scale ) *  rotation;
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
    Matrix4				  transform = Matrix4::scale( meshAsset->scale(), meshAsset->scale(), meshAsset->scale() );

    for( int i = 0; i < loader.vertexBuffer().size(); i++ ) {
        const fbx::Vertex& fbxVertex = loader.vertexBuffer()[i];
        relight::Vertex    v;

        v.position = Vec3( fbxVertex.position[0], fbxVertex.position[1], fbxVertex.position[2] );
        v.normal   = Vec3( fbxVertex.normal[0], fbxVertex.normal[1], fbxVertex.normal[2] );

        for( int j = 0; j < relight::Vertex::TotalUvLayers; j++ ) {
            v.uv[j] = relight::Uv( fbxVertex.uv[j][0], fbxVertex.uv[j][1] );
        }

        v.position = transform * v.position;
        v.normal   = transform * v.normal;
        v.normal.normalize();

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
		sceneMesh.m_diffuseColor	= Rgba( diffuse );
		sceneMesh.m_tintColor		= Rgba( material->color( uscene::Material::Tint ) );
    }

    // ** Create Relight mesh
    sceneMesh.m_mesh = relight::Mesh::create();
    sceneMesh.m_mesh->addFaces( vertices, indices );
#if GENERATE_UV
	sceneMesh.m_mesh->generateUv( 88.0f, 0.0f, 0.2f );
#endif

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

        sv.position.x  = rv.position[0]; sv.position.y  = rv.position[1]; sv.position.z  = rv.position[2];
        sv.normal.x    = rv.normal[0];   sv.normal.y    = rv.normal[1];   sv.normal.z    = rv.normal[2];
        sv.uv[0].x	   = rv.uv[0][0];    sv.uv[0].y		= rv.uv[0][1];
        sv.uv[1].x	   = rv.uv[1][0];    sv.uv[1].y		= rv.uv[1][1];
    }
    mesh.m_vertexBuffer->unlock();

    mesh.m_indexBuffer = m_hal->createIndexBuffer( mesh.m_mesh->indexCount(), false );
    memcpy( mesh.m_indexBuffer->lock(), mesh.m_mesh->indexBuffer(), mesh.m_mesh->indexCount() * sizeof( relight::Index ) );
    mesh.m_indexBuffer->unlock();
}

struct Camera {
	Vec3	pos;
	Vec3	view;
	Vec3	right;
	Vec3	up;

	float			yRotation;

	Camera( void ) : yRotation( 90 )
	{
		pos	  = Vec3( 0, 1, 2 );
		right = Vec3( 1, 0, 0 );
		up	  = Vec3( 0, 1, 0 );
		view  = Vec3( 0, -0.447213590, -0.894427180 );
	}

	void update( void )
	{
		platform::Input* input = platform::Input::sharedInstance();

		float speed = 0.001f;

		if( input->keyDown( platform::Key::Shift ) ) speed *= 5.0f;
		if( input->keyDown( platform::Key::Space ) ) speed *= 50.0f;

		if( input->keyDown( platform::Key::W ) ) pos = pos + view  * speed;
		if( input->keyDown( platform::Key::S ) ) pos = pos - view  * speed;

		if( input->keyDown( platform::Key::A ) ) pos = pos - right * speed;
		if( input->keyDown( platform::Key::D ) ) pos = pos + right * speed;
		
		static Vec2 mouse( -1, -1 );

		s32 x = input->mouseX();
		s32 y = input->mouseY();

		if( mouse.x >= 0 && mouse.y >= 0 ) {
			f32 dx = (x - mouse.x) * 0.1f;
			f32 dy = (y - mouse.y) * 0.1f;

			yRotation -= dy;

			if( yRotation >= 180.0f ) {
				yRotation = 180.0f;
			}
			else if( yRotation <= 0.0f ) {
				yRotation = 0.0f;
			}
			else {
				rotate( -dy, right );
			}

			rotate( -dx, up );

			right = view % up;
			right.normalize();

			input->setMouse( mouse.x, mouse.y );
		} else {
			mouse = Vec2( x, y );
		}
	}

	void rotate( float angle, const Vec3& axis )
	{
		view = Quat::rotateAroundAxis( angle, axis ).rotate( view );
	}
};

Camera gCamera;

// ** Lightmapping::handleUpdate
void Lightmapping::handleUpdate( platform::Window* window )
{
    if( !m_scene ) {
        return;
    }
    
    using namespace relight;

	if( platform::Input::sharedInstance()->keyDown( platform::Key::Escape ) ) platform::Application::sharedInstance()->quit();

    if( platform::Input::sharedInstance()->keyDown( platform::Key::RButton ) ) {
	    gCamera.update();
    }

	m_matrixView = Matrix4::lookAt( gCamera.pos, gCamera.pos + gCamera.view, gCamera.up );
	m_matrixProj = Matrix4::perspective( 60.0f, window->width() / float( window->height() ), 0.01f, 1000.0f );

    uscene::SceneSettings*	settings = m_scene->settings();
    const float*			ambient  = settings->ambient();
    const float*			fog      = settings->fogColor();
    Rgba					ambientColor( ambient[0], ambient[1], ambient[2], 1.0f );
    Rgba					fogColor( fog[0], fog[1], fog[2], 1.0f );

    if( !m_hal->clear( fogColor ) ) {
        return;
    }

	m_renderer->render( m_matrixView, m_matrixProj, m_simpleScene.get() );

#if !USE_BACKED
	for( int i = 0; i < m_scene->sceneObjectCount(); i++ ) {
		uscene::SceneObject* object   = m_scene->sceneObject( i );
		SceneMeshInstance*   instance = reinterpret_cast<scene::SceneObject*>( object->userData() )->userData<SceneMeshInstance>();

		if( !instance || !instance->m_dirty ) {
            continue;
        }

        relight::Lightmap* lm = instance->m_lm;

		lm->expand();

		char buffer[256];
		sprintf( buffer, "lightmaps/%d.tga", object->objectId() );
        lm->save( buffer, relight::TgaDoubleLdr );

    #if USE_HDR
        float* pixels = lm->toHdr();
        instance->m_lightmap->setData( 0, pixels );
        delete[]pixels;
    #else
        unsigned char* pixels = lm->toDoubleLdr();
        instance->m_lightmap->setData( 0, pixels );
        delete[]pixels;
    #endif

        instance->m_dirty = false;
    }
#endif

    m_hal->present();
}

// ** ThreadWorker::worker
void ThreadWorker::worker( void* userData )
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
	m_thread = thread::Thread::create();
	m_thread->start( dcThisMethod( ThreadWorker::worker ), data );
}

// ** ThreadWorker::wait
void ThreadWorker::wait( void )
{
	m_thread->wait();
}

// ** ThreadWorker::notify
void ThreadWorker::notify( const relight::Mesh* instance, int step, int stepCount )
{
    reinterpret_cast<SceneMeshInstance*>( instance->userData() )->m_dirty = true;
}