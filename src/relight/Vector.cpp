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

#include "Vector.h"

namespace relight {

// ** Uv::Uv
Uv::Uv( void ) : u( 0.0f ), v( 0.0f )
{

}

// ** Uv::Uv
Uv::Uv( float u, float v ) : u( u ), v( v )
{
    
}

// ** Color::Color
Color::Color( void ) : r( 0.0f ), g( 0.0f ), b( 0.0f )
{

}

// ** Color::Color
Color::Color( float r, float g, float b ) : r( r ), g( g ), b( b )
{
    
}

// ** Vec3::Vec3
Vec3::Vec3( void ) : x( 0.0f ), y( 0.0f ), z( 0.0f )
{

}

// ** Vec3::Vec3
Vec3::Vec3( float x, float y, float z ) : x( x ), y( y ), z( z )
{
    
}

// ** Matrix4::Matrix4
Matrix4::Matrix4( void )
{
    m[0 ] = 1.0f; m[1 ] = 0.0f; m[2 ] = 0.0f; m[3 ] = 0.0f;
    m[4 ] = 0.0f; m[5 ] = 1.0f; m[6 ] = 0.0f; m[7 ] = 0.0f;
    m[8 ] = 0.0f; m[9 ] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
    m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
}

// ** Matrix4::operator[]
float Matrix4::operator[]( int index ) const
{
    assert( index >= 0 && index < 16 );
    return m[index];
}

// ** Matrix4::operator[]
float& Matrix4::operator[]( int index )
{
    assert( index >= 0 && index < 16 );
    return m[index];
}

// ** Matrix4::translation
Matrix4 Matrix4::translation( float x, float y, float z )
{
    Matrix4 m;
    m[12] = x; m[13] = y; m[14] = z;
    return m;
}

// ** Matrix4::scale
Matrix4 Matrix4::scale( float x, float y, float z )
{
    Matrix4 m;

    m[0 ] = x;    m[1 ] = 0.0f; m[2 ] = 0.0f; m[3 ] = 0.0f;
    m[4 ] = 0.0f; m[5 ] = y;    m[6 ] = 0.0f; m[7 ] = 0.0f;
    m[8 ] = 0.0f; m[9 ] = 0.0f; m[10] = z;    m[11] = 0.0f;
    m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;

    return m;
}

} // namespace relight
