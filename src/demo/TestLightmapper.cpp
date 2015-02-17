//
//  Filename:	TestLightmapper.cpp
//	Created:	2:6:2012   20:10

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include    <Relight.h>

#include	"TestLightmapper.h"
//#include	"Lightmapper.h"
#include	"Lightmap.h"
//#include	"PhotonMap.h"
#include	"Mesh.h"
#include "mathlib.h"

#define		CALCULATE					(1)
#define		CALCULATE_LUMELS			(1)
#define		CALCULATE_DIRECT			(1)
#define		CALCULATE_INDIRECT			(1)

#define		DIRECT_LIGHTMAP_SIZE		(1024)
#define		INDIRECT_LIGHTMAP_SIZE		(512)
#define		INDIRECT_MAX_DEPTH			(3)
#define     INDIRECT_RADIUS             (7)
#define		INDIRECT_SAMPLES			(512)
#define		INDIRECT_INTERPOLATE		(false)
#define		PHOTON_PASS_COUNT			(32)

#define		LIGHTMAP_BLUR_PASS_COUNT	(0)
#define		LIGHTMAP_EXPAND				(0)

#define		MAX_THREADS					(4)


/*
=========================================================================================

			CODE

=========================================================================================
*/

using namespace relight;

// ** BakingProgress::notify
void BakingProgress::notify( int step, int stepCount )
{
    m_hasUpdates = true;
}

// ** cTestLightmapper::Create
void cTestLightmapper::Create( void )
{
    createScene();

    startLightmapsThread( IndirectLightSettings::fast(), AmbientOcclusionSettings::fast( 0.8f, 2.8f ) );

    m_rotationX = m_rotationY = 0.0f;
    m_useLightmaps = true;
}

void cTestLightmapper::startLightmapsThread( const IndirectLightSettings& indirectLight, const AmbientOcclusionSettings& ambientOcclusion )
{
    WorkerData* data = new WorkerData;
    pthread_t   thread;

    data->m_scene                   = m_scene;
    data->m_instances               = &m_instances;
    data->m_indirectLightSettings   = indirectLight;
    data->m_aoSettings              = ambientOcclusion;

    pthread_create( &thread, NULL, lightmapWorker, data );
}

void cTestLightmapper::startBakingThread( Instance* instance, int index, int threadCount, const IndirectLightSettings& indirectLight, const AmbientOcclusionSettings& ambientOcclusion )
{
    WorkerData* data = new WorkerData;
    pthread_t   thread;

    data->m_scene                   = m_scene;
    data->m_instance                = instance;
    data->m_startIndex              = index;
    data->m_step                    = threadCount;
    data->m_indirectLightSettings   = indirectLight;
    data->m_aoSettings              = ambientOcclusion;

    data->m_indirectLightSettings.m_photonPassCount /= threadCount;

    pthread_create( &thread, NULL, bakeWorker, data );
}

