//
//  Filename:	TestRadiosity.cpp
//	Created:	01:06:2012   18:11

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"TestRadiosity.h"
//#include	"Lightmapper.h"
#include	"Lightmap.h"
#include	"Mesh.h"

#define		RADIOSITY_LIGHTMAP_SIZE	(256)
#define		MAX_RADIOSITY_SAMPLES	(0)

/*
=========================================================================================

			CODE

=========================================================================================
*/

extern bool LoadTGA( TextureImage *texture, char *filename );

#ifdef WIN32
extern PFNGLCLIENTACTIVETEXTUREARBPROC glActiveTextureARB;
#endif

// ** cTestRadiosity::Create
void cTestRadiosity::Create( IRayTracer *model, const Model_OBJ& mesh )
{
/*
	// ** Create lightmapper & dynamic indirect lightmap
	lightmapper = new cLightmapper;
	indirect	= lightmapper->CreateLightmap();

	lightmapper->SetCopyColorFromVertex( true );

	// ** Create lightmap pixels (for upload to GPU)
	LoadTGA( &diffuse, "data/bricks.tga" );

	indirectPixels	= new float[RADIOSITY_LIGHTMAP_SIZE*RADIOSITY_LIGHTMAP_SIZE*3];
	directPixels	= new float[RADIOSITY_LIGHTMAP_SIZE*RADIOSITY_LIGHTMAP_SIZE*3];

	memset( indirectPixels, 0, sizeof( float ) * RADIOSITY_LIGHTMAP_SIZE * RADIOSITY_LIGHTMAP_SIZE * 3 );
	memset( directPixels, 0, sizeof( float ) * RADIOSITY_LIGHTMAP_SIZE * RADIOSITY_LIGHTMAP_SIZE * 3 );

	// ** Add faces to lightmapper
	for( int i = 0; i < mesh.meshes.size(); i++ ) {
		const sMesh& m = mesh.meshes[i];

		for( int j = 0; j < m.totalFaces; j++ ) {
			int *face = &m.indices[j * 3];
			lightmapper->AddFace( i, &m, face[0], face[1], face[2], m.vertices );
		}

		indirect->Create( i, RADIOSITY_LIGHTMAP_SIZE, RADIOSITY_LIGHTMAP_SIZE );
	}

	// ** Create radiosity solver
	solver = new cRadiositySolver( model );
	solver->Create( indirect );

	// ** Precompute
	DWORD time = timeGetTime();
	int formFactors = solver->ComputeFormFactors( MAX_RADIOSITY_SAMPLES );
	printf( "%d form factors calculated in %dms\n", formFactors, timeGetTime() - time );

	useLinearFiltration = false;
	usePrevResults		= true;
	renderDirect		= true;
	renderIndirect		= true;
	renderSample		= false;
	blurTexture			= true;
	solve				= true;
	interpolate			= true;
    rotation            = 0.0f;*/
}

// ** cTestRadiosity::Update
void cTestRadiosity::Update( void )
{/*
	// ** Inject
	DWORD time = timeGetTime();
	solver->Clear();
	for( int i = 0, n = totalLights; i < n; i++ ) {
		const sLight& l = lights[i];
		solver->InjectPointLight( l.x, l.y, l.z, l.r, l.g, l.b, l.intensity, l.radius );
	}
	solver->UpdateDirectTexture( directPixels, RADIOSITY_LIGHTMAP_SIZE, RADIOSITY_LIGHTMAP_SIZE );
	if( usePrevResults ) {
		solver->InjectIndirect();
	}
	DWORD injectTime = timeGetTime() - time;

	// ** Propagate
	time = timeGetTime();
	if( solve ) {
		solver->Solve();
	}
	DWORD propagateTime = timeGetTime() - time;

	solver->UpdateIndirectTexture( indirectPixels, RADIOSITY_LIGHTMAP_SIZE, RADIOSITY_LIGHTMAP_SIZE );
	if( blurTexture ) {
		BlurLightmap();
		BlurLightmap();
	}

	static DWORD _propagate = -1;
	static DWORD _inject	= -1;

	if( _propagate == -1 )	_propagate	= propagateTime;
	if( _inject == -1 )		_inject		= injectTime;

	_propagate	+= propagateTime; _propagate /= 2;
	_inject		+= injectTime;	  _inject /= 2;

	char buffer[512];
	sprintf( buffer, "Inject: %d; Propagate: %d; Total: %d; Radiosity fps: %d;", _inject, _propagate, _inject + _propagate, (injectTime + propagateTime) ? 1000 / (injectTime + propagateTime) : 999 );
	glutSetWindowTitle( buffer );*/
}

