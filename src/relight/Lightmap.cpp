//
//  Filename:	Lightmap.h
//	Created:	22:03:2012   15:35

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"Lightmap.h"

/*
=========================================================================================

			CODE

=========================================================================================
*/

// ** cLightmap::Clear
void cLightmap::Clear( void )
{
	for( int j = 0; j < height; j++ ) {
		for( int i = 0; i < width; i++ ) {
			sLumel& lumel	= lumels[j*width + i];
			lumel.color		= sColor( 0, 0, 0 );
		}
	}
}

// ** cLightmap::Create
void cLightmap::Create( int _index, int _width, int _height )
{
	width  = _width;
	height = _height;
	index  = _index;
	lumels = new sLumel[width * height];

	for( int i = 0, n = ( int )lightmapper->faces.size(); i < n; i++ ) {
		CreateLumels( i, lightmapper->faces[i] );
	}
}

// ** cLightmap::Blur
void cLightmap::Blur( void )
{
	for( int y = 1; y < height - 1; y++ ) {
		for( int x = 1; x < width - 1; x++ ) {
			sColor color( 0, 0, 0 );
			int count = 0;

			for( int j = y - 1; j <= y + 1; j++ ) {
				for( int i = x - 1; i <= x + 1; i++ ) {
					const sLumel& lumel = lumels[j * width + i];

					if( lumel.IsValid() ) {
						color += lumel.color;
						count++;
					}
				}
			}

			if( count ) {
				lumels[y * width + x].color = color * (1.0 / count);
			}
		}
	}
}

// ** cLightmap::GetNearestColor
const sColor& cLightmap::GetNearestColor( int x, int y, int radius ) const
{
	if( radius > width || radius > height ) {
		return lumels[0].color;
	}

	// ** Search range
	int xmin = std::max( 0, x - radius );
	int xmax = std::min( width - 1, x + radius );
	int ymin = std::max( 0, y - radius );
	int ymax = std::min( height - 1, y + radius );

	// ** Lumel
	const sLumel *nearest = NULL;
	float distance = FLT_MAX;

	for( int j = ymin; j <= ymax; j++ ) {
		for( int i = xmin; i <= xmax; i++ ) {
			const sLumel& lumel = lumels[j * width + i];
			if( !lumel.IsValid() /*&& !lumel.IsExpanded()*/ ) {
				continue;
			}

			float d = sqrtf( (x - i) * (x - i) + (y - j) * (y - j) );
			if( d < distance ) {
				distance = d;
				nearest	 = &lumel;
			}
		}
	}

	if( !nearest ) {
		return GetNearestColor( x, y, radius + 1 );
	}

	return nearest->color;
}

// ** cLightmap::Expand
void cLightmap::Expand( void )
{
	for( int y = 0; y < height; y++ ) {
		for( int x = 0; x < width; x++ ) {
			sLumel& lumel = lumels[y * width + x];
			if( lumel.IsValid() ) {
				continue;
			}

			lumel.color = GetNearestColor( x, y, 1 );
			lumel.SetExpanded();
		}
	//	printf( "Expanding lightmap %d%%\r", int( float( y ) / height * 100 ) );
	}
//	printf( "\n" );
}

// ** cLightmap::CreateLumels
void cLightmap::CreateLumels( int faceIndex, const sLMFace& face )
{
	if( face.lightmap != index ) {
		return;
	}

	int uStart	= face.uvMin.x * width;
	int uEnd	= face.uvMax.x * width;
	int vStart	= face.uvMin.y * height;
	int vEnd	= face.uvMax.y * height;

	for( int v = vStart; v <= vEnd; v++ ) {
		for( int u = uStart; u <= uEnd; u++ ) {
			sLumel *lumel = &lumels[width * v + u];

			float _u = (u + 0.5f) / float( width );
			float _v = (v + 0.5f) / float( height );

			float bu, bv;
			if( !cLightmapper::IsUVInside( face.vertices[0].uv, face.vertices[1].uv, face.vertices[2].uv, _u, _v, &bu, &bv ) ) {
				continue;
			}

			CalculateLumel( faceIndex, face, lumel, u, v, bu, bv );
		}
	}
}

// ** cLightmap::CalculateLumel
void cLightmap::CalculateLumel( int faceIndex, const sLMFace& face, sLumel *lumel, double u, double v, double bu, double bv )
{
	lumel->position		= cLightmapper::TriangleInterpolate( face.vertices[0].position, face.vertices[1].position, face.vertices[2].position, bu, bv );
	lumel->normal		= cLightmapper::TriangleInterpolate( face.vertices[0].normal, face.vertices[1].normal, face.vertices[2].normal, bu, bv );
	lumel->color			= sColor( 0, 0, 0 );
	lumel->SetValid();
	lumel->faceIndex		= faceIndex;

	if( lightmapper->IsCopyColorFromVertex() ) {
		lumel->color		= face.vertices[0].color;
	}
}