void cTestLightmapper::createScene( void )
{
    m_meshes[Mesh_Light]        = loadPrefab( "data/light.obj", NULL );
    m_meshes[Mesh_Ground]       = createGroundPlane( "data/textures/Ground07_D.tga", 10 );
    m_meshes[Mesh_Tomb05c]      = loadPrefab( "data/crypt/Tomb05_c.mesh", "data/textures/Tomb01_D.tga" );
    m_meshes[Mesh_Gravestone01] = loadPrefab( "data/crypt/Gravestone01.mesh", "data/textures/Gravestone01_D.tga" );

    m_scene = Scene::create();

    // ** Add lights
    m_scene->begin();

    if( true ) {
        m_scene->addLight( Light::createAreaLight( m_meshes[Mesh_Light].m_mesh, Vec3( -1.00f, 0.20f, -1.50f ), Color( 0.25f, 0.50f, 1.00f ), 2.0f, true ) );
        m_scene->addLight( Light::createAreaLight( m_meshes[Mesh_Light].m_mesh, Vec3(  1.50f, 2.50f,  1.50f ), Color( 1.00f, 0.50f, 0.25f ), 2.0f, true ) );
    }
    else if( false ) {
        m_scene->addLight( Light::createPointLight( Vec3( -1.00f, 0.20f, -1.50f ), 5.0f, Color( 0.25f, 0.50f, 1.00f ), 2.0f, true ) );
        m_scene->addLight( Light::createPointLight( Vec3(  1.50f, 2.50f,  1.50f ), 5.0f, Color( 1.00f, 0.50f, 0.25f ), 2.0f, true ) );

    //    m_scene->addLight( Light::createPointLight( Vec3( -1.00f, 0.20f, -1.50f ), 5.0f, Color( 0.25f, 0.50f, 1.00f ), 1.0f, true ) );
    //    m_scene->addLight( Light::createPointLight( Vec3(  1.50f, 2.50f,  1.50f ), 5.0f, Color( 1.00f, 0.50f, 0.25f ), 1.0f, true ) );
    //    m_scene->addLight( Light::createSpotLight( Vec3( 1.50f, 2.50f,  1.50f ), dir, 0.3f, 5.0f, Color( 1.00f, 0.50f, 0.25f ), 1.0f, true ) );
    }
    else if( true ) {
        Vec3 dir = Vec3( 0, 2, 0 ) - Vec3( 1.50f, 4.50f,  1.50f );
        dir.normalize();

        printf( "%f %f %f\n", dir.x, dir.y, dir.z );

        m_scene->addLight( Light::createDirectionalLight( dir, Color( 1.00f, 1.00f, 1.0f ), 3.0f, true ) );
    }
    else {
        m_scene->addLight( Light::createPointLight( Vec3( 3.00f, 4.20f, 2.0f ), 6.0f, Color( 1.0f, 1.0f, 1.00f ), 10.0f, true ) );
    }


    placeInstance( "tomb", m_meshes[Mesh_Tomb05c], relight::Matrix4::translation( 0, 0, 0 ), DIRECT_LIGHTMAP_SIZE );
    placeInstance( "ground", m_meshes[Mesh_Ground], relight::Matrix4::translation( 0, 0, 0 ), DIRECT_LIGHTMAP_SIZE );

    for( int i = -2; i <= 2; i++ ) {
        for( int j = -2; j <= 2; j++ ) {
            if( i == 0 && j == 0 ) {
                continue;
            }

            placeInstance( "gravestone_" + std::to_string( i ) + "_" + std::to_string( j ), m_meshes[Mesh_Gravestone01], relight::Matrix4::translation( i * 1.4f, 0, j * 1.4f ), 256 );
        }
    }

    m_scene->end();
}

Prefab cTestLightmapper::createGroundPlane( const char* diffuse, int size, const relight::Color& color ) const
{
    Prefab prefab;

    prefab.m_diffuse = createTextureFromFile( diffuse );
    prefab.m_mesh    = Mesh::create();

    VertexBuffer vertexBuffer;
    IndexBuffer  indexBuffer;

    // ** Vertices
    Vertex a;
    a.m_position                = Vec3( -size / 2, 0, -size / 2 );
    a.m_normal                  = Vec3( 0, 1, 0 );
    a.m_color                   = color;
    a.m_uv[Vertex::Lightmap]    = Uv( 0, 0 );
    a.m_uv[Vertex::Diffuse]     = Uv( 0, 0 );
    vertexBuffer.push_back( a );

    Vertex b;
    b.m_position                = Vec3(  size / 2, 0, -size / 2 );
    b.m_normal                  = Vec3( 0, 1, 0 );
    b.m_color                   = color;
    b.m_uv[Vertex::Lightmap]    = Uv( 0.99f, 0 );
    b.m_uv[Vertex::Diffuse]     = Uv( size, 0 );
    vertexBuffer.push_back( b );

    Vertex c;
    c.m_position                = Vec3(  size / 2, 0,  size / 2 );
    c.m_normal                  = Vec3( 0, 1, 0 );
    c.m_color                   = color;
    c.m_uv[Vertex::Lightmap]    = Uv( 0.99f, 0.99f );
    c.m_uv[Vertex::Diffuse]     = Uv( size, size );
    vertexBuffer.push_back( c );

    Vertex d;
    d.m_position                = Vec3( -size / 2, 0,  size / 2 );
    d.m_normal                  = Vec3( 0, 1, 0 );
    d.m_color                   = color;
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

    // ** Add buffer
    Texture* texture = Texture::createFromFile( diffuse );
    prefab.m_mesh->addFaces( vertexBuffer, indexBuffer, new TexturedMaterial( texture, Color( 1, 1, 1 ) ) );

    return prefab;
}

