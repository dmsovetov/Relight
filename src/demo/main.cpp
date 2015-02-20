
//#include "monteCarloPathTracing.h"
//#include "PathTracer.h"
//#include "Model.h"
//#include "EmbreeModel.h"
#include "Mesh.h"
//#include "UVUnwrap.h"
//#include "Lightmapper.h"
#include "Lightmap.h"
//#include "LightmapCalculator.h"
//#include "PhotonMap.h"
#include "Radiosity.h"
//#include "DirectLightCalculator.h"

#include <GLUT/GLUT.h>

#ifdef WIN32
    #include <windows.h>
#endif

#include "TestRadiosity.h"
#include "TestLightmapper.h"

#include "camera.h"
#include "input.h"

double random0to1()
{
    static double invRAND_MAX = 1.0/RAND_MAX;
    return rand()*invRAND_MAX;
}

inline void Normalize(double *a)
{
    double length = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);

    if(!length) return;

    double inv_length = 1.0/length;

    a[0] *= inv_length;
    a[1] *= inv_length;
    a[2] *= inv_length;
}

void RotateAroundAxis(double *rotation_axis, double theta, double *vec_in, double *vec_out)
{
    // rotate "vec_in" around "rotation_axis" using quarternion

    Normalize(rotation_axis);

    double common_factor = sin(theta*0.5);

    double a = cos(theta*0.5);
    double b = rotation_axis[0] * common_factor;
    double c = rotation_axis[1] * common_factor;
    double d = rotation_axis[2] * common_factor;

    double mat[9] = { a*a+b*b-c*c-d*d,     2*(b*c-a*d),      2*(b*d+a*c),
        2*(b*c+a*d), a*a-b*b+c*c-d*d,      2*(c*d-a*b),
        2*(b*d-a*c),     2*(c*d+a*b),  a*a-b*b-c*c+d*d };

    for(int i = 0; i < 3; i++) vec_out[i] = 0.0;

    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++) vec_out[i] += mat[i*3+j] * vec_in[j];
    }

}


Camera    g_camera;

const Vector3 CAMERA_ACCELERATION(8.0f, 8.0f, 8.0f);
const float   CAMERA_FOVX = 90.0f;
const Vector3 CAMERA_POS(0.0f, 3.0f, 4.0f);
const float   CAMERA_SPEED_ROTATION = 0.2f;
const float   CAMERA_SPEED_FLIGHT_YAW = 100.0f;
const Vector3 CAMERA_VELOCITY(2.0f, 2.0f, 2.0f);
const float   CAMERA_ZFAR = 100.0f;
const float   CAMERA_ZNEAR = 0.1f;

// ** GL_ARB_multitexture
#ifdef WIN32
    PFNGLACTIVETEXTUREARBPROC			glActiveTextureARB				= NULL;
    PFNGLCLIENTACTIVETEXTUREARBPROC		glClientActiveTextureARB		= NULL;
#endif

void    UpdateCamera(float elapsedTimeSec);
void    ProcessUserInput();

#define CALCULATE_LUMELS		(1)

#define CALCULATE				(0)
#define CALCULATE_DIRECT		(1)
#define CALCULATE_PHOTONS		(0)
#define CALCULATE_RADIOSITY		(0)

//#define MAX_RADIOSITY_SAMPLES	(0)
#define MAX_LIGHTMAPS			(16)
#define MAX_THREADS				(1)

extern void PathTracing(double *eye, GLfloat *image);

GLint window_width = 1024, window_height = 768;
int				lmSize				= 512;
int				phnSize				= 64;
//const int		radSize				= 128;
int				blurPassCount		= 1;	// 3
int				photonPassCount		= 16;
//Model_OBJ		mesh;
//Model_OBJ		light;
IRayTracer		*model;

cRadiositySolver	*radiosity;
/*
cLightmapper		lm;
cLightmap			*direct[MAX_LIGHTMAPS];
cLightmap			*photons[MAX_LIGHTMAPS];
cPhotonMap			*photonMap[MAX_LIGHTMAPS];
TextureImage		directLightmaps[MAX_LIGHTMAPS];
TextureImage		photonLightmaps[MAX_LIGHTMAPS];
*/
float x = 0, y = 3, z = 3;

