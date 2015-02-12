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

#define		CALCULATE					(1)
#define		CALCULATE_LUMELS			(1)
#define		CALCULATE_DIRECT			(1)
#define		CALCULATE_INDIRECT			(1)

#define		DIRECT_LIGHTMAP_SIZE		(512)
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
    createScene( "data/simple_scene_one_uv.obj" );
//    createScene( "data/boxes_uv.obj" );

    m_progress = new BakingProgress( m_diffuse, &m_diffuseGl );

    const int kThreads = 8;
    for( int i = 0; i < kThreads; i++ ) {
        startThread( i, kThreads, IndirectLightSettings::production(), AmbientOcclusionSettings::production() );
    }

    m_rotation        = 0.0f;
}

void cTestLightmapper::startThread( int index, int threadCount, const IndirectLightSettings& indirectLight, const AmbientOcclusionSettings& ambientOcclusion )
{
    WorkerData* data = new WorkerData;
    pthread_t   thread;

    data->m_scene                   = m_scene;
    data->m_progress                = m_progress;
    data->m_startIndex              = index;
    data->m_step                    = threadCount;
    data->m_indirectLightSettings   = indirectLight;
    data->m_aoSettings              = ambientOcclusion;

    data->m_indirectLightSettings.m_photonPassCount /= threadCount;

    pthread_create( &thread, NULL, worker, data );
}

void cTestLightmapper::createScene( const char* fileName )
{
    m_scene = Scene::create();
    m_light = Mesh::createFromFile( "data/light.obj" );

    // ** Add lights
    m_scene->begin();

    if( true ) {
        m_scene->addLight( MeshLight::create( m_light, Vec3( -1.00f, 0.20f, -1.50f ), Color( 0.25f, 0.50f, 1.00f ), 1.0f, true ) );
        m_scene->addLight( MeshLight::create( m_light, Vec3(  1.50f, 2.50f,  1.50f ), Color( 1.00f, 0.50f, 0.25f ), 1.0f, true ) );
    } else {
        m_scene->addLight( PointLight::create( Vec3( -1.00f, 0.20f, -1.50f ), 5.0f, Color( 0.25f, 0.50f, 1.00f ), 1.0f, true ) );
        m_scene->addLight( PointLight::create( Vec3(  1.50f, 2.50f,  1.50f ), 5.0f, Color( 1.00f, 0.50f, 0.25f ), 1.0f, true ) );
    }

    // ** Add mesh
    Mesh* mesh      = Mesh::createFromFile( fileName );
    Mesh* instance  = m_scene->addMesh( mesh, Matrix4::translation( 0, 0, 0 ) );
    m_diffuse = m_scene->createLightmap( DIRECT_LIGHTMAP_SIZE, DIRECT_LIGHTMAP_SIZE );
    m_photons = m_scene->createPhotonmap( DIRECT_LIGHTMAP_SIZE, DIRECT_LIGHTMAP_SIZE );
    RelightStatus status = m_diffuse->addMesh( instance );
    m_photons->addMesh( instance );

    m_scene->end();

    m_diffuseGl = createTextureFromLightmap( m_diffuse );
}

void* cTestLightmapper::worker( void* userData )
{
    WorkerData* data = reinterpret_cast<WorkerData*>( userData );
//    Relight::bakeDirectLight( data->m_scene, data->m_progress, new bake::FaceBakeIterator( data->m_startIndex, data->m_step ) );
//    Relight::bakeIndirectLight( data->m_scene, data->m_progress, data->m_indirectLightSettings, new bake::FaceBakeIterator( data->m_startIndex, data->m_step ) );
    Relight::bakeAmbientOcclusion( data->m_scene, data->m_progress, data->m_aoSettings, new bake::FaceBakeIterator( data->m_startIndex, data->m_step ) );


    data->m_progress->notify( 1000, 0 );
}

unsigned int cTestLightmapper::createTextureFromLightmap( const relight::Lightmap* lightmap ) const
{
    unsigned int id;

    float* pixels = lightmap->toRgb32F();

    glGenTextures( 1, &id );
    glBindTexture( GL_TEXTURE_2D, id );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
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

    glRotatef( m_rotation, 0, 1, 0 );
    glScalef( 0.7f, 0.7f, 0.7f );

    m_rotation += 0.05f;

    for( int i = 0; i < m_scene->meshCount(); i++ ) {
        renderInstance( m_scene->mesh( i ) );
    }

    glBindTexture( GL_TEXTURE_2D, 0 );

    // ** Render lights
    glDisable( GL_TEXTURE_2D );
    for( int i = 0; i < m_scene->lightCount(); i++ ) {
        const relight::Light* light = m_scene->light( i );

        glPushMatrix();

        glColor3f( light->color().r, light->color().g, light->color().b );
        glTranslatef( light->position().x, light->position().y, light->position().z );
        glScalef( 0.05, 0.05, 0.05 );

        renderInstance( m_light );

        glPopMatrix();
    }
    glColor3f( 1.0, 1.0, 1.0 );

    glPopMatrix();

    if( m_progress->m_hasUpdates ) {
        float* pixels = m_diffuse->toRgb32F();
        glBindTexture( GL_TEXTURE_2D, m_diffuseGl );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_diffuse->width(), m_diffuse->height(), 0, GL_RGB, GL_FLOAT, pixels );
        glBindTexture( GL_TEXTURE_2D, 0 );
        delete[]pixels;

        m_progress->m_hasUpdates = false;
    }
}

void cTestLightmapper::renderInstance( const relight::Mesh* mesh ) const
{
    const relight::Vertex*  vertices = mesh->vertexBuffer();
    const relight::Index*   indices  = mesh->indexBuffer();

    glBindTexture( GL_TEXTURE_2D, m_diffuseGl );

    glEnableClientState(GL_VERTEX_ARRAY);						// Enable vertex arrays
    glEnableClientState(GL_NORMAL_ARRAY);						// Enable normal arrays
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);				// Enable texture coord arrays
    glEnableClientState(GL_COLOR_ARRAY);				// Enable texture coord arrays

    glVertexPointer( 3, GL_FLOAT, sizeof( relight::Vertex ), &vertices->m_position.x );				// Vertex Pointer to triangle array
    glNormalPointer( GL_FLOAT, sizeof( relight::Vertex ), &vertices->m_normal.x );						// Normal pointer to normal array
    glColorPointer( 3, GL_FLOAT, sizeof( relight::Vertex ), &vertices->m_color.r );						// Normal pointer to normal array

    glClientActiveTextureARB( GL_TEXTURE1_ARB );
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer( 2, GL_FLOAT, sizeof( relight::Vertex ), &vertices->m_uv[relight::Vertex::Lightmap].u );

    glClientActiveTextureARB( GL_TEXTURE0_ARB );
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer( 2, GL_FLOAT, sizeof( relight::Vertex ), &vertices->m_uv[relight::Vertex::Lightmap].u );

    glDrawElements( GL_TRIANGLES, mesh->indexCount(), GL_UNSIGNED_SHORT, indices );

    glDisableClientState(GL_VERTEX_ARRAY);						// Disable vertex arrays
    glDisableClientState(GL_NORMAL_ARRAY);						// Disable normal arrays
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);				// Disable texture coord arrays
    glDisableClientState(GL_COLOR_ARRAY);				// Enable texture coord arrays
}

// ** cTestLightmapper::KeyPressed
void cTestLightmapper::KeyPressed( int key )
{

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