Prefab cTestLightmapper::loadPrefab( const char *fileName, const char *diffuse, const relight::Color& color )
{
    Prefab prefab;

    prefab.m_mesh       = strstr( fileName, ".obj" ) ? Mesh::createFromFile( fileName ) : loadMesh( fileName, diffuse, color );
    prefab.m_diffuse    = diffuse ? createTextureFromFile( diffuse ) : 0;

    return prefab;
}

Mesh* cTestLightmapper::loadMesh( const char* fileName, const char* diffuse, const relight::Color& color ) const
{
    FILE* file = fopen( fileName, "rb" );
    if( !file ) {
        return NULL;
    }

    Mesh* mesh = Mesh::create();
    int   kTmpStrLen = 1024;

    ::Matrix4 T;
    T.rotate( Vector3( 1, 0, 0 ), -90 );

    unsigned int magik = 0;
    unsigned int boneCount = 0;
    unsigned int submeshCount = 0;

    struct Str {
        static std::string read( FILE* file ) {
            unsigned int len;
            fread( &len, sizeof( int ), 1, file );
            std::string str;
            str.resize( len );
            fread( &str[0], len, 1, file );
            return str;
        }
    };

    fread(&magik, sizeof(unsigned int), 1, file); //magick

    // skeleton data
    fread(&boneCount, sizeof(unsigned int), 1, file);
    if( boneCount != 0 )
    {
        assert( false );
        return NULL;
    }

    // submesh count
    fread(&submeshCount, sizeof(unsigned int), 1, file);

    struct MeshVertex {
        Vec3 position;
        Vec3 normal;
        Vec3 tangent;
        Uv   texture;
        Uv   lightmap;
    };

    Material* material = new TexturedMaterial( Texture::createFromFile( diffuse ), color );

    for( unsigned int i = 0; i < submeshCount; ++i )
    {
        size_t       reads = 0;
        unsigned int indexCount = 0;
        unsigned int vertexCount = 0;
        unsigned int vertexStride = 0;

        std::string surfaceMeshName = Str::read( file );
        std::string surfaceMaterialName = Str::read( file );
        std::string vertexDeclStr = Str::read( file );

        reads = fread(&vertexCount,  sizeof(unsigned int), 1, file);  //vertex count
        reads = fread(&vertexStride, sizeof(unsigned int), 1, file);  //vertex stride
        reads = fread(&indexCount,   sizeof(unsigned int), 1, file);  //index count

        assert( sizeof( MeshVertex ) == vertexStride );
        MeshVertex* vertices = new MeshVertex[vertexCount];

        fread(vertices, vertexStride * vertexCount, 1, file);

        Index* indices = new Index[indexCount];
        fread( indices, sizeof(unsigned short) * indexCount, 1, file );

        VertexBuffer vertexBuffer;
        IndexBuffer  indexBuffer;

        for( int j = 0; j < vertexCount; j++ ) {
            Vector3 pos  = Vector3( vertices[j].position.x, vertices[j].position.y, vertices[j].position.z ) * T;
            Vector3 norm = Vector3( vertices[j].normal.x, vertices[j].normal.y, vertices[j].normal.z ) * T;

            Vertex v;
            v.m_position = Vec3( pos.x * 0.005f, pos.y * 0.005f, pos.z * 0.005f );
            v.m_normal = Vec3( norm.x, norm.y, norm.z );
            v.m_color = Color( 1, 1, 1 );
            v.m_uv[Vertex::Lightmap] = vertices[j].lightmap;
            v.m_uv[Vertex::Diffuse] = vertices[j].texture;
            vertexBuffer.push_back( v );
        }

        for( int j = 0; j < indexCount; j++ ) {
            indexBuffer.push_back( indices[j] );
        }

        mesh->addFaces( vertexBuffer, indexBuffer, material );
    }

    return mesh;
}

