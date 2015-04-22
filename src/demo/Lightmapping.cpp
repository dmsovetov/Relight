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

// ** Lightmap size
const int k_LightmapMaxSize = 128;
const int k_LightmapMinSize = 32;

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
const relight::Rgb k_AmbientColor = relight::Rgb( /*0.34f, 0.34f, 0.34f*/0, 0, 0 );

// ** Indirect light settings
const relight::IndirectLightSettings k_IndirectLight = relight::IndirectLightSettings::fast( /*k_BlueSky*/k_BlackSky, k_AmbientColor, 100, 500 );

// ** Lightmapping::Lightmapping
Lightmapping::Lightmapping( renderer::Hal* hal ) : m_hal( hal )
{
    using namespace uscene;

	// ** Create the vertex layout
    m_meshVertexLayout = m_hal->createVertexDeclaration( "P3:N:T0:T1", sizeof( SceneVertex ) );

	// ** Create the colored unlit shader
	m_shaderColored = m_hal->createShader(
		GLSL(
			uniform mat4 u_mvp;
			varying vec4 v_color;

			void main()
			{
				v_color		= gl_Color;
				gl_Position = u_mvp * gl_Vertex;
			}
		),
		GLSL(
			varying vec4 v_color;

			void main()
			{
				gl_FragColor = v_color;
			}
		) );

	// ** Create a normals shader
	m_shaderNormals = m_hal->createShader(
		GLSL(
			uniform mat4 u_mvp;
			varying vec4 v_color;

			void main()
			{
				v_color		= vec4( gl_Normal * 0.5 + 0.5, 1.0 );
				gl_Position = u_mvp * gl_Vertex;
			}
		),
		GLSL(
			varying vec4 v_color;

			void main()
			{
				gl_FragColor = v_color;
			}
		) );

	// ** Create the lightmap shader
	m_shaderLightmaped = m_hal->createShader(
		GLSL(
			uniform mat4 u_mvp;
			varying vec2 v_tex0;
			varying vec2 v_tex1;

			void main()
			{
				v_tex0 = gl_MultiTexCoord0.xy;
				v_tex1 = gl_MultiTexCoord1.xy;
				// Transforming The Vertex
				gl_Position = u_mvp * gl_Vertex;
			} ),
		GLSL(
			uniform sampler2D u_diffuse;
			uniform sampler2D u_lightmap;
			uniform vec3	  u_diffuseColor;
			varying vec2	  v_tex0;
			varying vec2	  v_tex1;

			void main()
			{
				// Setting Each Pixel To Red
				gl_FragColor = texture2D( u_diffuse, v_tex0 ) * texture2D( u_lightmap, v_tex1 ) * u_diffuseColor;
			} ) );

    m_assets = Assets::parse( "Assets/assets" );
//    m_scene  = Scene::parse( m_assets, "Assets/Crypt/Scenes/Simple.scene" );
//    m_scene  = Scene::parse( m_assets, "Assets/Crypt/Demo/NoTerrain.scene" );
	m_scene = Scene::parse( m_assets, "Assets/Demo/Demo7.scene" );
//	m_scene = Scene::parse( m_assets, "Assets/Test.scene" );

    if( !m_scene ) {
        printf( "Failed to create scene\n" );
        return;
    }

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

		if( light ) {
			m_relightScene->addLight( relight::Light::createPointLight( affineTransform( transform ) * math::Vec3(), light->range(), light->color(), light->intensity() ) );
			continue;
		}

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

        float area = instance->m_mesh->m_mesh->area();
		maxArea	   = math::max2( maxArea, instance->m_mesh->m_mesh->area() );
        int   size = math::nextPowerOf2( ceil( k_LightmapMinSize + (k_LightmapMaxSize - k_LightmapMinSize) * (area / 170.0f) ) );

        totalLightmapPixels += size * size;

        instance->m_lm = m_relight->createLightmap( size, size );
        instance->m_pm = m_relight->createPhotonmap( size, size );

        relight::Mesh* m = m_relightScene->addMesh( instance->m_mesh->m_mesh, instance->m_transform, instance->m_mesh->m_material );
        m->setUserData( instance );

        instance->m_lm->addMesh( m );
        instance->m_pm->addMesh( m );
    }
    m_relightScene->end();

	printf( "%d instances added to relight scene, maximum mesh area %2.4f (%d lightmap pixels used, %d mb used)\n", m_relightScene->meshCount(), maxArea, totalLightmapPixels, totalLightmapPixels * sizeof( relight::Lumel ) / 1024 / 1024 );

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

    relight::Vec3 position = relight::Vec3( -transform->position()[0], transform->position()[1], transform->position()[2] );
    relight::Vec3 scale    = relight::Vec3( transform->scale()[0], transform->scale()[1], transform->scale()[2] );
    relight::Quat rotation = relight::Quat( -transform->rotation()[0], transform->rotation()[1], transform->rotation()[2], -transform->rotation()[3] );
	relight::Matrix4 R     = rotation;

	return affineTransform( transform->parent() ) * relight::Matrix4::translation( position ) * relight::Matrix4::scale( scale ) * R;

