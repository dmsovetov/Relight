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

#ifndef __Relight_Math_H__
#define __Relight_Math_H__

#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>

namespace relight {

    //! Generates a random value in a [0, 1] range.
    inline float rand0to1( void ) {
        static double invRAND_MAX = 1.0 / RAND_MAX;
        return rand() * invRAND_MAX;
    }

    //! Does a linear interpolation between two values.
    inline float lerp( float a, float b, float scalar ) {
        return a * scalar + b * (1.0f - scalar);
    }

    //! Returns a minimum value of two.
    inline float min( float a, float b ) {
        return a < b ? a : b;
    }

    //! Returns a maximum value of two.
    inline float max( float a, float b ) {
        return a > b ? a : b;
    }

    /*!
     Texture UV coordinates.
     */
    class Uv {
    public:

                Uv( void );
                Uv( float u, float v );

        float   operator * ( const Uv& other ) const;
        Uv      operator - ( const Uv& other ) const;
        Uv      operator + ( const Uv& other ) const;
        Uv      operator * ( float scalar ) const;

    public:

        float   u, v;
    };

    // ** Uv::operator *
    inline float Uv::operator * ( const Uv& other ) const {
        return u * other.u + v * other.v;
    }

    // ** Uv::operator -
    inline Uv Uv::operator - ( const Uv& other ) const {
        return Uv( u - other.u, v - other.v );
    }

    // ** Uv::operator +
    inline Uv Uv::operator + ( const Uv& other ) const {
        return Uv( u + other.u, v + other.v );
    }

    // ** Uv::operator *
    inline Uv Uv::operator * ( float scalar ) const {
        return Uv( u * scalar, v * scalar );
    }

    typedef Uv Barycentric;

    /*!
     Euclidean 3-dimensinal vector.
     */
    class Vec3 {
    public:

                    Vec3( void );
                    Vec3( float x, float y, float z );

        float&      operator[]( int index );
        float       operator[]( int index ) const;
        Vec3        operator - ( void ) const;
        Vec3        operator + ( const Vec3& other ) const;
        Vec3        operator - ( const Vec3& other ) const;
        float       operator * ( const Vec3& other ) const;
        Vec3        operator * ( float scalar ) const;
        Vec3        operator / ( float scalar ) const;

        //! Normalizes vector.
        float       normalize( void );

        //! Returns a vector length.
        float       length( void ) const;

        //! Rotates around an axis.
        static Vec3 rotateAroundAxis( const Vec3& axis, float theta, const Vec3& point );

        //! Returns a random point in sphere.
        static Vec3 randomInSphere( const Vec3& center, float radius );

        //! Returns a random direction.
        static Vec3 randomDirection( void );

        //! Returns a random direction on hemisphere.
        static Vec3 randomHemisphereDirection( const Vec3& point, const Vec3& normal );

    public:

        float   x, y, z;
    };

    // ** Vec3::operator[]
    inline float Vec3::operator[]( int index ) const {
        switch( index ) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        }