void* cTestLightmapper::lightmapWorker( void* userData )
{
    TimeMeasure measure( "All baked" );
    WorkerData* data = reinterpret_cast<WorkerData*>( userData );

    std::vector<pthread_t> threads(8);

    for( int i = 0; i < data->m_instances->size(); i++ ) {
        for( int j = 0; j < threads.size(); j++ ) {
            WorkerData* instanceData = new WorkerData;
            instanceData->m_scene                   = data->m_scene;
            instanceData->m_aoSettings              = data->m_aoSettings;
            instanceData->m_indirectLightSettings   = data->m_indirectLightSettings;
            instanceData->m_instances               = data->m_instances;
            instanceData->m_instance                = data->m_instances->at( i );
            instanceData->m_startIndex              = j;
            instanceData->m_step                    = threads.size();
            instanceData->m_indirectLightSettings.m_photonPassCount /= threads.size();

            pthread_create( &threads[j], NULL, bakeWorker, instanceData );
        }

        for( int j = 0; j < threads.size(); j++ ) {
            pthread_join( threads[j], NULL );
        }

        data->m_instances->at( i )->m_lightmap->save( ("data/lightmaps/" + data->m_instances->at( i )->m_name + ".tga").c_str() );
    }

    printf( "All baked!" );
}

void* cTestLightmapper::bakeWorker( void* userData )
{
    RelightStatus status;
    WorkerData*   data = reinterpret_cast<WorkerData*>( userData );

    status = Relight::bakeDirectLight( data->m_scene, data->m_instance->m_mesh, data->m_instance->m_progress, new bake::FaceBakeIterator( data->m_startIndex, data->m_step ) );
    assert( status == RelightSuccess );

    status = Relight::bakeIndirectLight( data->m_scene, data->m_instance->m_mesh, data->m_instance->m_progress, data->m_indirectLightSettings, new bake::FaceBakeIterator( data->m_startIndex, data->m_step ) );
    assert( status == RelightSuccess );

//    Relight::bakeAmbientOcclusion( data->m_scene, data->m_instance->m_mesh, data->m_instance->m_progress, data->m_aoSettings, new bake::FaceBakeIterator( data->m_startIndex, data->m_step ) );

    data->m_instance->m_lightmap->expand();

    data->m_instance->m_photons->save( "output/photons/" + data->m_instance->m_name + ".tga" );

    data->m_instance->m_progress->notify( 1000, 0 );
}

unsigned int cTestLightmapper::createTextureFromLightmap( const relight::Lightmap* lightmap ) const
{
    unsigned int id;

    float* pixels = lightmap->toRgb32F();

    glGenTextures( 1, &id );
    glBindTexture( GL_TEXTURE_2D, id );
    glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, lightmap->width(), lightmap->height(), 0, GL_RGB, GL_FLOAT, pixels );

    delete[]pixels;

    return id;
}

// ** cTestLightmapper::Render
void cTestLightmapper::Render( void )
{
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_CULL_FACE );
	glPushMatrix();

    glRotatef( m_rotationY, 0, 1, 0 );
    glRotatef( m_rotationX, 1, 0, 0 );
    glScalef( 0.9f, 0.9f, 0.9f );
