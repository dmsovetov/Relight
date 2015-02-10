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
#include	"Lightmapper.h"
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
void cTestLightmapper::Create( IRayTracer *model, const Model_OBJ& _mesh )
{
//    createScene( "data/simple_scene_one_uv.obj" );
    createScene( "data/boxes_uv.obj" );

    m_progress = new BakingProgress( m_diffuse, &m_diffuseGl );

    m_data.m_scene      = m_scene;
    m_data.m_progress   = m_progress;
    pthread_create( &m_thread, NULL, worker, &m_data );

    renderDirect	= true;
    renderIndirect	= false;
    rotation        = 0.0f;
/*
    // **

	this->model		= model;

	memset( direct, 0, sizeof( direct ) );
	memset( indirect, 0, sizeof( indirect ) );
	memset( photonMap, 0, sizeof( photonMap ) );

	// ** Create lightmapper
	lightmapper = new cLightmapper;
	lightmapper->SetRayTracer( model );

	// ** Add faces to lightmapper
	for( int i = 0; i < mesh.meshes.size(); i++ ) {
		const sMesh& m = mesh.meshes[i];

		direct[i] = lightmapper->CreateLightmap();
        indirect[i] = lightmapper->CreateLightmap();

		for( int j = 0; j < m.totalFaces; j++ ) {
			int *face = &m.indices[j * 3];
			lightmapper->AddFace( i, &m, face[0], face[1], face[2], m.vertices );
		}

		// ** Save/load lumels
		char lumelsFileName[256];
		sprintf( lumelsFileName, "output/direct%d.lumels", i );

	#if CALCULATE_LUMELS && CALCULATE
		printf( "Creating lightmap for direct light...\n" );
		direct[i]->Create( i, DIRECT_LIGHTMAP_SIZE, DIRECT_LIGHTMAP_SIZE );
		direct[i]->SaveLumels( lumelsFileName );
	#else
		direct[i]->LoadLumels( lumelsFileName );
        indirect[i]->LoadLumels( lumelsFileName );
	#endif
	}

	// ** Add lights
	printf( "Adding lights...\n" );
	for( int i = 0, n = totalLights; i < n; i++ ) {
		lightmapper->AddLight( lights[i] );
	}

	// ** Bake
#if CALCULATE
	DWORD start = timeGetTime();
	#if CALCULATE_DIRECT
	CalculateDirectLight();
	#endif

	#if CALCULATE_INDIRECT
	CalculateIndirectLight();
	#endif
	printf( "Baked in %ds\n", (timeGetTime() - start) / 1000 );

    SaveLightmaps();
#else
    LoadLightmaps();
#endif

	float *indirectPixels = new float[INDIRECT_LIGHTMAP_SIZE*INDIRECT_LIGHTMAP_SIZE*3];
	float *directPixels	  = new float[DIRECT_LIGHTMAP_SIZE*DIRECT_LIGHTMAP_SIZE*3];

	// ** Upload pixels
	for( int i = 0; i < MAX_LIGHTMAPS; i++ ) {
		if( direct[i] ) {
			UpdatePixels( direct[i], directPixels );

			glGenTextures( 1, &texDirect[i] );
			glBindTexture( GL_TEXTURE_2D, texDirect[i] );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, DIRECT_LIGHTMAP_SIZE, DIRECT_LIGHTMAP_SIZE, 0, GL_RGB, GL_FLOAT, directPixels );
		}

		if( indirect[i] ) {
			UpdatePixels( indirect[i], indirectPixels );

			glGenTextures( 1, &texIndirect[i] );
			glBindTexture( GL_TEXTURE_2D, texIndirect[i] );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, INDIRECT_LIGHTMAP_SIZE, INDIRECT_LIGHTMAP_SIZE, 0, GL_RGB, GL_FLOAT, indirectPixels );
		}
	}

	delete[]indirectPixels;
	delete[]directPixels;
*/
}

void cTestLightmapper::createScene( const char* fileName )
{
    m_scene = Scene::create();

    // ** Add lights
    m_scene->begin();

    m_scene->addLight( PointLight::create( Vec3( -1.00f, 0.20f, -1.50f ), 5.0f, Color( 0.25f, 0.50f, 1.00f ), 1.0f, true ) );
    m_scene->addLight( PointLight::create( Vec3(  1.50f, 2.50f,  1.50f ), 5.0f, Color( 1.00f, 0.50f, 0.25f ), 1.0f, true ) );

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
    relight::TimeMeasure measure( "Bake" );
    data->m_scene->bake( BakeAll, data->m_progress );
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
void cTestLightmapper::Render( Model_OBJ& mesh, Model_OBJ& lightMesh )
{
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_CULL_FACE );
	glPushMatrix();

    glRotatef( rotation, 0, 1, 0 );
    glScalef( 0.7f, 0.7f, 0.7f );

    rotation += 0.25f;

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
        lightMesh.Draw( 0 );

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
	switch( key ) {
	case '9': renderIndirect	= !renderIndirect;	break;
	case '0': renderDirect		= !renderDirect;	break;
	}
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