//	return affineTransform( transform->parent() ) * relight::Matrix4::translation( position ) * R * relight::Matrix4::scale( scale );
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

struct Camera {
	relight::Vec3	pos;
	relight::Vec3	view;
	relight::Vec3	right;
	relight::Vec3	up;

	float			yRotation;

	Camera( void ) : yRotation( 90 )
	{
		pos	  = relight::Vec3( 0, 1, 2 );
		right = relight::Vec3( 1, 0, 0 );
		up	  = relight::Vec3( 0, 1, 0 );
		view  = relight::Vec3( 0, -0.447213590, -0.894427180 );
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
		
		static relight::Vec2 mouse( -1, -1 );

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
			mouse = relight::Vec2( x, y );
		}
	}

	void rotate( float angle, const relight::Vec3& axis )
	{
		view = relight::Quat::rotateAroundAxis( angle, axis ).rotate( view );
	}
};

Camera gCamera;

// ** Lightmapping::renderBasis
void Lightmapping::renderBasis( const math::Vec3& origin, const math::Vec3& front, const math::Vec3& up, const math::Vec3& right )
{
	m_hal->setDepthTest( true, renderer::Always );
	m_hal->setShader( m_shaderColored );
	m_shaderColored->setMatrix( m_shaderColored->findUniformLocation( "u_mvp" ), m_matrixProj * m_matrixView * math::Matrix4::translation( origin ) );

	glBegin( GL_LINES );
		glColor3f( 1, 0, 0 ); glVertex3f( 0, 0, 0 ); glVertex3fv( &right.x );
		glColor3f( 0, 1, 0 ); glVertex3f( 0, 0, 0 ); glVertex3fv( &front.x );
		glColor3f( 0, 0, 1 ); glVertex3f( 0, 0, 0 ); glVertex3fv( &up.x );
	glEnd();

	m_hal->setDepthTest( true, renderer::Less );
}