// **
cTest *test = NULL;

/*
struct sRadiositySample;

struct sFormFactor {
	sRadiositySample	*sample;
	float				scalar;
						sFormFactor( void ) : sample( NULL ), scalar( 0.0f ) {}

	static bool				Compare( const sFormFactor& a, const sFormFactor& b ) {
		return a.scalar > b.scalar;
	}
};

struct sRadiositySample {
	sVector3		position;
	sVector3		normal;
	sVector3		injectColor, directColor;
	sVector3		diffuse;
	sVector3		indirect;
	float			intensity;
	int				tx, ty;

	std::vector< sFormFactor >	formFactor;

					sRadiositySample( void ) : directColor( 0.0f, 0.0f, 0.0f ), injectColor( 0.0f, 0.0f, 0.0f ), indirect( 0.0f, 0.0f, 0.0f ), tx( 0 ), ty( 0 ) {}

	sFormFactor*	AddFormFactor( sRadiositySample& sample, float scalar ) {
		sFormFactor ff; ff.sample = &sample; ff.scalar = scalar;
		formFactor.push_back( ff );
		return NULL;
	}

	void			SetColor( unsigned char *pixels, const sVector3& color ) { 
		unsigned char *pixel = &pixels[ty * radSize * 3 + tx * 3];
		pixel[0] = color.x * 255;
		pixel[1] = color.y * 255;
		pixel[2] = color.z * 255;
	}

	sVector3		GetColor( unsigned char *pixels ) const { 
		unsigned char *pixel = &pixels[ty * radSize * 3 + tx * 3];
		return sVector3( pixel[0] / 255.0f, pixel[1] / 255.0f, pixel[2] / 255.0f );
	}

	bool			operator == ( const sRadiositySample& other ) const { return position == other.position; }
};

std::vector< sRadiositySample >	radiositySamples;
*/
/*
TextureImage	directLightmap;
TextureImage	indirectLightmap[8];
TextureImage	bluredLightmap;
*/
// ** Radiosity
/*GLuint			indirectDynLightmap, directDynLightmap;
//unsigned char	directLightmapPixels[radSize*radSize*3];
//unsigned char	indirectLightmapPixels[radSize*radSize*3];
float			directLightmapPixels[radSize*radSize*3];
float			indirectLightmapPixels[radSize*radSize*3];
cLightmap		*radDirect, *radIndirect;*/
// *********************

int				indirectTextureID = 0;
bool			renderDirect	= true;
bool			renderIndirect	= true;
bool			rotateScene		= false;
//bool			usePrevResults	= true;
//bool			linearFilter	= false;
//bool			blurLightmap	= false;

float			sampleX = 0.0f;
float			sampleZ = 0.0f;

/*
sLight lights[] = {
	{  1, 1.2,  0,   1.0f,  0.5f, 0.25f, 0.25f, 5, 0.05f, 64, true },
	{  0, 0.4,  1,   0.5f,  1.0f, 0.25f, 0.25f, 5, 0.05f, 64, true },
	{ -1, 0.6,  0,   0.25f, 0.5f, 1.0f,  0.25f, 5, 0.05f, 64, true },
	{ -1, 0.2, -1.5, 1.0f,  1.0f, 1.0f,  0.25f, 5, 0.05f, 64, true },
};
*/
/*
sLight lights[] = {
	{ -1, 0.2, -1.5,	0.8f, 0.8f, 0.8f,  1.0f, 5, 0.05f, 16, true },
};
*/
/*
sLight lights[] = { // !!
	{ -1, 0.2,	 -1.5,	0.25f, 0.5f, 1.0f,   4.0f, 5, 0.05f, 1, true },
	{  1.5, 2.5,  1.5,	1.0f,  0.5f, 0.25f,  4.0f, 5, 0.05f, 1, true },
};
*/
/*
sLight lights[] = {
	{ -0.5f, 2, -2.2f, 1.0f,  1.0f, 1.0f,  4.0f, 5, 0.05f, 64, true },
};
*/
/*
sLight lights[] = { // sponza light
	{  1.5, 2.5,  0,	1.0f,  0.5f, 0.25f,  4.0f, 5, 0.05f, 1, true },
};
*/
/*
sLight lights[] = {
	{ 0, 3, 0, 1.0f, 1.0f, 1.0f,  1.0f, 5, 0.05f, 16, true },
};
*/
IRayTracer *rayTracer = NULL;

