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

#include "../BuildCheck.h"
#include "Material.h"

namespace relight {

// --------------------------------------- Material ---------------------------------------- //

// ** Material::Material
Material::Material( const Rgb& color ) : m_color( color )
{

}

// ** Material::colorAt
Rgba Material::colorAt( const Uv& uv ) const
{
    return m_color;
}

// ** Material::color
const Rgb& Material::color( void ) const
{
    return m_color;
}

// ** Material::setColor
void Material::setColor( const Rgb& value )
{
    m_color = value;
}

// ----------------------------------- TexturedMaterial ------------------------------------ //

// ** TexturedMaterial::TexturedMaterial
TexturedMaterial::TexturedMaterial( const Texture* texture, const Rgb& color ) : Material( color ), m_texture( texture )
{
    assert( m_texture != NULL );
}

// **  TexturedMaterial::colorAt
Rgba TexturedMaterial::colorAt( const Uv& uv ) const
{
    return Material::colorAt( uv ) * m_texture->colorAt( uv );
}

// ---------------------------------------- Texture ---------------------------------------- //

// ** Texture::Texture
Texture::Texture( int width, int height, int channels, const unsigned char* pixels ) : m_width( width ), m_height( height ), m_pixels( NULL ), m_channels( channels )
{
    m_pixels = new unsigned char[width * height * channels];
    memcpy( m_pixels, pixels, width * height * channels );
}

Texture::~Texture( void )
{
    delete[]m_pixels;
}

// ** Texture::colorAt
Rgba Texture::colorAt( const Uv& uv ) const
{
    int x = int( uv.x * m_width  ) % m_width;
    int y = int( uv.y * m_height ) % m_height;

    if( x >= 0 && x < m_width && y >= 0 && y < m_height ) {
        const unsigned char* pixel = &m_pixels[y * m_width * m_channels + x * m_channels];
        return Rgba( pixel[0] / 255.0f, pixel[1] / 255.0f, pixel[2] / 255.0f, m_channels == 4 ? pixel[3] / 255.0f : 1.0f );
    }

    return Rgba( 1, 0, 1, 1 );
}

// ** Texture::width
int Texture::width( void ) const
{
    return m_width;
}

// ** Texture::height
int Texture::height( void ) const
{
    return m_height;
}

// ** Texture::channels
int Texture::channels( void ) const
{
    return m_channels;
}

// ** Texture::pixels
const unsigned char* Texture::pixels( void ) const
{
    return m_pixels;
}

// ** Texture::create
Texture* Texture::create( int width, int height, int channels, const unsigned char* pixels )
{
    return new Texture( width, height, channels, pixels );
}

// ** Texture::createFromFile
Texture* Texture::createFromFile( const char* fileName )
{
    unsigned char TGAheader[12]  = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    unsigned char TGAcompare[12];
    unsigned char header[6];

    FILE *file = fopen( fileName, "rb" );

    if( !file ) {
        return NULL;
    }

    if( fread( TGAcompare, 1, sizeof( TGAcompare ), file ) != sizeof( TGAcompare ) ) {
        fclose( file );
        return NULL;
    }

    if( memcmp( TGAcompare, TGAcompare, sizeof( TGAheader ) ) != 0 ) {
        fclose( file );
        return NULL;
    }

    if( fread( header, 1, sizeof( header ), file ) != sizeof( header ) ) {
        fclose( file );
        return NULL;
    }

    int width  = header[1] * 256 + header[0];
    int height = header[3] * 256 + header[2];

    if( width <= 0 || height <= 0 || (header[4] != 24 && header[4] != 32) ) {
        fclose(file);
        return NULL;
    }

    int bytesPerPixel           = header[4] / 8;
    int imageSize               = width * height * bytesPerPixel;
    unsigned char* imageData    = new unsigned char[imageSize];
    fread( imageData, 1, imageSize, file );
    fclose( file );

    for( int i = 0; i < width * height * bytesPerPixel; i += bytesPerPixel ) {
        unsigned char tmp = imageData[i + 0];
        imageData[i + 0] = imageData[i + 2];
        imageData[i + 2] = tmp;
    }

    Texture* texture = Texture::create( width, height, bytesPerPixel, imageData );
    delete[]imageData;

    return texture;
}

} // namespace relight