// ** Lightmapping::handleUpdate
void Lightmapping::handleUpdate( platform::Window* window )
{
    if( !m_scene ) {
        return;
    }
    
    using namespace relight;

	if( platform::Input::sharedInstance()->keyDown( platform::Key::Escape ) ) platform::Application::sharedInstance()->quit();

	gCamera.update();

#if 1
	Matrix4 camera = affineTransform( const_cast<uscene::SceneObject*>( m_scene->findSceneObject( "Camera" ) )->transform() );
	Vec4    pos    = camera * Vec4( 0, 0, 0, 1 );
	Vec4	right  = camera * Vec4( -1, 0, 0, 0 );
	Vec4	up     = camera * Vec4( 0, 1, 0, 0 );
	Vec4	view   = camera * Vec4( 0, 0, 1, 0 );
#endif

//	uscene::Transform* camera1		= const_cast<uscene::SceneObject*>( m_scene->findSceneObject( "Camera" ) )->transform();
//	uscene::Transform* camera2		= const_cast<uscene::SceneObject*>( m_scene->findSceneObject( "Camera2" ) )->transform();
//	uscene::Transform* mainCamera	= const_cast<uscene::SceneObject*>( m_scene->findSceneObject( "Main Camera" ) )->transform();

//	m_matrixView = Matrix4::view( pos, view, up, right );
//	m_matrixView = Matrix4::view( pos, Vec3( 0, 0, 1 ), Vec3( 0, 1, 0 ), Vec3( 1, 0, 0 ) );
//	m_matrixView = Matrix4::view( gCamera.pos, gCamera.view, gCamera.up, gCamera.right );
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

//    m_hal->setFog( renderer::FogExp2, settings->fogDensity() * 7, fogColor, 0, 300 );
	m_hal->setShader( m_shaderLightmaped );

    {
        renderObjects( m_shaderLightmaped, m_solidRenderList );
    }

    {
        m_hal->setAlphaTest( renderer::Greater, 0.5f );
        renderObjects( m_shaderLightmaped, m_transparentRenderList );
        m_hal->setAlphaTest( renderer::CompareDisabled );
    }

    {
        m_hal->setDepthTest( false, renderer::Less );
        m_hal->setBlendFactors( renderer::BlendOne, renderer::BlendOne );

        renderObjects( m_shaderLightmaped, m_additiveRenderList );

        m_hal->setDepthTest( true, renderer::Less );
        m_hal->setBlendFactors( renderer::BlendDisabled, renderer::BlendDisabled );
    }

	for( int i = 0; i < m_relightScene->lightCount(); i++ ) {
		renderBasis( m_relightScene->light( i )->position() );
	}

	renderBasis();

	m_hal->setShader( NULL );

    m_hal->present();
}

// ** Lightmapping::renderObjects
void Lightmapping::renderObjects( renderer::Shader* shader, const uscene::SceneObjectArray& objects )
{
	using namespace relight;

    for( int i = 0; i < objects.size(); i++ ) {
        uscene::SceneObject* object   = objects[i];
        SceneMeshInstance*   instance = reinterpret_cast<SceneMeshInstance*>( object->userData() );

        if( !instance ) {
            continue;
        }

		Matrix4    mvp   = m_matrixProj * m_matrixView * instance->m_transform;
		const Rgb& color = instance->m_mesh->m_material->color();

		shader->setVec3( shader->findUniformLocation( "u_diffuseColor" ), Vec3( color.r, color.g, color.b ) );
		shader->setMatrix( shader->findUniformLocation( "u_mvp" ), mvp );
		shader->setInt( shader->findUniformLocation( "u_diffuse" ), 0 );
		shader->setInt( shader->findUniformLocation( "u_lightmap" ), 1 );

        const relight::Rgb& diffuse = instance->m_mesh->m_material->color();

		m_hal->setTexture( 0, instance->m_mesh->m_diffuse );
		m_hal->setTexture( 1, instance->m_lightmap );

        m_hal->setVertexBuffer( instance->m_mesh->m_vertexBuffer );
        m_hal->renderIndexed( renderer::PrimTriangles, instance->m_mesh->m_indexBuffer, 0, instance->m_mesh->m_indexBuffer->size() );

        if( instance->m_dirty ) {
            relight::Lightmap* lm = instance->m_lm;

			if( instance->m_lightmap == NULL ) {
				instance->m_lightmap = m_hal->createTexture2D( lm->width(), lm->height(), renderer::PixelRgb32F );
			}

			lm->save( "1.tga" );

            float* pixels = lm->toRgb32F();
            instance->m_lightmap->setData( 0, pixels );
            delete[]pixels;

            instance->m_dirty = false;
        }
    }

    m_hal->setVertexBuffer( NULL );
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