// ** cLightmap::Save
void cLightmap::Save( const char *fileName ) const
{
	char buffer[256];

	FILE	*file;
	int		image_size = 0;

	sprintf( buffer, "%s-%d.tga", fileName, width );

	unsigned char tga_header_a[12]   = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	unsigned char tga_info_header[6] = { 0, 0, 0, 0, 0, 0 };

	file = fopen( buffer, "wb" );
	if( !file ) {
		return;
	}

	fwrite( tga_header_a, 1, sizeof( tga_header_a ), file );

	int channels = 3;

	tga_info_header[0] = width  % 256;
	tga_info_header[1] = width  / 256;
	tga_info_header[2] = height % 256;
	tga_info_header[3] = height / 256;
	tga_info_header[4] = channels * 8;
	tga_info_header[5] = 0;

	fwrite( tga_info_header, 1, sizeof( tga_info_header ), file );
	image_size = width * height * channels;

	for( int j = 0; j < height; j++ ) {
		for( int i = 0; i < width; i++ ) {
			const sLumel& lumel = lumels[j*width + i];

			unsigned char r = std::min( lumel.color.b * 255.0, 255.0 );
			unsigned char g = std::min( lumel.color.g * 255.0, 255.0 );
			unsigned char b = std::min( lumel.color.r * 255.0, 255.0 );

		//	unsigned char pixel[] = { r > 255 ? 255 : r, g > 255 ? 255 : g, b > 255 ? 255 : b };
            unsigned char pixel[] = { r, g, b };
			fwrite( pixel, sizeof( pixel ), 1, file );
		}
	}

	fclose( file );
}

// ** cLightmap::Load
void cLightmap::Load( const char *fileName )
{
	GLubyte		TGAheader[12] = {0,0,2,0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header
	GLubyte		TGAcompare[12];								// Used To Compare TGA Header
	GLubyte		header[6];									// First 6 Useful Bytes From The Header

	FILE *file = fopen( fileName, "rb" );						// Open The TGA File

	if(	file==NULL ||										// Does File Even Exist?
       fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||	// Are There 12 Bytes To Read?
       memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0				||	// Does The Header Match What We Want?
       fread(header,1,sizeof(header),file)!=sizeof(header))				// If So Read Next 6 Header Bytes
	{
        if (file == NULL) {									// Did The File Even Exist? *Added Jim Strong*
            printf( "cLightmap::Load : failed to load %s\n", fileName);
			return;									// Return False
        }
		else
		{
            printf( "cLightmap::Load : failed to load %s\n", fileName);
			fclose(file);									// If Anything Failed, Close The File
			return;									// Return False
		}
	}

    width  = header[1] * 256 + header[0];			// Determine The TGA Width	(highbyte*256+lowbyte)
    height = header[3] * 256 + header[2];			// Determine The TGA Height	(highbyte*256+lowbyte)

 	if( width <=0 || height	<=0	|| (header[4] != 24 && header[4] != 32) )
	{
		fclose(file);
		return;
	}

    // ** Read pixels
	int bytesPerPixel    = header[4] / 8;
	int imageSize		 = width * height * bytesPerPixel;
    unsigned char *image = new unsigned char[imageSize];
    fread( image, 1, imageSize, file );
    fclose( file );	

    // ** Create lumels
    for( int y = 0; y < height; y++ ) {
        for( int x = 0; x < width; x++ ) {
            unsigned char *pixel = &image[y * width * bytesPerPixel + x * bytesPerPixel];
            lumels[y * width + x].color = sColor( pixel[0] / 255.0f, pixel[1] / 255.0f, pixel[2] / 255.0f );
        }
    }

    delete[]image;
}

// ** cLightmap::SaveLumels
void cLightmap::SaveLumels( const char *fileName )
{
	FILE *file = fopen( fileName, "wb" );
    assert(file != NULL);

	fwrite( &width, sizeof( width ), 1, file );
	fwrite( &height, sizeof( height ), 1, file );
	fwrite( lumels, sizeof( sLumel ), width * height, file );
	fclose( file );
}

// ** cLightmap::LoadLumels
void cLightmap::LoadLumels( const char *fileName )
{
	FILE *file = fopen( fileName, "rb" );

	fread( &width, sizeof( width ), 1, file );
	fread( &height, sizeof( height ), 1, file );

	lumels = new sLumel[width * height];
	fread( lumels, sizeof( sLumel ), width * height, file );
	fclose( file );
}