float circleX = 100, circleY = 100;

void keyPressed (unsigned char key, int x, int y)
{
	if( test ) {
		test->KeyPressed( key );
	}
/*
	switch( key ) {
	case 'i': lights[0].x += 0.1; break;
	case 'k': lights[0].x -= 0.1; break;
	case 'j': lights[0].z -= 0.1; break;
	case 'l': lights[0].z += 0.1; break;
	case 'u': lights[0].y -= 0.1; break;
	case 'o': lights[0].y += 0.1; break;
	}
*/
}  

void windowReshapeFunc( GLint width, GLint height )
{
	glMatrixMode(GL_PROJECTION);
	glViewport(0, 0, width, height);
	GLfloat aspect = (GLfloat)width / height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(60, aspect, 0.25, 100);
    glMatrixMode(GL_MODELVIEW);
}

void display()
{
	static float g_rotationY = 0;
	static float g_rotationZ = 0;

	if( test ) {
		test->Update();
	}

//	glClearColor( 0.1, 0.2, 0.3, 1.0 );

    glClearColor( 0.52734375f, 0.8046875f, 0.91796875f, 1.0f ); // Sky blue
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

//    float fogColor[] = { 0.05, 0.1, 0.15, 1.0 };
//    float fogStart = 5;

    float fogColor[] = { 0.52734375f, 0.8046875f, 0.91796875f, 1.0f };
    float fogStart = 0.05f;

//    glEnable( GL_FOG );
    glFogi( GL_FOG_MODE, GL_LINEAR);
    glFogfv( GL_FOG_COLOR, fogColor );
    glFogf( GL_FOG_DENSITY, 0.2f );
    glFogf( GL_FOG_START, fogStart );
    glFogf( GL_FOG_END, 8 );

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&g_camera.getProjectionMatrix()[0][0]);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&g_camera.getViewMatrix()[0][0]);

//	glBindTexture( GL_TEXTURE_2D, renderDirect ? directLightmap.texID : indirectTextureID );
	glEnable( GL_TEXTURE_2D );
	glPushMatrix();

	// ** Test rendering
	if( test ) {
        test->Render( /*mesh, light*/ );
	}

	glPopMatrix();
/*
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
 */
//    if(k < N_SAMPLES) glutPostRedisplay();

#ifdef WIN32
    Mouse::instance().update();
    Keyboard::instance().update();
#endif
	UpdateCamera( 1.0f / 60.0f );
	ProcessUserInput();

    glutSwapBuffers();
	glutPostRedisplay();
}