// ** cTestRadiosity::Render
void cTestRadiosity::Render( void )
{
/*
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_CULL_FACE );
	glPushMatrix();

    glRotatef( rotation, 0, 1, 0 );
    glScalef( 0.7f, 0.7f, 0.7f );

    rotation += 0.25f;
	
	if( renderDirect ) {
		glActiveTextureARB( GL_TEXTURE1_ARB );
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, diffuse.texID );
		glMatrixMode( GL_TEXTURE );
		glLoadIdentity();
		glScalef( 10.0f, 10.0f, 10.0f );
		glMatrixMode( GL_MODELVIEW );
		glActiveTextureARB( GL_TEXTURE0_ARB );

		glBindTexture( GL_TEXTURE_2D, texDirect );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, RADIOSITY_LIGHTMAP_SIZE, RADIOSITY_LIGHTMAP_SIZE, 0, GL_RGB, GL_FLOAT, directPixels );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, useLinearFiltration ? GL_LINEAR : GL_NEAREST );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, useLinearFiltration ? GL_LINEAR : GL_NEAREST );

		for( int i = 0, n = mesh.meshes.size(); i < n; i++ ) {
			mesh.Draw( i );
		}

		glMatrixMode( GL_TEXTURE );
		glLoadIdentity();
		glMatrixMode( GL_MODELVIEW );
	}

	if( renderIndirect ) {
		if( renderDirect ) {
			glEnable( GL_BLEND );
			glBlendFunc( GL_ONE, GL_ONE );
			glDepthFunc( GL_LEQUAL );
		}

		glBindTexture( GL_TEXTURE_2D, texIndirect );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, RADIOSITY_LIGHTMAP_SIZE, RADIOSITY_LIGHTMAP_SIZE, 0, GL_RGB, GL_FLOAT, indirectPixels );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, useLinearFiltration ? GL_LINEAR : GL_NEAREST );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, useLinearFiltration ? GL_LINEAR : GL_NEAREST );
		for( int i = 0, n = mesh.meshes.size(); i < n; i++ ) {
			mesh.Draw( i );
		}
		glDisable( GL_BLEND );
	}

	glBindTexture( GL_TEXTURE_2D, 0 );

    // ** Render lights
    glDisable( GL_TEXTURE_2D );
    for( int i = 0, n = sizeof( lights ) / sizeof( sLight ); i < n; i++ ) {
        sLight l = lights[i];

        glPushMatrix();

        glColor3f( l.r, l.g, l.b );
        glTranslatef( lights[i].x, lights[i].y, lights[i].z );
        glScalef( 0.05, 0.05, 0.05 );
        light.Draw( 0 );
        glPopMatrix();
    }
    glColor3f( 1.0, 1.0, 1.0 );

	// ** Render nearest sample
	if( !nearest || !renderSample ) {
		return;
	}

	glBegin( GL_LINES );
	for( int i = 0, n = nearest->formFactors.size(); i < n; i++ ) {
		const sFormFactor&		ff	  = nearest->formFactors[i];
		const sRadiositySample&	other = solver->GetSample( ff.sample );

		const float scaleWeight = n;
		glColor3f( ff.weight * scaleWeight, ff.weight * scaleWeight, ff.weight * scaleWeight );
		glVertex3fv( &nearest->position.x );
		glVertex3fv( &other.position.x );
	}
	glEnd();
*/
}

// ** cTestRadiosity::KeyPressed
void cTestRadiosity::KeyPressed( int key )
{
	switch( key ) {
	case 't': usePrevResults		= !usePrevResults;		break;
	case 'y': useLinearFiltration	= !useLinearFiltration;	break;
	case 'q': solve					= !solve;				break;
	case 'x': solver->SetInterpolate( !solver->IsInterpolate() );	break;
	case '1': renderSample			= !renderSample;		break;
	case '9': renderIndirect		= !renderIndirect;		break;
	case '0': renderDirect			= !renderDirect;		break;
	case 'b': blurTexture			= !blurTexture;			break;
	}

	FindNearestSample();
}

// ** cTestRadiosity::FindNearestSample
void cTestRadiosity::FindNearestSample( void )
{
/*
	float	 distance = FLT_MAX;
	sVector3 L		  = sVector3( lights[0].x, lights[0].y, lights[0].z );

	for( int i = 0, n = solver->GetTotalSamples(); i < n; i++ ) {
		const sRadiositySample& sample = solver->GetSample( i );
		float d = (sample.position - L).length();

		if( d < distance ) {
			distance = d;
			nearest  = &sample;
		}
	}
*/
}

// ** cTestRadiosity::BlurLightmap
void cTestRadiosity::BlurLightmap( void )
{
	for( int y = 1; y < RADIOSITY_LIGHTMAP_SIZE - 1; y++ ) {
		for( int x = 1; x < RADIOSITY_LIGHTMAP_SIZE - 1; x++ ) {
			sColor color = sColor( 0.0f, 0.0f, 0.0f );

			for( int j = y - 1; j <= y + 1; j++ ) {
				for( int i = x - 1; i <= x + 1; i++ ) {
					color.r += indirectPixels[j * RADIOSITY_LIGHTMAP_SIZE * 3 + i * 3 + 0];
					color.g += indirectPixels[j * RADIOSITY_LIGHTMAP_SIZE * 3 + i * 3 + 1];
					color.b += indirectPixels[j * RADIOSITY_LIGHTMAP_SIZE * 3 + i * 3 + 2];
				}
			}

			color = color / 9.0f;

			indirectPixels[y * RADIOSITY_LIGHTMAP_SIZE * 3 + x * 3 + 0] = color.r;
			indirectPixels[y * RADIOSITY_LIGHTMAP_SIZE * 3 + x * 3 + 1] = color.g;
			indirectPixels[y * RADIOSITY_LIGHTMAP_SIZE * 3 + x * 3 + 2] = color.b;
		}
	}
}
