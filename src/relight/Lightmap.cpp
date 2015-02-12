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

#include "BuildCheck.h"

#include "Lightmap.h"
#include "scene/Mesh.h"

namespace relight {

// ------------------------------------------------ Lightmap ------------------------------------------------ //

// ** Lightmap::Lightmap
Lightmap::Lightmap( int width, int height ) : m_width( width ), m_height( height )
{
    m_lumels.resize( width * height );
}

// ** Lightmap::width
int Lightmap::width( void ) const
{
    return m_width;
}

// ** Lightmap::height
int Lightmap::height( void ) const
{
    return m_height;
}

// ** Lightmap::lumels
Lumel* Lightmap::lumels( void )
{
    return &m_lumels[0];
}

// ** Lightmap::lumel
Lumel& Lightmap::lumel( const Uv& uv )
{
    return lumel( uv.u * m_width, uv.v * m_height );
}

// ** Lightmap::lumel
const Lumel& Lightmap::lumel( const Uv& uv ) const
{
    return lumel( uv.u * m_width, uv.v * m_height );
}

// ** Lightmap::lumel
Lumel& Lightmap::lumel( int x, int y )
{
    assert( x >= 0 && x < m_width && y >= 0 && y < m_height );
    return m_lumels[y * m_width + x];
}

// ** Lightmap::lumel
const Lumel& Lightmap::lumel( int x, int y ) const
{
    assert( x >= 0 && x < m_width && y >= 0 && y < m_height );
    return m_lumels[y * m_width + x];
}

// ** Lightmap::addMesh
RelightStatus Lightmap::addMesh( const Mesh* mesh, bool copyVertexColor )
{
    if( mesh->lightmap() ) {
        return RelightInvalidCall;
    }

    // ** Attach this lightmap to an instance
    const_cast<Mesh*>( mesh )->setLightmap( this );

    // ** Initialize lumels
    initializeLumels( mesh, copyVertexColor );

    return RelightSuccess;
}

// ** Lightmap::initializeLumels
void Lightmap::initializeLumels( const Mesh* mesh, bool copyVertexColor )
{
    // ** For each face in a sub mesh
    for( int i = 0, n = mesh->faceCount(); i < n; i++ ) {
        initializeLumels( mesh->face( i ), copyVertexColor );
    }
}

// ** Lightmap::initializeLumels
void Lightmap::initializeLumels( const Face& face, bool copyVertexColor )
{
    Uv   min, max;

    // ** Calculate UV bounds
    face.uvRect( min, max );

    int uStart = min.u * m_width;
    int uEnd   = max.u * m_width;
    int vStart = min.v * m_height;
    int vEnd   = max.v * m_height;

    // ** Initialize lumels
    for( int v = vStart; v <= vEnd; v++ ) {
        for( int u = uStart; u <= uEnd; u++ ) {
            Lumel& lumel = m_lumels[m_width * v + u];

            Uv uv( (u + 0.5f) / float( m_width ), (v + 0.5f) / float( m_height ) );
            Barycentric barycentric;

            if( !face.isUvInside( uv, barycentric, Vertex::Lightmap ) ) {
                continue;
            }

            initializeLumel( lumel, face, barycentric, copyVertexColor );
        }
    }
}

// ** Lightmap::initializeLumel
void Lightmap::initializeLumel( Lumel& lumel, const Face& face, const Uv& barycentric, bool copyVertexColor )
{
    lumel.m_faceIdx     = face.faceIdx();
    lumel.m_position    = face.positionAt( barycentric );
    lumel.m_normal      = face.normalAt( barycentric );
    lumel.m_color       = copyVertexColor ? face.vertex( 0 )->m_color : Color( 0, 0, 0 );
    lumel.m_flags       = lumel.m_flags | Lumel::Valid;
}

// ** Lightmap::expand
void Lightmap::expand( void )
{
    for( int y = 1; y < m_height - 1; y++ ) {
        for( int x = 1; x < m_width - 1; x++ ) {
            Lumel& current = m_lumels[y * m_width + x];
            if( !current ) {
                continue;
            }

            fillInvalidAt( x - 1, y - 1, current.m_color );
            fillInvalidAt( x - 1, y + 1, current.m_color );
            fillInvalidAt( x + 1, y - 1, current.m_color );
            fillInvalidAt( x + 1, y + 1, current.m_color );
        }
    }
}

// ** Lightmap::fillInvalidAt
void Lightmap::fillInvalidAt( int x, int y, const Color& color )
{
    Lumel& l = lumel( x, y );

    if( l ) {
        return;
    }

    l.m_color = color;
}

// ** Lightmap::blur
void Lightmap::blur( void )
{
    for( int y = 1; y < m_height - 1; y++ ) {
        for( int x = 1; x < m_width - 1; x++ ) {
            Color color( 0, 0, 0 );
            int count = 0;

            for( int j = y - 1; j <= y + 1; j++ ) {
                for( int i = x - 1; i <= x + 1; i++ ) {
                    const Lumel& lumel = m_lumels[j * m_width + i];

                    if( lumel  ) {
                        color += lumel.m_color;
                        count++;
                    }
                }
            }
            
            if( count ) {
                m_lumels[y * m_width + x].m_color = color * (1.0 / count);
            }
        }
    }
}
    
// ** Lightmap::save
bool Lightmap::save( const String& fileName ) const
{
    char buffer[256];

    FILE	*file;
    int		image_size = 0;

    unsigned char tga_header_a[12]   = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    unsigned char tga_info_header[6] = { 0, 0, 0, 0, 0, 0 };

    file = fopen( fileName.c_str(), "wb" );
    if( !file ) {
        return false;
    }

    fwrite( tga_header_a, 1, sizeof( tga_header_a ), file );

    int channels = 3;

    tga_info_header[0] = m_width  % 256;
    tga_info_header[1] = m_width  / 256;
    tga_info_header[2] = m_height % 256;
    tga_info_header[3] = m_height / 256;
    tga_info_header[4] = channels * 8;
    tga_info_header[5] = 0;

    fwrite( tga_info_header, 1, sizeof( tga_info_header ), file );
    image_size = m_width * m_height * channels;

    for( int j = 0; j < m_height; j++ ) {
        for( int i = 0; i < m_width; i++ ) {
            const Lumel& lumel   = m_lumels[j*m_width + i];
            int          photons = lumel.m_photons ? lumel.m_photons : 1;

            unsigned char r = std::min( lumel.m_color.b / photons * 255.0, 255.0 );
            unsigned char g = std::min( lumel.m_color.g / photons * 255.0, 255.0 );
            unsigned char b = std::min( lumel.m_color.r / photons * 255.0, 255.0 );

            unsigned char pixel[] = { r, g, b };
            fwrite( pixel, sizeof( pixel ), 1, file );
        }
    }
    
    fclose( file );

    return true;
}

// ** Lightmap::toRgb8
unsigned char* Lightmap::toRgb8( void ) const
{
    unsigned char* pixels = new unsigned char[m_width * m_height * 3];
    const int      stride = m_width * 3;

    for( int y = 0; y < m_height; y++ ) {
        for( int x = 0; x < m_width; x++ ) {
            const Lumel&    lumel   = m_lumels[y * m_width + x];
            unsigned char*  pixel   = &pixels[y * stride + x * 3];

            pixel[2] = std::min( lumel.m_color.r * 255.0, 255.0 );
            pixel[1] = std::min( lumel.m_color.g * 255.0, 255.0 );
            pixel[0] = std::min( lumel.m_color.b * 255.0, 255.0 );
        }
    }

    return pixels;
}

// ** Lightmap::toRgb32F
float* Lightmap::toRgb32F( void ) const
{
    float*    pixels = new float[m_width * m_height * 3];
    const int stride = m_width * 3;

    for( int y = 0; y < m_height; y++ ) {
        for( int x = 0; x < m_width; x++ ) {
            const Lumel& lumel = m_lumels[y * m_width + x];
            float*       pixel = &pixels[y * stride + x * 3];

            pixel[0] = lumel.m_color.r;
            pixel[1] = lumel.m_color.g;
            pixel[2] = lumel.m_color.b;
        }
    }
    
    return pixels;
}

// ------------------------------------------------ Photonmap ------------------------------------------------ //

// ** Photonmap::Photonmap
Photonmap::Photonmap( int width, int height ) : Lightmap( width, height )
{

}

// ** Photonmap::addMesh
RelightStatus Photonmap::addMesh( const Mesh* mesh, bool copyVertexColor )
{
    if( mesh->photonmap() ) {
        return RelightInvalidCall;
    }

    // ** Attach this lightmap to an instance
    const_cast<Mesh*>( mesh )->setPhotonmap( this );

    // ** Initialize lumels
    initializeLumels( mesh, copyVertexColor );

    return RelightSuccess;
}

// ** Photonmap::gather
void Photonmap::gather( int radius )
{
    for( int y = 0; y < m_height; y++ ) {
        for( int x = 0; x < m_height; x++ ) {
            lumel( x, y ).m_gathered = gather( x, y, radius );
        }
    }
}

// ** Photonmap::gather
Color Photonmap::gather( int x, int y, int radius ) const
{
    Color color;
    int   photons = 0;

    for( int j = y - radius; j <= y + radius; j++ ) {
        for( int i = x - radius; i <= x + radius; i++ ) {
            if( i < 0 || j < 0 || i >= m_width || j >= m_height ) {
                continue;
            }

            const Lumel& lumel = m_lumels[j * m_width + i];
            float distance = sqrtf( (x - i) * (x - i) + (y - j) * (y - j) );
            if( distance > radius ) {
                continue;
            }

            color   += lumel.m_color;
            photons += lumel.m_photons;
        }
    }

    if( photons == 0 ) {
        return Color( 0.0f, 0.0f, 0.0f );
    }

    return color / photons;
}

} // namespace relight