bool LoadTGA(TextureImage *texture, char *filename)			// Loads A TGA File Into Memory
{    
	GLubyte		TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
	GLubyte		TGAcompare[12];								// Used To Compare TGA Header
	GLubyte		header[6];									// First 6 Useful Bytes From The Header
	GLuint		bytesPerPixel;								// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint		imageSize;									// Used To Store The Image Size When Setting Aside Ram
	GLuint		temp;										// Temporary Variable
	GLuint		type=GL_RGBA;								// Set The Default GL Mode To RBGA (32 BPP)

	FILE *file = fopen(filename, "rb");						// Open The TGA File

	if(	file==NULL ||										// Does File Even Exist?
		fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||	// Are There 12 Bytes To Read?
		memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0				||	// Does The Header Match What We Want?
		fread(header,1,sizeof(header),file)!=sizeof(header))				// If So Read Next 6 Header Bytes
	{
		if (file == NULL)									// Did The File Even Exist? *Added Jim Strong*
			return false;									// Return False
		else
		{
			fclose(file);									// If Anything Failed, Close The File
			return false;									// Return False
		}
	}

	texture->width  = header[1] * 256 + header[0];			// Determine The TGA Width	(highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2];			// Determine The TGA Height	(highbyte*256+lowbyte)
    
 	if(	texture->width	<=0	||								// Is The Width Less Than Or Equal To Zero
		texture->height	<=0	||								// Is The Height Less Than Or Equal To Zero
		(header[4]!=24 && header[4]!=32))					// Is The TGA 24 or 32 Bit?
	{
		fclose(file);										// If Anything Failed, Close The File
		return false;										// Return False
	}

	texture->bpp	= header[4];							// Grab The TGA's Bits Per Pixel (24 or 32)
	bytesPerPixel	= texture->bpp/8;						// Divide By 8 To Get The Bytes Per Pixel
	imageSize		= texture->width*texture->height*bytesPerPixel;	// Calculate The Memory Required For The TGA Data

	texture->imageData=(GLubyte *)malloc(imageSize);		// Reserve Memory To Hold The TGA Data

	if(	texture->imageData==NULL ||							// Does The Storage Memory Exist?
		fread(texture->imageData, 1, imageSize, file)!=imageSize)	// Does The Image Size Match The Memory Reserved?
	{
		if(texture->imageData!=NULL)						// Was Image Data Loaded
			free(texture->imageData);						// If So, Release The Image Data

		fclose(file);										// Close The File
		return false;										// Return False
	}

	for(GLuint i=0; i<int(imageSize); i+=bytesPerPixel)		// Loop Through The Image Data
	{														// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp=texture->imageData[i];							// Temporarily Store The Value At Image Data 'i'
		texture->imageData[i] = texture->imageData[i + 2];	// Set The 1st Byte To The Value Of The 3rd Byte
		texture->imageData[i + 2] = temp;					// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	fclose (file);											// Close The File

	// Build A Texture From The Data
	glGenTextures(1, &texture[0].texID);					// Generate OpenGL texture IDs

	glBindTexture(GL_TEXTURE_2D, texture[0].texID);			// Bind Our Texture
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// Linear Filtered
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	// Linear Filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	// Linear Filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);	// Linear Filtered
	glEnable( GL_GENERATE_MIPMAP_SGIS );
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
//	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	
	if (texture[0].bpp==24)									// Was The TGA 24 Bits
	{
		type=GL_RGB;										// If So Set The 'type' To GL_RGB
	}

//	glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, GL_UNSIGNED_BYTE, texture[0].imageData);
//	gluBuild2DMipmaps( GL_TEXTURE_2D, 3, texture[0].width, texture[0].height, GL_RGB, GL_FLOAT, texture[0].imageData );
	gluBuild2DMipmaps( GL_TEXTURE_2D, 3, texture[0].width, texture[0].height, GL_RGB, GL_UNSIGNED_BYTE, texture[0].imageData );

	return true;											// Texture Building Went Ok, Return True
}

struct sTriangle {
	relight::Vec3	A, B, C;
};

void Triangulate4( const sTriangle& face, sTriangle result[4] )
{
	// ** Source triangle
	const relight::Vec3& A = face.A;
	const relight::Vec3& B = face.B;
	const relight::Vec3& C = face.C;

	// ** Tesselation points
	relight::Vec3 xA = (A + B) * 0.5f;
	relight::Vec3 xB = (B + C) * 0.5f;
	relight::Vec3 xC = (C + A) * 0.5f;

	// ** Triangles
	sTriangle ta, tb, tc, td;

	// ** Triangle A
	ta.A =  A;
	ta.B = xA;
	ta.C = xC;

	// ** Triangle B
	tb.A = xA;
	tb.B =  B;
	tb.C = xB;

	// ** Triangle C
	tc.A = xC;
	tc.B = xB;
	tc.C =  C;

	// ** Triangle D
	td.A = xA;
	td.B = xB;
	td.C = xC;

	result[0] = ta;
	result[1] = tb;
	result[2] = tc;
	result[3] = td;
}

float TriangleArea( const relight::Vec3& A, const relight::Vec3& B, const relight::Vec3& C )
{
	float a = (A - B).length();
	float b = (B - C).length();
	float c = (C - A).length();
	float p = (a + b + c) * 0.5f;

	return sqrtf( p * (p - a) * (p - b) * (p - c) );
}
/*
void GenerateRadiositySamples( const sTriangle& triangle, const sVector3& N )
{
	if( TriangleArea( triangle.A, triangle.B, triangle.C ) > 0.1f ) {
		sTriangle result[4];
		Triangulate4( triangle, result );

		for( int i = 0; i < 4; i++ ) {
			GenerateRadiositySamples( result[i], N );
		}

		return;
	}

	sRadiositySample sample;

	sample.position		= triangle.A;
	sample.directColor	= sVector3( 0.0f, 0.0f, 0.0f );
	sample.normal		= N;
	radiositySamples.push_back( sample );

	sample.position		= triangle.B;
	sample.directColor	= sVector3( 0.0f, 0.0f, 0.0f );
	sample.normal		= N;
	radiositySamples.push_back( sample );

	sample.position		= triangle.C;
	sample.directColor	= sVector3( 0.0f, 0.0f, 0.0f );
	sample.normal		= N;
	radiositySamples.push_back( sample );
}

int formFactorCount = 0;

void CalculateFormFactor( sRadiositySample& A, sRadiositySample& B )
{
	sVector3 v = B.position - A.position;
	float	 r = v.normalize();

	float ba =   v * A.normal;
	float ab = -(v * B.normal);

	if( ba <= 0.0f || ab <= 0.0f ) {
		return;
	}

	if( ba > 0.01f ) A.AddFormFactor( B, ba / r );
	if( ab > 0.01f ) B.AddFormFactor( A, ab / r );
}

void RefineFormFactors( sRadiositySample& sample, int maxInfluences )
{
	if( sample.formFactor.empty() ) {
		return;
	}

	if( maxInfluences ) {
		while( sample.formFactor.size() > maxInfluences ) {
			sample.formFactor.erase( sample.formFactor.begin() + rand() % sample.formFactor.size() );
		}
	//	std::sort( sample.formFactor.begin(), sample.formFactor.end(), sFormFactor::Compare );
	//	sample.formFactor.erase( sample.formFactor.begin() + std::min( maxInfluences, ( int )sample.formFactor.size() ), sample.formFactor.end() );
	}

	// ** Normalize weights
	float weightSum = 0.0f;
	for( int i = 0, n = sample.formFactor.size(); i < n; i++ ) {
		weightSum += sample.formFactor[i].scalar;
	}

	for( int i = 0, n = sample.formFactor.size(); i < n; i++ ) {
		sample.formFactor[i].scalar /= weightSum;
	}

	formFactorCount += sample.formFactor.size();
}

int CalculateFormFactorCount( void )
{
	int count = radiositySamples.size();
	formFactorCount = 0;

	for( int i = 0; i < count; i++ ) {
		sRadiositySample& sample = radiositySamples[i];

		for( int j = 0; j < count; j++ ) {
			if( i == j ) continue;

			sRadiositySample& other = radiositySamples[j];
			sVector3 dir = other.position - sample.position;

		/*	// ** Trace scene
			sVector3 start = sample.position + dir * 0.01;
			sRayTraceResult result;
			if( model->TraceRay( start, other.position, &result ) >= 0 ) {
				continue;
			}

			CalculateFormFactor( sample, other );* /

		/*	// ** Intensity
			float r = dir.normalize();

			if( r ) {
				float Ai = dir * sample.normal;
				float Aj = -(dir * other.normal);

				if( Ai <= 0.0f || Aj <= 0.0f ) continue;

				float FF = (Ai * Aj) / 3.14159265358979323846f * (r * r);

				// ** Trace scene
				sVector3 start = sample.position + dir * 0.01;
				sRayTraceResult result;
				if( model->TraceRay( start, other.position, &result ) >= 0 ) {
					continue;
				}

				if( FF > 0.01f ) {
					sample.AddFormFactor( other, FF );
				}
			}* /

			
		/*	// ** Intensity
			float distance = dir.normalize();

			if( distance ) {
				float Ai = cDirectLightCalculator::CalculateLambertIntensity( dir, sample.normal );
			//	float Aj = cDirectLightCalculator::CalculateLambertIntensity( dir, other.normal );
				float FF = Ai / 3.141592f * (distance * distance);

				if( FF > 0.01f ) {
					sample.AddFormFactor( other, FF );
				}
			}* /
		

			// ** Intensity
			float r = dir.normalize();
			float intensity = cDirectLightCalculator::CalculateLambertIntensity( dir, sample.normal ) / r;

			if( intensity > 0.01f ) {
				// ** Trace scene
				sVector3 start = sample.position + dir * 0.01f;
				sRayTraceResult result;
				if( model->TraceRay( start, other.position, &result ) >= 0 ) {
					continue;
				}

				sample.AddFormFactor( other, intensity );
			}
		}

		RefineFormFactors( sample, MAX_RADIOSITY_SAMPLES );

		printf( "Form factor %d / %d\r", i, count );
	}

	return formFactorCount;
}

void GenerateRadiositySamples( cLightmap *lm )
{
	int w = lm->GetWidth();
	int h = lm->GetHeight();
	const sLumel *lumels = lm->GetLumels();

	for( int j = 0; j < h; j++ ) {
		for( int i = 0; i < w; i++ ) {
			const sLumel& l = lumels[j * w + i];

			if( !l.isValid ) {
				continue;
			}

			sRadiositySample sample;

			sample.position		= l.position;
			sample.directColor	= sVector3( 0.0f, 0.0f, 0.0f );
			sample.diffuse		= sVector3( l.color.r, l.color.g, l.color.b );
			sample.normal		= l.normal;
			sample.tx			= i;
			sample.ty			= j;
			radiositySamples.push_back( sample );
		}
	}
}

void GenerateRadiositySamples( const sMesh& mesh )
{
	for( int j = 0; j < mesh.totalFaces; j++ ) {
		int *face = &mesh.indices[j * 3];

		sTriangle triangle;
		triangle.A = mesh.vertices[ face[0] ].position;
		triangle.B = mesh.vertices[ face[1] ].position;
		triangle.C = mesh.vertices[ face[2] ].position;

		sVector3 N = sVector3( mesh.vertices[ face[0] ].normal ) +
					 sVector3( mesh.vertices[ face[1] ].normal ) + 
					 sVector3( mesh.vertices[ face[2] ].normal );
		N = N * (1.0f / 3.0f);

		GenerateRadiositySamples( triangle, N );
	}

	for( int i = 0; i < radiositySamples.size(); i++ ) {
		const sRadiositySample& r = radiositySamples[i];

		for( int j = 0; j < radiositySamples.size(); j++ ) {
			if( i == j ) continue;

			if( r == radiositySamples[j] ) {
				radiositySamples.erase( radiositySamples.begin() + j );
			}
		}
	}

	printf( "%d samples generated\n", radiositySamples.size() );
}
*/

int main(int argc, char *argv[])
{
	// ** Tracer
//	tracer.SetDirectLighting( true );
//	tracer.SetMaxDepth( 100 );
//	tracer.SetLightIntensityScale( 8 );
//	tracer.SetTracer( new cRayTracer );
//	tracer.SetTracer( model );

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);

    glutInitWindowPosition(300, 20);
    glutInitWindowSize(window_width, window_height);
    glutCreateWindow(argv[0]);
	glutSetWindowTitle( "Lightmapper" );
	glutKeyboardFunc(keyPressed);
	glutReshapeFunc(windowReshapeFunc);

    glMatrixMode(GL_PROJECTION);
	glViewport(0, 0, window_width, window_height);
	GLfloat aspect = (GLfloat) window_width / window_height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(60, aspect, 0.25, 100);
    glMatrixMode(GL_MODELVIEW);
    glShadeModel( GL_SMOOTH );
    glClearColor( 0.0f, 0.1f, 0.0f, 0.5f );
    glClearDepth( 1.0f );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
	glDisable( GL_CULL_FACE );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
	glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE );

#ifdef WIN32
	glActiveTextureARB		 = ( PFNGLACTIVETEXTUREARBPROC )wglGetProcAddress( "glActiveTextureARB" );
	glClientActiveTextureARB = ( PFNGLACTIVETEXTUREARBPROC )wglGetProcAddress( "glClientActiveTextureARB" );
#endif

	// ** Unwrap
//	cUVUnwrap	unwrap;
//	cLSCMMesh	_mesh;

//	_mesh.Load( "data/"filename".obj" );
 //	unwrap.Unwrap( &_mesh );

//	_mesh.Save( "data/"filename"_uv.obj" );
//	printf( "Loading model...\n" );
//	mesh.Load( "data/sponza_one_uv.obj" );
//    mesh.Load( "data/simple_scene_one_uv.obj" );
//    mesh.Load( "data/boxes_uv.obj" );
//	light.Load( "data/light.obj" );
/*
	// ** Concat mesh
	sMesh compoundMesh;
	for( int i = 0; i < mesh.meshes.size(); i++ ) {
		compoundMesh.totalFaces		+= mesh.meshes[i].totalFaces;
		compoundMesh.totalVertices	+= mesh.meshes[i].totalVertices;
	}

	compoundMesh.indices	= new int[compoundMesh.totalFaces * 3];
	compoundMesh.vertices	= new sMeshVertex[compoundMesh.totalVertices];

	sMeshVertex *vtx	  = compoundMesh.vertices;
	int			*idx	  = compoundMesh.indices;
	int			idxOffset = 0;
	for( int i = 0; i < mesh.meshes.size(); i++ ) {
		const sMesh& m = mesh.meshes[i];

		memcpy( vtx, m.vertices, sizeof( sMeshVertex ) * m.totalVertices );
		for( int j = 0; j < m.totalVertices; j++ ) {
			vtx[j].lightmapIndex = i;
		}

		for( int j = 0; j < m.totalFaces * 3; j++ ) {
			idx[j] = m.indices[j] + idxOffset;
		}

		idxOffset += m.totalVertices;

		vtx += m.totalVertices;
		idx += m.totalFaces * 3;
	}

	// ** Model
	printf( "Creating tracer...\n" );*/