/*
    static std::vector<Vec3> vertices;

    if( vertices.empty() ) {
        Vec3   dir    = Vec3( -0.457496, -0.762493, -0.457496 ); dir.normalize();
        Bounds bounds = m_scene->bounds();
        Plane  plane( dir, dir * 2 );

        for( int i = 0; i < 10000; i++ ) {
            Vec3 v = bounds.randomPointInside();
            vertices.push_back( plane * v );
        }
    }

    glPointSize( 2 );
    glBegin( GL_POINTS );
    for( int i = 0; i < vertices.size(); i++ ) {
        glColor3f( 1.0f, 1.0f, 0.0f );
        glVertex3fv( &vertices[i].x );
    }
    glEnd();
*/

    for( int i = 0; i < m_instances.size(); i++ ) {
        renderInstance( m_instances[i] );
    }


    glActiveTextureARB( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, 0 );

    glActiveTextureARB( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, 0 );

    // ** Render lights
    glDisable( GL_TEXTURE_2D );
    for( int i = 0; i < m_scene->lightCount(); i++ ) {
        const relight::Light* light = m_scene->light( i );

        glPushMatrix();

        glColor3f( light->color().r, light->color().g, light->color().b );
        glTranslatef( light->position().x, light->position().y, light->position().z );
        glScalef( 0.05, 0.05, 0.05 );

    //    renderInstance( m_light, 0, 0 );

        glPopMatrix();
    }
    glColor3f( 1.0, 1.0, 1.0 );

    glDisable( GL_DEPTH_TEST );
    glBegin( GL_LINES );
    glColor3f( 1, 0, 0 ); glVertex3f( 0, 0, 0 ); glVertex3f( 1, 0, 0 );
    glColor3f( 0, 1, 0 ); glVertex3f( 0, 0, 0 ); glVertex3f( 0, 1, 0 );
    glColor3f( 0, 0, 1 ); glVertex3f( 0, 0, 0 ); glVertex3f( 0, 0, 1 );
    glEnd();
    glEnable( GL_DEPTH_TEST );

    glPopMatrix();

    for( int i = 0; i < m_instances.size(); i++ ) {
        Instance* instance = m_instances[i];

        if( !instance->m_progress->m_hasUpdates ) {
            continue;
        }

        float* pixels = instance->m_lightmap->toRgb32F();
        glBindTexture( GL_TEXTURE_2D, instance->m_lightmapId );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, instance->m_lightmap->width(), instance->m_lightmap->height(), 0, GL_RGB, GL_FLOAT, pixels );
        glBindTexture( GL_TEXTURE_2D, 0 );
        delete[]pixels;

        instance->m_progress->m_hasUpdates = false;
    }
}

// ** cTestLightmapper::createTextureFromFile
unsigned int cTestLightmapper::createTextureFromFile( const char* fileName ) const
{
    Texture* texture = Texture::createFromFile( fileName );
    if( !texture ) {
        return 0;
    }

    unsigned int id;

    glGenTextures( 1, &id );
    glBindTexture( GL_TEXTURE_2D, id );
    glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, texture->width(), texture->height(), 0, GL_RGB, GL_FLOAT, &texture->pixels()[0].r );

    delete texture;
    return id;
}

void cTestLightmapper::renderInstance( const Instance* instance ) const
{
    const relight::Vertex*  vertices = instance->m_mesh->vertexBuffer();
    const relight::Index*   indices  = instance->m_mesh->indexBuffer();

    glEnableClientState(GL_VERTEX_ARRAY);						// Enable vertex arrays
    glEnableClientState(GL_NORMAL_ARRAY);						// Enable normal arrays
    glEnableClientState(GL_COLOR_ARRAY);				// Enable texture coord arrays

    glVertexPointer( 3, GL_FLOAT, sizeof( relight::Vertex ), &vertices->m_position.x );				// Vertex Pointer to triangle array
    glNormalPointer( GL_FLOAT, sizeof( relight::Vertex ), &vertices->m_normal.x );						// Normal pointer to normal array
    glColorPointer( 3, GL_FLOAT, sizeof( relight::Vertex ), &vertices->m_color.r );						// Normal pointer to normal array

    if( instance->m_lightmapId && m_useLightmaps ) {
        glActiveTextureARB( GL_TEXTURE1_ARB );
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, instance->m_lightmapId );

        glClientActiveTextureARB( GL_TEXTURE1_ARB );
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer( 2, GL_FLOAT, sizeof( relight::Vertex ), &vertices->m_uv[relight::Vertex::Lightmap].u );
    } else {
        glActiveTextureARB( GL_TEXTURE1_ARB );
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable( GL_TEXTURE_2D );
    }

    if( instance->m_diffuseId ) {
        glActiveTextureARB( GL_TEXTURE0_ARB );
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, instance->m_diffuseId );

        glClientActiveTextureARB( GL_TEXTURE0_ARB );
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer( 2, GL_FLOAT, sizeof( relight::Vertex ), &vertices->m_uv[relight::Vertex::Diffuse].u );
    } else {
        glActiveTextureARB( GL_TEXTURE0_ARB );
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable( GL_TEXTURE_2D );
    }

    glDrawElements( GL_TRIANGLES, instance->m_mesh->indexCount(), GL_UNSIGNED_SHORT, indices );

    glDisableClientState(GL_VERTEX_ARRAY);						// Disable vertex arrays
    glDisableClientState(GL_NORMAL_ARRAY);						// Disable normal arrays
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);       // Disable texture coord arrays
    glDisableClientState(GL_COLOR_ARRAY);				// Enable texture coord arrays
}