        assert( false );
        return 0;
    }

    // ** Vec3::operator[]
    inline float& Vec3::operator[]( int index ) {
        switch( index ) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
        }

        assert( false );
        return x;
    }

    // ** Vec3::operator *
    inline Vec3 Vec3::operator * ( float scalar ) const {
        return Vec3( x * scalar, y * scalar, z * scalar );
    }

    // ** Vec3::operator /
    inline Vec3 Vec3::operator / ( float scalar ) const {
        return Vec3( x / scalar, y / scalar, z / scalar );
    }

    // ** Vec3::operator *
    inline float Vec3::operator * ( const Vec3& other ) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // ** Vec3::operator +
    inline Vec3 Vec3::operator + ( const Vec3& other ) const {
        return Vec3( x + other.x, y + other.y, z + other.z );
    }

    // ** Vec3::operator -
    inline Vec3 Vec3::operator - ( const Vec3& other ) const {
        return Vec3( x - other.x, y - other.y, z - other.z );
    }

    inline Vec3 Vec3::operator - ( void ) const {
        return Vec3( -x, -y, -z );
    }

    // ** Vec3::length
    inline float Vec3::length( void ) const {
        return sqrt( x*x + y*y + z*z ) ;
    }

    // ** Vec3::normalize
    inline float Vec3::normalize( void ) {
        float len = length();
        if( len ) {
            x /= len ; y /= len ; z /= len;
        }
        
        return len;
    }

    // ** Vec3::rotateAroundAxis
    inline Vec3 Vec3::rotateAroundAxis( const Vec3& axis, float theta, const Vec3& point )
    {
        Vec3 rotationAxis = axis;
        rotationAxis.normalize();

        double common_factor = sin( theta * 0.5 );

        double a = cos( theta * 0.5 );
        double b = rotationAxis.x * common_factor;
        double c = rotationAxis.y * common_factor;
        double d = rotationAxis.z * common_factor;

        double mat[9] = {
            a*a+b*b-c*c-d*d, 2*(b*c-a*d),     2*(b*d+a*c),
            2*(b*c+a*d),     a*a-b*b+c*c-d*d, 2*(c*d-a*b),
            2*(b*d-a*c),     2*(c*d+a*b),     a*a-b*b-c*c+d*d
        };

    //    for( int i = 0; i < 3; i++ ) vec_out[i] = 0.0;

        Vec3 result;

        for(int i = 0; i < 3; i++){
            for( int j = 0; j < 3; j++ ) result[i] += mat[i*3+j] * point[j];
        }

        return result;
    }

    // ** Vec3::randomInSphere
    inline Vec3 Vec3::randomInSphere( const Vec3& center, float radius )
    {
        return center + Vec3::randomDirection() * radius;
    }

    // ** Vec3::randomDirection
    inline Vec3 Vec3::randomDirection( void )
    {
        Vec3 result( rand0to1() * 2 - 1, rand0to1() * 2 - 1, rand0to1() * 2 - 1 );
        result.normalize();

        return result;
    }

    // ** Vec3::GetRandomDirection
    inline Vec3 Vec3::randomHemisphereDirection( const Vec3& point, const Vec3& normal )
    {
        float theta = acosf( sqrtf( rand0to1() ) );
        float phi   = rand0to1() * 6.28318531f;	// ** TwoPi

        // temporary created rondom direction
        Vec3 temp = Vec3( sinf( theta ) * cosf( phi ), sinf( theta ) * sinf( phi ), cosf( theta ) );

        // rotate "temp" such that z-axis used for sampling is aligned with "normal"
        float angle_between	= acosf( normal.z );

        Vec3 rotation_axis;
        if( fabs( normal.x ) < 0.0001f && fabs( normal.y ) < 0.0001f ) {
            rotation_axis = Vec3( -normal.y, normal.z, 0.0 );
        } else {
            rotation_axis = Vec3( -normal.y, normal.x, 0.0 );
        }

        rotation_axis.normalize();

        return Vec3::rotateAroundAxis( rotation_axis, angle_between, temp );
    }

    /*!
     A plane in a 3-dimensinal space.
     */
    class Plane {
    public:

                        //! Constructs a Plane instance.
                        /*!
                         \param normal Plane normal.
                         \param point Point on plane.
                         */
                        Plane( const Vec3& normal = Vec3( 0, 0, 0 ), float distance = 0.0f )
                            : m_normal( normal ), m_distance( distance ) {}

        //! Projects a point onto this plane
        Vec3            operator * ( const Vec3& point ) const;

        //! Create plane from point and normal.
        static Plane    calculate( const Vec3& normal, const Vec3& point );

    private:

        //! Plane normal.
        Vec3            m_normal;

        //! Plane distance to origin.
        float           m_distance;
    };

    // ** Plane::calculate
    inline Plane Plane::calculate( const Vec3& normal, const Vec3& point) {
        return Plane( normal, -(normal * point) );
    }

    // ** Plane::operator *
    inline Vec3 Plane::operator * ( const Vec3& point ) const {
        float distanceToPoint = m_normal * point + m_distance;
        return point - m_normal * distanceToPoint;
    }

    /*!
     A bounding box class.
     */
    class Bounds {
    public:

                        //! Constructs a Bounds instance
                        /*!
                         \param min A bounds minimum point.
                         \param max A bounds maximum point.
                         */
                        Bounds( Vec3 min = Vec3( FLT_MAX, FLT_MAX, FLT_MAX ), Vec3 max = Vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX ) );

        //! Adds a new point to a bounding box (extends a bounds if needed).
        const Bounds&   operator += ( const Vec3& point );

        //! Adds two bounding volumes.
        const Bounds&   operator += ( const Bounds& other );

        //! Returns a bounds volume.
        float           volume( void ) const;

        //! Returns a minimum bound point.
        const Vec3&     min( void ) const;

        //! Returns a maximum bound point.
        const Vec3&     max( void ) const;

        //! Returns a random point in bounding box.
        Vec3            randomPointInside( void ) const;

    private:

        //! Lower corner coordinate.
        Vec3            m_min;

        //! Upper corner coordinate.
        Vec3            m_max;
    };

    // ** Bounds::Bounds
    inline Bounds::Bounds( Vec3 min, Vec3 max ) : m_min( min ), m_max( max ) {

    }

    // ** Bounds::randomPointInside
    inline Vec3 Bounds::randomPointInside( void ) const
    {
        return Vec3(  lerp( m_min.x, m_max.x, rand0to1() )
                    , lerp( m_min.y, m_max.y, rand0to1() )
                    , lerp( m_min.z, m_max.z, rand0to1() ) );
    }

    // ** Bounds::volume
    inline float Bounds::volume( void ) const {
        return (m_max.x - m_min.x) * (m_max.y - m_min.y) * (m_max.z - m_min.z);
    }

    // ** Bounds::min
    inline const Vec3& Bounds::min( void ) const {
        return m_min;
    }

    // ** Bounds::max
    inline const Vec3& Bounds::max( void ) const {
        return m_max;
    }

    // ** Bounds::operator +=
    inline const Bounds& Bounds::operator += ( const Vec3& point ) {
        for( int i = 0; i < 3; i++ ) {
            m_min[i] = ::relight::min( m_min[i], point[i] );
            m_max[i] = ::relight::max( m_max[i], point[i] );
        }
        return *this;
    }

    // ** Bounds::operator +=
    inline const Bounds& Bounds::operator += ( const Bounds& other ) {
        *this += other.m_min;
        *this += other.m_max;
        return *this;
    }

    /*!
     Affine transform matrix.
     */
    class Matrix4 {
    public:

                        //! Constructs an identity matrix.
                        Matrix4( void );

        Matrix4         operator * ( const Matrix4& other ) const;
        Vec3            operator * ( const Vec3& v ) const;

        //! Constructs a translation transform matrix.
        static Matrix4  translation( float x, float y, float z );

    public:

        float           m[16];
    };

    // ** Matrix4::operator *
    inline Vec3 Matrix4::operator * ( const Vec3& v ) const {
        Vec3 r;

        r.x = v.x * m[0] + v.y * m[4] + v.z * m[8] + m[12];
        r.y = v.x * m[1] + v.y * m[5] + v.z * m[9] + m[13];
        r.z = v.x * m[2] + v.y * m[6] + v.z * m[10]+ m[14];

        return r;
    }

    /*!
     Color
     */
    class Color {
    public:

                        Color( void );
                        Color( float r, float g, float b );

        const Color&    operator += ( const Color& other );
        const Color&    operator *= ( const Color& other );
        const Color&    operator *= ( float scalar );
        Color           operator * ( float scalar ) const;
        Color           operator + ( const Color& other ) const;
        Color           operator * ( const Color& other ) const;
        Color           operator / ( float scalar ) const;

        //! Calculates color luminance,
        float           luminance( void ) const;

    public:

        float           r, g, b;
    };

    // ** Color::luminance
    inline float Color::luminance( void ) const {
        return 0.2126f * r + 0.7152f * g + 0.0722f * b;
    }

    // ** Color::operator +=
    inline const Color& Color::operator += ( const Color& other ) {
        r += other.r; g += other.g; b += other.b;
        return *this;
    }

    // ** Color::operator *=
    inline const Color& Color::operator *= ( const Color& other ) {
        r *= other.r; g *= other.g; b *= other.b;
        return *this;
    }

    // ** Color::operator *=
    inline const Color& Color::operator *= ( float scalar ) {
        r *= scalar; g *= scalar; b *= scalar;
        return *this;
    }

    // ** Color::operator *
    inline Color Color::operator * ( float scalar ) const {
        return Color( r * scalar, g * scalar, b * scalar );
    }

    // ** Color::operator +
    inline Color Color::operator + ( const Color& other ) const {
        return Color( r + other.r, g + other.g, b + other.b );
    }

    // ** Color::operator *
    inline Color Color::operator * ( const Color& other ) const {
        return Color( r * other.r, g * other.g, b * other.b );
    }

    // ** Color::operator /
    inline Color Color::operator / ( float scalar ) const {
        return Color( r / scalar, g / scalar, b / scalar );
    }

} // namespace relight

#endif  /*  !defined( __Relight_Math_H__ )  */