/*
	model = new cEmbryModel;
    model->Create();
	model->AddMesh( compoundMesh );
*/
//	for( int i = 0; i < mesh.meshes.size(); i++ ) {
//		model->AddMesh( mesh.meshes[i] );
//	}
//	model->Create();
	rayTracer = model;

	// ** Create tests
//	test = new cTestRadiosity( lights, sizeof( lights ) / sizeof( sLight ) );
	test = new cTestLightmapper;
	test->Create();

	// ** Start
#ifdef WIN32
	Mouse::instance().attach( FindWindow( NULL, "Lightmapper" ) );
#endif
    
	// Setup camera.

    g_camera.perspective(CAMERA_FOVX,
        static_cast<float>(800) / static_cast<float>(600),
        CAMERA_ZNEAR, CAMERA_ZFAR);

    g_camera.setBehavior(Camera::CAMERA_BEHAVIOR_FLIGHT);
    g_camera.setPosition(CAMERA_POS);
    g_camera.setAcceleration(CAMERA_ACCELERATION);
    g_camera.setVelocity(CAMERA_VELOCITY);
    g_camera.lookAt( Vector3( 0, 0, 0 ) );
	/////////////////////////////////////////////////
/*
	char buffer[256];
	// ** Load lightmap
	sprintf( buffer, "output/lm-%d.tga", lmSize );
	LoadTGA( &directLightmap, buffer );

//	LoadTGA( &indirectLightmap[0], "output/lm-indirect-16.tga" );
//	LoadTGA( &indirectLightmap[1], "output/lm-indirect-32.tga" );
	LoadTGA( &indirectLightmap[0], "output/lm-indirect-64.tga" );
	LoadTGA( &indirectLightmap[1], "output/lm-indirect-128.tga" );
	LoadTGA( &indirectLightmap[2], "output/lm-indirect-256.tga" );
	LoadTGA( &indirectLightmap[3], "output/lm-indirect-512.tga" );
	LoadTGA( &indirectLightmap[4], "output/lm-indirect-1024.tga" );

	for( int i = 0; i < MAX_LIGHTMAPS; i++ ) {
		char lightmapFileName[256];
		sprintf( lightmapFileName, "output/lm%d-%d.tga", i, lmSize );
		LoadTGA( &directLightmaps[i], lightmapFileName );

		sprintf( lightmapFileName, "output/photons-filtered%d-blured-%d.tga", i, phnSize );
		LoadTGA( &photonLightmaps[i], lightmapFileName );
	}

	char photonFileName[256];
	sprintf( photonFileName, "output/photons-%d.tga", phnSize );
	LoadTGA( &indirectLightmap[5], photonFileName );

	sprintf( photonFileName, "output/photons-filtered-%d.tga", phnSize );
	LoadTGA( &indirectLightmap[6], photonFileName );

	indirectTextureID = directLightmap.texID;
	glEnable( GL_TEXTURE_2D );*/

    glutDisplayFunc(display);
    glutMainLoop();
    return 1;
}