Instance* cTestLightmapper::placeInstance( const std::string& name, const Prefab& prefab, const relight::Matrix4& T, int lightmapSize )
{
    Instance* instance      = new Instance;
    instance->m_name        = name;
    instance->m_mesh        = m_scene->addMesh( prefab.m_mesh, T );
    instance->m_lightmap    = m_scene->createLightmap( lightmapSize, lightmapSize );
    instance->m_photons     = m_scene->createPhotonmap( lightmapSize, lightmapSize );
    instance->m_lightmapId  = createTextureFromLightmap( instance->m_lightmap );
    instance->m_diffuseId   = prefab.m_diffuse;
    instance->m_progress    = new BakingProgress( instance );

    instance->m_lightmap->addMesh( instance->m_mesh );
    instance->m_photons->addMesh( instance->m_mesh );

    instance->m_lightmap->save( "output/lm.tga" );

    m_instances.push_back( instance );

    return instance;
}

// ** cTestLightmapper::KeyPressed
void cTestLightmapper::KeyPressed( int key )
{
    if( key == 'q' ) m_rotationY += 2.8f;
    if( key == 'e' ) m_rotationY -= 2.8f;

    if( key == 'w' ) m_rotationX += 2.8f;
    if( key == 's' ) m_rotationX -= 2.8f;

    if( key == 'l' ) m_useLightmaps = !m_useLightmaps;
}

// ** cTestLightmapper::CalculateDirectLight
void cTestLightmapper::CalculateDirectLight( void )
{
/*
	printf( "Calculating direct lighting...\n" );
	DWORD start = timeGetTime();

	sLightmapCalculation *calc[MAX_THREADS];

	// ** Threads
	for( int k = 0; k < MAX_LIGHTMAPS; k++ ) {
		if( !direct[k] ) {
			continue;
		}

		printf( "\tLightmap %d\n", k );
		for( int i = 0; i < MAX_THREADS; i++ ) {
			calc[i] = lightmapper->CalculateDirectLight( direct[k], i, MAX_THREADS );
		}
		for( int i = 0; i < MAX_THREADS; ) {
			if( calc[i]->isReady ) i++;
		}

		for( int i = 0; i < LIGHTMAP_BLUR_PASS_COUNT; i++ ) {
			if( direct[k] ) {
				direct[k]->Blur();
			}
		}
	}

	printf( "Direct light for %d lights in resolution %dx%d calculated in %ds\n", totalLights, DIRECT_LIGHTMAP_SIZE, DIRECT_LIGHTMAP_SIZE, (timeGetTime() - start) / 1000 );

#if LIGHTMAP_EXPAND
	for( int k = 0; k < MAX_LIGHTMAPS; k++ ) {
		if( !direct[k] ) {
			continue;
		}

		DWORD time = timeGetTime();
		direct[k]->Expand();
		printf( "Expanded in %dms\n", timeGetTime() - time );
	}
#endif*/
}