void GetMovementDirection(Vector3 &direction)
{
#ifdef WIN32
    static bool moveForwardsPressed = false;
    static bool moveBackwardsPressed = false;
    static bool moveRightPressed = false;
    static bool moveLeftPressed = false;
    static bool moveUpPressed = false;
    static bool moveDownPressed = false;

    Vector3 velocity = g_camera.getCurrentVelocity();
    Keyboard &keyboard = Keyboard::instance();
    
    direction.set(0.0f, 0.0f, 0.0f);

    if (keyboard.keyDown(Keyboard::KEY_W))
    {
        if (!moveForwardsPressed)
        {
            moveForwardsPressed = true;
            g_camera.setCurrentVelocity(velocity.x, velocity.y, 0.0f);
        }

        direction.z += 1.0f;
    }
    else
    {
        moveForwardsPressed = false;
    }

    if (keyboard.keyDown(Keyboard::KEY_S))
    {
        if (!moveBackwardsPressed)
        {
            moveBackwardsPressed = true;
            g_camera.setCurrentVelocity(velocity.x, velocity.y, 0.0f);
        }

        direction.z -= 1.0f;
    }
    else
    {
        moveBackwardsPressed = false;
    }

    if (keyboard.keyDown(Keyboard::KEY_D))
    {
        if (!moveRightPressed)
        {
            moveRightPressed = true;
            g_camera.setCurrentVelocity(0.0f, velocity.y, velocity.z);
        }

        direction.x += 1.0f;
    }
    else
    {
        moveRightPressed = false;
    }

    if (keyboard.keyDown(Keyboard::KEY_A))
    {
        if (!moveLeftPressed)
        {
            moveLeftPressed = true;
            g_camera.setCurrentVelocity(0.0f, velocity.y, velocity.z);
        }

        direction.x -= 1.0f;
    }
    else
    {
        moveLeftPressed = false;
    }
    
    if (keyboard.keyDown(Keyboard::KEY_E))
    {
        if (!moveUpPressed)
        {
            moveUpPressed = true;
            g_camera.setCurrentVelocity(velocity.x, 0.0f, velocity.z);
        }

        direction.y += 1.0f;
    }
    else
    {
        moveUpPressed = false;
    }

    if (keyboard.keyDown(Keyboard::KEY_Q))
    {
        if (!moveDownPressed)
        {
            moveDownPressed = true;
            g_camera.setCurrentVelocity(velocity.x, 0.0f, velocity.z);
        }

        direction.y -= 1.0f;
    }
    else
    {
        moveDownPressed = false;
    }
#endif
}
float     g_cameraRotationSpeed = CAMERA_SPEED_ROTATION;