// ** cTestLightmapper::CalculateIndirectLight
void cTestLightmapper::CalculateIndirectLight( void )
{
/*
	for( int k = 0; k < MAX_LIGHTMAPS; k++ ) {
		if( !direct[k] ) {
			continue;
		}

		photonMap[k] = lightmapper->CreatePhotonMap();
		photonMap[k]->Create( lightmapper, INDIRECT_LIGHTMAP_SIZE, INDIRECT_LIGHTMAP_SIZE, INDIRECT_MAX_DEPTH, INDIRECT_SAMPLES, INDIRECT_RADIUS, INDIRECT_INTERPOLATE );
	}

	printf( "Calculating photon map...\n" );
	DWORD start = timeGetTime();
	for( int j = 0; j < PHOTON_PASS_COUNT; j++ ) {
		for( int i = 0, n = totalLights; i < n; i++ ) {
			photonMap[0]->Calculate( &lights[i] );
		}
		printf( "Pass %d / %d\r", j + 1, PHOTON_PASS_COUNT );
	}
	printf( "Calculated in %ds, %d photons stored\n", (timeGetTime() - start) / 1000, photonMap[0]->GetTotalPhotons() );

	for( int k = 0; k < MAX_LIGHTMAPS; k++ ) {
		if( !direct[k] ) {
			continue;
		}

		indirect[k] = lightmapper->CreateLightmap();
		indirect[k]->Create( k, INDIRECT_LIGHTMAP_SIZE, INDIRECT_LIGHTMAP_SIZE );
	}

	for( int k = 0; k < MAX_LIGHTMAPS; k++ ) {
		if( !direct[k] ) {
			continue;
		}

		photonMap[k]->ApplyTo( indirect[k], lightmapper );

    #if LIGHTMAP_EXPAND
		indirect[k]->Expand();
    #endif

		for( int i = 0; i < LIGHTMAP_BLUR_PASS_COUNT; i++ ) {
			indirect[k]->Blur();
		}
	}
*/
}

// ** cTestLightmapper::UpdatePixels
void cTestLightmapper::UpdatePixels( const cLightmap *lightmap, float *pixels )
{
/*
	int width  = lightmap->GetWidth();
	int height = lightmap->GetHeight();
	const sLumel *lumels = lightmap->GetLumels();

	for( int y = 0; y < height; y++ ) {
		for( int x = 0; x < width; x++ ) {
			const sLumel& lumel = lumels[y * width + x];
			float *pixel = &pixels[y * width * 3 + x * 3];

			pixel[0] = lumel.color.r;
			pixel[1] = lumel.color.g;
			pixel[2] = lumel.color.b;
		}
	}
*/
}

// ** cTestLightmapper::SaveLightmaps
void cTestLightmapper::SaveLightmaps( void )
{
/*
	char buffer[512];

	for( int i = 0; i < MAX_LIGHTMAPS; i++ ) {
		if( photonMap[i] ) {
			sprintf( buffer, "output/photons%d", i );
			photonMap[i]->Debug_Save( buffer );
		}

		if( direct[i] ) {
			sprintf( buffer, "output/direct-%d", i );
			direct[i]->Save( buffer );
		}

		if( indirect[i] ) {
			sprintf( buffer, "output/indirect-%d", i );
			indirect[i]->Save( buffer );
		}
	}
*/
}

// ** cTestLightmapper::LoadLightmaps
void cTestLightmapper::LoadLightmaps( void )
{
/*
	char buffer[512];

	for( int i = 0; i < MAX_LIGHTMAPS; i++ ) {

		if( direct[i] ) {
			sprintf( buffer, "output/direct-%d-%d.tga", i, DIRECT_LIGHTMAP_SIZE );
			direct[i]->Load( buffer );
		}

		if( indirect[i] ) {
			sprintf( buffer, "output/indirect-%d-%d.tga", INDIRECT_INTERPOLATE, INDIRECT_LIGHTMAP_SIZE );
			indirect[i]->Load( buffer );
		}
	}
*/
}