void UpdateCamera(float elapsedTimeSec)
{
#ifdef WIN32
    float heading = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;
    Vector3 direction;
    Mouse &mouse = Mouse::instance();

    GetMovementDirection(direction);

    switch (g_camera.getBehavior())
    {
    case Camera::CAMERA_BEHAVIOR_FIRST_PERSON:
        pitch = mouse.yDistanceFromWindowCenter() * g_cameraRotationSpeed;
        heading = -mouse.xDistanceFromWindowCenter() * g_cameraRotationSpeed;
        
        g_camera.rotate(heading, pitch, 0.0f);
        break;

    case Camera::CAMERA_BEHAVIOR_FLIGHT:
    //    heading = -direction.x * CAMERA_SPEED_FLIGHT_YAW * elapsedTimeSec;
        pitch = mouse.yDistanceFromWindowCenter() * g_cameraRotationSpeed;
	    roll = -mouse.xDistanceFromWindowCenter() * g_cameraRotationSpeed;
        
        g_camera.rotate(roll, pitch, 0);
        direction.x = 0.0f; // ignore yaw motion when updating camera velocity
        break;
    }

    g_camera.updatePosition(direction, elapsedTimeSec);

    mouse.moveToWindowCenter();
#else
//    g_camera.rotate(0.01f, 0.0f, 0);
#endif
}

void ProcessUserInput()
{
#ifdef WIN32
	Keyboard &keyboard = Keyboard::instance();

    if(keyboard.keyPressed(Keyboard::KEY_ESCAPE))
        exit( 0 );
#endif
}