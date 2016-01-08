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

#ifndef __Relight_Types_H__
#define __Relight_Types_H__

#include <vector>
#include <string>
#include <map>
#include <algorithm>

#pragma warning( push )
#pragma warning( disable : 4244 4018 )

#ifdef RELIGHT_BUILD_LIBRARY
	#define assert( expr )		_ASSERTE( expr )
	#define DC_BREAK_IF( expr )	assert( !(expr) )
	#define BIT( bit )          ( 1 << bit )
#endif

#ifdef min
	#undef min	
#endif

#ifdef max
	#undef max
#endif

#ifdef emit
	#undef emit
#endif

namespace relight {

	const float Pi = 3.1415926535897932f;

	class Rgba;

	typedef unsigned char	u8;
	typedef float			f32;
	typedef unsigned short	u16;
	typedef int				s32;
	typedef unsigned int	u32;
	typedef std::string		String;

	template<typename T> class Array : public std::vector<T> {};
	template<typename K, typename V, typename P = std::less<K>> class Map : public std::map<K, V, P> {};
	template<typename T> class AutoPtr : public std::auto_ptr<T> {};

    //! Does a linear interpolation between two values.
	template<typename TValue>
    TValue lerp( TValue a, TValue b, f32 scalar )
	{
        return static_cast<TValue>( a * (1.0f - scalar) + b * scalar );
    }

    //! Converts radians to degrees
    inline float degrees( float radians ) {
        return (radians) * 180.0f / Pi;
    }

    //! Returns a minimum value of two.
	template<typename TValue>
    TValue min2( const TValue& a, const TValue& b ) {
        return a < b ? a : b;
    }

    //! Returns a minimum value of three.
	template<typename TValue>
    TValue min3( const TValue& a, const TValue& b, const TValue& c ) {
        return min2( a, min2( b, c ) );
    }

    //! Returns a maximum value of two.
	template<typename TValue>
    TValue max2( const TValue& a, const TValue& b ) {
        return a > b ? a : b;
    }

    //! Returns a maximum value of three.
	template<typename TValue>
    TValue max3( const TValue& a, const TValue& b, const TValue& c ) {
        return max2( a, max2( b, c ) );
    }

    //! Generates a random value in a [0, 1] range.
    inline float rand0to1( void ) {
        static float invRAND_MAX = 1.0f / RAND_MAX;
        return rand() * invRAND_MAX;
    }

	//! Returns true if an argument is not a number.
	inline bool isNaN( float value )
	{
	#ifdef WIN32
		return _isnan( value ) ? true : false;
	#else
		return isnan( value );
	#endif
	}

    /*!
     Double LDR pixel data.
    */
    struct DoubleLdr {
                        //! Constructs the DoubleLdr instance.
                        DoubleLdr( unsigned char r, unsigned char g, unsigned char b )
                            : r( r ), g( g ), b( b ) {}

        unsigned char   r;
        unsigned char   g;
        unsigned char   b;
    };

    /*!
     RGBm LDR pixel data.
    */
    struct RgbmLdr {
                        //! Constructs the RgbmLdr instance.
                        RgbmLdr( unsigned char r, unsigned char g, unsigned char b, unsigned char m )
                            : r( r ), g( g ), b( b ), m( m ) {}

        unsigned char   r;
        unsigned char   g;
        unsigned char   b;
        unsigned char   m;
    };

    /*!
     Rgb color value
     */
    class Rgb {
    public:

                        Rgb( void );
                        Rgb( float r, float g, float b );
                        Rgb( const float* v );
        explicit        Rgb( const Rgba& other );

        bool            operator == ( const Rgb& other ) const;
        const Rgb&      operator += ( const Rgb& other );
        const Rgb&      operator *= ( const Rgb& other );
        const Rgb&      operator *= ( float scalar );
        Rgb             operator * ( float scalar ) const;
        Rgb             operator + ( const Rgb& other ) const;
        Rgb             operator * ( const Rgb& other ) const;
        Rgb             operator / ( const Rgb& other ) const;
        Rgb             operator / ( float scalar ) const;
        Rgb             operator + ( float scalar ) const;
        Rgb             operator - ( float scalar ) const;

        //! Calculates color luminance,
        float           luminance( void ) const;

        //! Converts color to double LDR encoded value.
        DoubleLdr       doubleLdr( void ) const;

        //! Converts color to RGBM LDR encoded value.
        RgbmLdr         rgbm( void ) const;

		//! Constructs Rgb color instance from bytes.
		static Rgb		fromBytes( u8 r, u8 g, u8 b );

    public:

        float           r, g, b;
    };

    // ** Rgb::Rgb
    inline Rgb::Rgb( void ) : r( 0.0f ), g( 0.0f ), b( 0.0f )
    {

    }

    // ** Rgb::Rgb
    inline Rgb::Rgb( float r, float g, float b ) : r( r ), g( g ), b( b )
    {

    }

    // ** Rgb::Rgb
    inline Rgb::Rgb( const float* v ) : r( v[0] ), g( v[1] ), b( v[2] )
    {

    }

    // ** Rgb::luminance
    inline float Rgb::luminance( void ) const {
        return 0.2126f * r + 0.7152f * g + 0.0722f * b;
    }

    // ** Rgb::doubleLdr
    inline DoubleLdr Rgb::doubleLdr( void ) const
    {
        return DoubleLdr( static_cast<unsigned char>( min2( 255.0f, r * 0.5f * 255.0f ) )
                        , static_cast<unsigned char>( min2( 255.0f, g * 0.5f * 255.0f ) )
                        , static_cast<unsigned char>( min2( 255.0f, b * 0.5f * 255.0f ) )
                        );
    }

    // ** Rgb::rgbm
    inline RgbmLdr Rgb::rgbm( void ) const
    {
        float m = 255.0f / max3( r, g, b );

        return RgbmLdr( static_cast<unsigned char>( min2( 255.0f, r / m * 255.0f ) )
                      , static_cast<unsigned char>( min2( 255.0f, g / m * 255.0f ) )
                      , static_cast<unsigned char>( min2( 255.0f, b / m * 255.0f ) )
                      , static_cast<unsigned char>( min2( 255.0f, m * 255.0f ) )
                      );
    }

    // ** Rgb::operator ==
    inline bool Rgb::operator == ( const Rgb& other ) const {
        return r == other.r && g == other.g && b == other.b;
    }

    // ** Rgb::operator +=
    inline const Rgb& Rgb::operator += ( const Rgb& other ) {
        r += other.r; g += other.g; b += other.b;
        return *this;
    }

    // ** Rgb::operator *=
    inline const Rgb& Rgb::operator *= ( const Rgb& other ) {
        r *= other.r; g *= other.g; b *= other.b;
        return *this;
    }

    // ** Rgb::operator *=
    inline const Rgb& Rgb::operator *= ( float scalar ) {
        r *= scalar; g *= scalar; b *= scalar;
        return *this;
    }

    // ** Rgb::operator *
    inline Rgb Rgb::operator * ( float scalar ) const {
        return Rgb( r * scalar, g * scalar, b * scalar );
    }

    // ** Rgb::operator +
    inline Rgb Rgb::operator + ( const Rgb& other ) const {
        return Rgb( r + other.r, g + other.g, b + other.b );
    }

    // ** Rgb::operator *
    inline Rgb Rgb::operator * ( const Rgb& other ) const {
        return Rgb( r * other.r, g * other.g, b * other.b );
    }

    // ** Rgb::operator /
    inline Rgb Rgb::operator / ( const Rgb& other ) const {
        return Rgb( r / other.r, g / other.g, b / other.b );
    }

    // ** Rgb::operator /
    inline Rgb Rgb::operator / ( float scalar ) const {
        return Rgb( r / scalar, g / scalar, b / scalar );
    }

    // ** Rgb::operator +
    inline Rgb Rgb::operator + ( float scalar ) const {
        return Rgb( r + scalar, g + scalar, b + scalar );
    }

    // ** Rgb::operator -
    inline Rgb Rgb::operator - ( float scalar ) const {
        return Rgb( r - scalar, g - scalar, b - scalar );
    }

	// ** Rgb::fromBytes
	inline Rgb Rgb::fromBytes( u8 r, u8 g, u8 b ) {
		return Rgb( r / 255.0f, g / 255.0f, b / 255.0f );
	}

    /*!
     Rgba color value.
     */
    class Rgba {
    public:

                        Rgba( void );
                        Rgba( float r, float g, float b, float a = 1.0f );
						Rgba( const float *v );
                        Rgba( const Rgb& other );

        bool            operator == ( const Rgba& other ) const;
        const Rgba&     operator += ( const Rgba& other );
        const Rgba&     operator *= ( const Rgba& other );
        const Rgba&     operator *= ( float scalar );
        Rgba            operator * ( float scalar ) const;
        Rgba            operator + ( const Rgba& other ) const;
        Rgba            operator * ( const Rgba& other ) const;
        Rgba            operator / ( float scalar ) const;

		//! Returns the color with modulated alpha by a given factor.
		Rgba			transparent( f32 factor ) const;

		//! Constructs Rgba color instance from bytes.
		static Rgba		fromBytes( u8 r, u8 g, u8 b, u8 a = 255 );

    public:

        float           r, g, b, a;
    };

    // ** Rgba::Rgba
    inline Rgba::Rgba( void ) : r( 0.0f ), g( 0.0f ), b( 0.0f ), a( 1.0f )
    {

    }

    // ** Rgba::Rgba
    inline Rgba::Rgba( float r, float g, float b, float a ) : r( r ), g( g ), b( b ), a( a )
    {

    }

    // ** Rgba::Rgba
    inline Rgba::Rgba( const float* v ) : r( v[0] ), g( v[1] ), b( v[2] ), a( v[3] )
    {

    }

    // ** Rgba::Rgba
    inline Rgba::Rgba( const Rgb& other ) : r( other.r ), g( other.g ), b( other.b ), a( 1.0f )
    {

    }

    // ** Rgba::operator ==
    inline bool Rgba::operator == ( const Rgba& other ) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    // ** Rgba::operator +=
    inline const Rgba& Rgba::operator += ( const Rgba& other ) {
        r += other.r; g += other.g; b += other.b; a += other.a;
        return *this;
    }

    // ** Rgba::operator *=
    inline const Rgba& Rgba::operator *= ( const Rgba& other ) {
        r *= other.r; g *= other.g; b *= other.b; a *= other.a;
        return *this;
    }

    // ** Rgba::operator *=
    inline const Rgba& Rgba::operator *= ( float scalar ) {
        r *= scalar; g *= scalar; b *= scalar; a *= scalar;
        return *this;
    }

    // ** Rgba::operator *
    inline Rgba Rgba::operator * ( float scalar ) const {
        return Rgba( r * scalar, g * scalar, b * scalar, a * scalar );
    }

    // ** Rgba::operator +
    inline Rgba Rgba::operator + ( const Rgba& other ) const {
        return Rgba( r + other.r, g + other.g, b + other.b, a + other.a );
    }

    // ** Rgba::operator *
    inline Rgba Rgba::operator * ( const Rgba& other ) const {
        return Rgba( r * other.r, g * other.g, b * other.b, a * other.a );
    }

    // ** Rgba::operator /
    inline Rgba Rgba::operator / ( float scalar ) const {
        return Rgba( r / scalar, g / scalar, b / scalar, a / scalar );
    }

	// ** Rgba::transparent
	inline Rgba Rgba::transparent( f32 factor ) const {
		return Rgba( r, g, b, a * factor );
	}

	// ** Rgba::fromBytes
	inline Rgba Rgba::fromBytes( u8 r, u8 g, u8 b, u8 a ) {
		return Rgba( r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f );
	}

    /*!
     2-component euclidean space vector.
     */
    class Vec2 {
    public:

                Vec2( void );
				Vec2( float value );
                Vec2( float x, float y );
				Vec2( int x, int y );
                Vec2( const float* v );
				Vec2( const double* v );

        float&  operator[]( int index );
        float   operator[]( int index ) const;
        bool    operator == ( const Vec2& other ) const;

		const Vec2& operator += ( const Vec2& other );
		const Vec2&	operator *= ( f32 scalar );

        float		operator * ( const Vec2& other ) const;
        Vec2		operator - ( const Vec2& other ) const;
        Vec2		operator + ( const Vec2& other ) const;
		Vec2		operator / ( const Vec2& other ) const;
        Vec2		operator * ( float scalar ) const;
		Vec2		operator - ( void ) const;

		//! Normalizes the vector.
		float	normalize( void );

		//! Returns the vector length.
		float	length( void ) const;

		//! Returns the squared vector length.
		float	lengthSqr( void ) const;

        //! Returns a perpendicular vector to a this one.
        Vec2    perp( void ) const;

		//! Returns the normalized vector.
		static Vec2	normalized( const Vec2& v );

		//! Returns the random direction vector.
		static Vec2 randDirection( void );

		//! Returns the vector by a clock-wise angle
		static Vec2 fromAngle( float angle );

    public:

        float   x, y;
    };

    // ** Vec2::Vec2
    inline Vec2::Vec2( void ) : x( 0.0f ), y( 0.0f )
    {

    }

    // ** Vec2::Vec2
    inline Vec2::Vec2( float value ) : x( value ), y( value )
    {

    }

    // ** Vec2::Vec2
    inline Vec2::Vec2( float x, float y ) : x( x ), y( y )
    {

    }

    // ** Vec2::Vec2
    inline Vec2::Vec2( int x, int y ) : x( ( float )x ), y( ( float )y )
    {

    }

    // ** Vec2::Vec2
    inline Vec2::Vec2( const float* v ) : x( v[0] ), y( v[1] )
    {

    }

    // ** Vec2::Vec2
    inline Vec2::Vec2( const double* v ) : x( ( float )v[0] ), y( ( float )v[1] )
    {

    }

    // ** Vec2::operator[]
    inline float Vec2::operator[]( int index ) const {
        switch( index ) {
        case 0: return x;
        case 1: return y;
        }

        _ASSERT( false );
        return 0;
    }

    // ** Vec2::operator[]
    inline float& Vec2::operator[]( int index ) {
        switch( index ) {
        case 0: return x;
        case 1: return y;
        }

        _ASSERT( false );
        return x;
    }

    // ** Vec2::operator ==
    inline bool Vec2::operator == ( const Vec2& other ) const {
        return x == other.x && y == other.y;
    }

    // ** Vec2::operator +=
    inline const Vec2& Vec2::operator += ( const Vec2& other )
	{
        x += other.x;
		y += other.y;
		return *this;
    }

    // ** Vec2::operator *=
    inline const Vec2& Vec2::operator *= ( f32 scalar )
	{
        x *= scalar;
		y *= scalar;
		return *this;
    }

    // ** Vec2::operator *
    inline float Vec2::operator * ( const Vec2& other ) const {
        return x * other.x + y * other.y;
    }

    // ** Vec2::operator *
    inline Vec2 Vec2::operator / ( const Vec2& other ) const {
        return Vec2( x / other.x, y / other.y );
    }

    // ** Vec2::operator -
    inline Vec2 Vec2::operator - ( const Vec2& other ) const {
        return Vec2( x - other.x, y - other.y );
    }

    // ** Vec2::operator +
    inline Vec2 Vec2::operator + ( const Vec2& other ) const {
        return Vec2( x + other.x, y + other.y );
    }

    // ** Vec2::operator *
    inline Vec2 Vec2::operator * ( float scalar ) const {
        return Vec2( x * scalar, y * scalar );
    }

    // ** Vec2::operator -
    inline Vec2 Vec2::operator - ( void ) const {
        return Vec2( -x, -y );
    }

	// ** Vec2::normalize
	inline float Vec2::normalize( void )
	{
		float len = length();

		if( len == 0.0f ) {
			return len;
		}

		x /= len;
		y /= len;

		return len;
	}

	// ** Vec2::length
	inline float Vec2::length( void ) const
	{
		return sqrtf( lengthSqr() );
	}

	// ** Vec2::lengthSqr
	inline float Vec2::lengthSqr( void ) const
	{
		return x * x + y * y;
	}

    // ** Vec2::perp
    inline Vec2 Vec2::perp( void ) const
    {
        return Vec2( y, -x );
    }

	// ** Vec2::normalized
	inline Vec2 Vec2::normalized( const Vec2& v )
	{
		Vec2 result = v;
		result.normalize();
		return result;
	}

    // ** Vec2::randDirection
    inline Vec2 Vec2::randDirection( void )
    {
        float angle = float( rand() % 360 );
        return fromAngle( angle );
    }

    // ** Vec2::fromAngle
    //inline Vec2 Vec2::fromAngle( float angle )
    //{
    //    float r = radians( angle );
    //    return Vec2( sinf( r ), cosf( r ) );
    //}

    // ** operator *
    inline Vec2 operator * ( const float scalar, const Vec2& v )
    {
		return Vec2( v.x * scalar, v.y * scalar );
	}

    /*!
     Euclidean 3-dimensinal vector.
     */
    class Vec3 {
    public:

                    Vec3( void );
                    Vec3( float x, float y, float z );
					Vec3( int x, int y, int z );
                    Vec3( const float* v );
					Vec3( const double* v );

					operator Vec2( void ) const;

        bool        operator == ( const Vec3& other ) const;
        float&      operator[]( int index );
        float       operator[]( int index ) const;
        Vec3        operator - ( void ) const;
        Vec3        operator + ( float scalar ) const;
        Vec3        operator + ( const Vec3& other ) const;
        Vec3        operator - ( const Vec3& other ) const;
        float       operator * ( const Vec3& other ) const;
        const Vec3& operator += ( const Vec3& other );
        Vec3        operator * ( float scalar ) const;
        Vec3        operator / ( float scalar ) const;
        const Vec3& operator /= ( float scalar );
		const Vec3& operator *= ( float scalar );
        Vec3        operator % ( const Vec3& other ) const;

        //! Normalizes vector.
        float       normalize( void );

        //! Returns a vector length.
        float       length( void ) const;

		//! Returns a vector squared length.
		float		lengthSqr( void ) const;

        //! Returns an ordinal axis.
        Vec3        ordinal( void ) const;

		//! Returns the zero vector.
		static Vec3	zero( void );

		//! Returns the X axis vector.
		static Vec3 axisX( void );

		//! Returns the Y axis vector.
		static Vec3 axisY( void );

		//! Returns the Z axis vector.
		static Vec3 axisZ( void );

        //! Rotates around an axis.
        static Vec3 rotateAroundAxis( const Vec3& axis, float theta, const Vec3& point );

        //! Returns a random point in sphere.
        static Vec3 randomInSphere( const Vec3& center, float radius );

        //! Returns a random direction.
        static Vec3 randomDirection( void );

        //! Returns a random direction on hemisphere.
        static Vec3 randomHemisphereDirection( const Vec3& normal );

		//! Returns a random cosine weighted direcion on hemisphere.
		static Vec3 randomHemisphereDirectionCosine( const Vec3& normal );

        //! Returns a normalized vector.
        static Vec3 normalize( const Vec3& v );

    public:

        float   x, y, z;
    };

    // ** Vec3::Vec3
    inline Vec3::Vec3( void ) : x( 0.0f ), y( 0.0f ), z( 0.0f )
    {

    }

    // ** Vec3::Vec3
    inline Vec3::Vec3( float x, float y, float z ) : x( x ), y( y ), z( z )
    {

    }

    // ** Vec3::Vec3
    inline Vec3::Vec3( int x, int y, int z ) : x( ( float )x ), y( ( float )y ), z( ( float )z )
    {

    }

    // ** Vec3::Vec3
    inline Vec3::Vec3( const float* v ) : x( v[0] ), y( v[1] ), z( v[2] )
    {

    }

    // ** Vec3::Vec3
    inline Vec3::Vec3( const double* v ) : x( ( float )v[0] ), y( ( float )v[1] ), z( ( float )v[2] )
    {

    }

	// ** Vec3::Vec2
	inline Vec3::operator Vec2( void ) const
	{
		return Vec2( x, y );
	}

    // ** Vec3::operator[]
    inline float Vec3::operator[]( int index ) const {
        switch( index ) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        }

        _ASSERT( false );
        return 0;
    }

    // ** Vec3::operator[]
    inline float& Vec3::operator[]( int index ) {
        switch( index ) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        }

        _ASSERT( false );
        return x;
    }

    // ** Vec3::operator ==
    inline bool Vec3::operator == ( const Vec3& other ) const {
        return x == other.x && y == other.y && z == other.z;
    }

    // ** Vec3::operator *
    inline Vec3 Vec3::operator * ( float scalar ) const {
        return Vec3( x * scalar, y * scalar, z * scalar );
    }

    // ** Vec3::operator /
    inline Vec3 Vec3::operator / ( float scalar ) const {
        return Vec3( x / scalar, y / scalar, z / scalar );
    }

    // ** Vec3::operator /
    inline const Vec3& Vec3::operator /= ( float scalar ) {
        x /= scalar; y /= scalar; z /= scalar;
        return *this;
    }

    // ** Vec3::operator /
    inline const Vec3& Vec3::operator *= ( float scalar ) {
        x *= scalar; y *= scalar; z *= scalar;
        return *this;
    }

    // ** Vec3::operator *
    inline float Vec3::operator * ( const Vec3& other ) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // ** Vec3::operator +
    inline Vec3 Vec3::operator + ( float scalar ) const {
        return Vec3( x + scalar, y + scalar, z + scalar );
    }

    // ** Vec3::operator +
    inline Vec3 Vec3::operator + ( const Vec3& other ) const {
        return Vec3( x + other.x, y + other.y, z + other.z );
    }

    // ** Vec3::operator +
    inline const Vec3& Vec3::operator += ( const Vec3& other ) {
        x += other.x; y += other.y; z += other.z;
        return *this;
    }

    // ** Vec3::operator -
    inline Vec3 Vec3::operator - ( const Vec3& other ) const {
        return Vec3( x - other.x, y - other.y, z - other.z );
    }

    // ** Vec3::operator %
    inline Vec3 Vec3::operator % ( const Vec3& other ) const {
        return Vec3( y*other.z - z*other.y, z*other.x - x*other.z, x*other.y - y*other.x );
    }

    inline Vec3 Vec3::operator - ( void ) const {
        return Vec3( -x, -y, -z );
    }

    // ** Vec3::ordinal
    inline Vec3 Vec3::ordinal( void ) const
    {
        float nx = fabs( x );
		float ny = fabs( y );
		float nz = fabs( z );

		if( nx > ny && nx > nz ) return Vec3( 1, 0, 0 );
		if( ny > nx && ny > nz ) return Vec3( 0, 1, 0 );

        return Vec3( 0, 0, 1 );
    }

    // ** Vec3::length
    inline float Vec3::length( void ) const {
        return sqrt( lengthSqr() );
    }

    // ** Vec3::lengthSqr
    inline float Vec3::lengthSqr( void ) const {
        return x*x + y*y + z*z;
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

        Vec3 result;

        for(int i = 0; i < 3; i++){
            for( int j = 0; j < 3; j++ ) result[i] += static_cast<float>( mat[i*3+j] * point[j] );
        }

        _ASSERT( !isNaN( result.x ) );
        _ASSERT( !isNaN( result.y ) );
        _ASSERT( !isNaN( result.z ) );

        return result;
    }

	// ** Vec3::zero
	inline Vec3	Vec3::zero( void )
	{
		return Vec3( 0.0f, 0.0f, 0.0f );
	}

	// ** Vec3::axisX
	inline Vec3 Vec3::axisX( void )
	{
		return Vec3( 1.0f, 0.0f, 0.0f );
	}

	// ** Vec3::axisY
	inline Vec3 Vec3::axisY( void )
	{
		return Vec3( 0.0f, 1.0f, 0.0f );
	}

	// ** Vec3::axisZ
	inline Vec3 Vec3::axisZ( void )
	{
		return Vec3( 0.0f, 0.0f, 1.0f );
	}

    // ** Vec3::randomInSphere
    inline Vec3 Vec3::randomInSphere( const Vec3& center, float radius )
    {
        return center + Vec3::randomDirection() * radius;
    }

    // ** Vec3::randomDirection
    inline Vec3 Vec3::randomDirection( void )
    {
		Vec3 dir;
		f32  len;

		do {
		   dir.x = (rand0to1() * 2.0f - 1.0f);
		   dir.y = (rand0to1() * 2.0f - 1.0f);
		   dir.z = (rand0to1() * 2.0f - 1.0f);
		   len   = dir.length();
		} while( len > 1.0f );

		return dir / len;
    }

    // ** Vec3::randomHemisphereDirection
    inline Vec3 Vec3::randomHemisphereDirection( const Vec3& normal )
    {
        Vec3 dir = randomDirection();

        if( dir * normal < 0 ) {
            dir = -dir;
        }

        return dir;
    }

	// ** Vec3::randomHemisphereDirectionCosine
	inline Vec3 Vec3::randomHemisphereDirectionCosine( const Vec3& normal )
	{
		f32 Xi1 = (f32)rand()/(f32)RAND_MAX;
		f32 Xi2 = (f32)rand()/(f32)RAND_MAX;

		f32  theta = acosf(sqrtf(1.0f-Xi1));
		f32  phi = 2.0f * 3.1415926535897932384626433832795f * Xi2;

		f32 xs = sinf(theta) * cosf(phi);
		f32 ys = cosf(theta);
		f32 zs = sinf(theta) * sinf(phi);

		Vec3 d( xs, ys, zs );
		return d * normal < 0 ? -d : d;

	/*
		Vec3 y = normal;
		Vec3 h = y;
		if (fabs(h.x)<=fabs(h.y) && fabs(h.x)<=fabs(h.z))
			h.x= 1.0;
		else if (fabs(h.y)<=fabs(h.x) && fabs(h.y)<=fabs(h.z))
			h.y= 1.0;
		else
			h.z= 1.0;


		Vec3 x = (h % y); x.normalize();
		Vec3 z = (x % y); y.normalize();

		Vec3 direction = x * xs + y * ys + z * zs;
		direction.normalize();
		return direction;*/
	}

    // ** Vec3::normalize
    inline Vec3 Vec3::normalize( const Vec3& v ) {
        Vec3 result = v;
        result.normalize();
        return result;
    }

	//! A 2d bounding rectangle class.
	class Rect {
	public:

						//! Constructs a Rect from corner points.
						Rect( const Vec2& min = Vec2(), const Vec2& max = Vec2() );

						//! Constructs a Rect instance from it's coordinates.
						Rect( f32 x1, f32 y1, f32 x2, f32 y2 );

		//! Returns the upper left bounding rect corner.
		const Vec2&		min( void ) const;

		//! Returns the lower right bounding rect corner.
		const Vec2&		max( void ) const;

		//! Returns the left side of a rectangle.
		f32				left( void ) const;

		//! Returns the right side of a rectangle.
		f32				right( void ) const;

		//! Returns the top side of a rectangle.
		f32				top( void ) const;

		//! Returns the bottom side of a rectangle.
		f32				bottom( void ) const;

		//! Returns rectangle width.
		f32				width( void ) const;

		//! Returns rectangle height.
		f32				height( void ) const;

	private:

		Vec2			m_min;	//!< Upper left bounding rect corner.
		Vec2			m_max;	//!< Lower right bounding rect corner.
	};

	// ** Rect::Rect
	inline Rect::Rect( const Vec2& min, const Vec2& max )
		: m_min( min ), m_max( max )
	{
	}

	// ** Rect::Rect
	inline Rect::Rect( f32 x1, f32 y1, f32 x2, f32 y2 )
		: m_min( x1, y1 ), m_max( x2, y2 )
	{
	}

	// ** Rect::min
	inline const Vec2& Rect::min( void ) const
	{
		return m_min;
	}

	// ** Rect::max
	inline const Vec2& Rect::max( void ) const
	{
		return m_max;
	}

	// ** Rect::left
	inline f32 Rect::left( void ) const
	{
		return m_min.x;
	}

	// ** Rect::right
	inline f32 Rect::right( void ) const
	{
		return m_max.x;
	}

	// ** Rect::top
	inline f32 Rect::top( void ) const
	{
		return m_max.y;
	}

	// ** Rect::bottom
	inline f32 Rect::bottom( void ) const
	{
		return m_min.y;
	}

	// ** Rect::min
	inline f32 Rect::width( void ) const
	{
		return m_max.x - m_min.x;
	}

	// ** Rect::max
	inline f32 Rect::height( void ) const
	{
		return m_max.y - m_min.y;
	}

    /*!
     Affine transform matrix.
     */
    class Matrix4 {
    public:

                        //! Constructs an identity matrix.
                        Matrix4( void );

                        //! Constructs a matrix.
                        Matrix4( float t11, float t12, float t13, float t14,
                                 float t21, float t22, float t23, float t24,
                                 float t31, float t32, float t33, float t34,
                                 float t41, float t42, float t43, float t44 );

        float           operator[] ( int index ) const;
        float&          operator[] ( int index );

		//! Multiplies two 4x4 matrices.
        Matrix4         operator * ( const Matrix4& other ) const;

		//! Multiplies a matrix by a 3-component vector.
        Vec3            operator * ( const Vec3& v ) const;

		//! Returns the matrix value.
		float			value( int row, int col ) const;

        //! Calculate the inverse matrix.
        Matrix4         inversed( void ) const;

		//! Calculate the transpose matrix.
		Matrix4			transposed( void ) const;

        //! Rotates vector.
        Vec3            rotate( const Vec3& v ) const;

        //! Constructs a translation transform matrix.
        static Matrix4  translation( float x, float y, float z );

        //! Constructs a translation transform matrix.
        static Matrix4  translation( const Vec3& position );

        //! Constructs a scale transform matrix.
        static Matrix4  scale( float x, float y, float z );

        //! Constructs a scale transform matrix.
        static Matrix4  scale( const Vec3& scale );

        //! Creates a perspective projection matrix.
        static Matrix4  perspective( float fov, float aspect, float zNear, float zFar );

        //! Creates a left-handed perspective projection matrix.
        static Matrix4  perspectiveLeft( float fov, float aspect, float zNear, float zFar );

        //! Creates a ortho projection matrix.
        static Matrix4  ortho( float left, float right, float bottom, float top, float zNear, float zFar );

        //! Creates a look at view matrix.
        static Matrix4  lookAt( const Vec3& position, const Vec3& target, const Vec3& up );

        //! Creates a left-handed look at view matrix.
        static Matrix4  lookAtLeft( const Vec3& position, const Vec3& target, const Vec3& up );

		//! Creates a view matrix from basis vectors.
		static Matrix4	view( const Vec3& position, const Vec3& direction, const Vec3& up, const Vec3& right );

		//! Creates a left-handed view matrix from basis vectors.
		static Matrix4	viewLeft( const Vec3& position, const Vec3& direction, const Vec3& up, const Vec3& right );

    public:

        float           m[16];
    };

    // ** Matrix4::Matrix4
    inline Matrix4::Matrix4( void )
    {
        m[0 ] = 1.0f; m[1 ] = 0.0f; m[2 ] = 0.0f; m[3 ] = 0.0f;
        m[4 ] = 0.0f; m[5 ] = 1.0f; m[6 ] = 0.0f; m[7 ] = 0.0f;
        m[8 ] = 0.0f; m[9 ] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
        m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
    }

    // ** Matrix4::Matrix4
    inline Matrix4::Matrix4( float t11, float t12, float t13, float t14,
                             float t21, float t22, float t23, float t24,
                             float t31, float t32, float t33, float t34,
                             float t41, float t42, float t43, float t44 ) {
        m[ 0] = t11; m[ 1] = t12; m[ 2] = t13; m[ 3] = t14;
        m[ 4] = t21; m[ 5] = t22; m[ 6] = t23; m[ 7] = t24;
        m[ 8] = t31; m[ 9] = t32; m[10] = t33; m[11] = t34;
        m[12] = t41; m[13] = t42; m[14] = t43; m[15] = t44;
    }

    // ** Matrix4::operator *
    inline Vec3 Matrix4::operator * ( const Vec3& v ) const {
        Vec3 r;

        r.x = v.x * m[0] + v.y * m[4] + v.z * m[8] + m[12];
        r.y = v.x * m[1] + v.y * m[5] + v.z * m[9] + m[13];
        r.z = v.x * m[2] + v.y * m[6] + v.z * m[10]+ m[14];

        return r;
    }

    // ** Matrix4::operator *
    inline Matrix4 Matrix4::operator * ( const Matrix4& other ) const
    {
        return Matrix4(
                       m[0]*other[0]  + m[4]*other[1]  + m[8]*other[2]   + m[12]*other[3],
                       m[1]*other[0]  + m[5]*other[1]  + m[9]*other[2]   + m[13]*other[3],
                       m[2]*other[0]  + m[6]*other[1]  + m[10]*other[2]  + m[14]*other[3],
                       m[3]*other[0]  + m[7]*other[1]  + m[11]*other[2]  + m[15]*other[3],
                       m[0]*other[4]  + m[4]*other[5]  + m[8]*other[6]   + m[12]*other[7],
                       m[1]*other[4]  + m[5]*other[5]  + m[9]*other[6]   + m[13]*other[7],
                       m[2]*other[4]  + m[6]*other[5]  + m[10]*other[6]  + m[14]*other[7],
                       m[3]*other[4]  + m[7]*other[5]  + m[11]*other[6]  + m[15]*other[7],
                       m[0]*other[8]  + m[4]*other[9]  + m[8]*other[10]  + m[12]*other[11],
                       m[1]*other[8]  + m[5]*other[9]  + m[9]*other[10]  + m[13]*other[11],
                       m[2]*other[8]  + m[6]*other[9]  + m[10]*other[10] + m[14]*other[11],
                       m[3]*other[8]  + m[7]*other[9]  + m[11]*other[10] + m[15]*other[11],
                       m[0]*other[12] + m[4]*other[13] + m[8]*other[14]  + m[12]*other[15],
                       m[1]*other[12] + m[5]*other[13] + m[9]*other[14]  + m[13]*other[15],
                       m[2]*other[12] + m[6]*other[13] + m[10]*other[14] + m[14]*other[15],
                       m[3]*other[12] + m[7]*other[13] + m[11]*other[14] + m[15]*other[15] );
    }

    // ** Matrix4::operator[]
    inline float Matrix4::operator[]( int index ) const
    {
        _ASSERTE( index >= 0 && index < 16 );
        return m[index];
    }

    // ** Matrix4::operator[]
    inline float& Matrix4::operator[]( int index )
    {
        _ASSERTE( index >= 0 && index < 16 );
        return m[index];
    }

	// ** Matrix4::value
	inline float Matrix4::value( int row, int col ) const
	{
		return m[row * 4 + col];
	}

    // ** Matrix4::inversed
	inline Matrix4 Matrix4::inversed( void ) const
    {
		f32 m00 = value(0,0), m01 = value(0,1), m02 = value(0,2), m03 = value(0,3);
		f32 m10 = value(1,0), m11 = value(1,1), m12 = value(1,2), m13 = value(1,3);
		f32 m20 = value(2,0), m21 = value(2,1), m22 = value(2,2), m23 = value(2,3);
		f32 m30 = value(3,0), m31 = value(3,1), m32 = value(3,2), m33 = value(3,3);

		f32 v0 = m20 * m31 - m21 * m30;
		f32 v1 = m20 * m32 - m22 * m30;
		f32 v2 = m20 * m33 - m23 * m30;
		f32 v3 = m21 * m32 - m22 * m31;
		f32 v4 = m21 * m33 - m23 * m31;
		f32 v5 = m22 * m33 - m23 * m32;

		f32 t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
		f32 t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
		f32 t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
		f32 t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

		f32 invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

		f32 d00 = t00 * invDet;
		f32 d10 = t10 * invDet;
		f32 d20 = t20 * invDet;
		f32 d30 = t30 * invDet;

		f32 d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
		f32 d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
		f32 d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
		f32 d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

		v0 = m10 * m31 - m11 * m30;
		v1 = m10 * m32 - m12 * m30;
		v2 = m10 * m33 - m13 * m30;
		v3 = m11 * m32 - m12 * m31;
		v4 = m11 * m33 - m13 * m31;
		v5 = m12 * m33 - m13 * m32;

		f32 d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
		f32 d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
		f32 d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
		f32 d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

		v0 = m21 * m10 - m20 * m11;
		v1 = m22 * m10 - m20 * m12;
		v2 = m23 * m10 - m20 * m13;
		v3 = m22 * m11 - m21 * m12;
		v4 = m23 * m11 - m21 * m13;
		v5 = m23 * m12 - m22 * m13;

		f32 d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
		f32 d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
		f32 d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
		f32 d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

		return Matrix4(
			d00, d01, d02, d03,
			d10, d11, d12, d13,
			d20, d21, d22, d23,
			d30, d31, d32, d33);
	}

    // ** Matrix4::rotate
    inline Vec3 Matrix4::rotate( const Vec3& v ) const
    {
        Vec3 r;

        r.x = v.x * m[0] + v.y * m[4] + v.z * m[8];
        r.y = v.x * m[1] + v.y * m[5] + v.z * m[9];
        r.z = v.x * m[2] + v.y * m[6] + v.z * m[10];

        return r;
    }

    // ** Matrix4::translation
    inline Matrix4 Matrix4::translation( float x, float y, float z )
    {
        Matrix4 m;
        m[12] = x; m[13] = y; m[14] = z;
        return m;
    }

    // ** Matrix4::translation
    inline Matrix4 Matrix4::translation( const Vec3& position )
    {
        Matrix4 m;
        m[12] = position.x; m[13] = position.y; m[14] = position.z;
        return m;
    }

    // ** Matrix4::scale
    inline Matrix4 Matrix4::scale( float x, float y, float z )
    {
        Matrix4 m;

        m[0 ] = x;    m[1 ] = 0.0f; m[2 ] = 0.0f; m[3 ] = 0.0f;
        m[4 ] = 0.0f; m[5 ] = y;    m[6 ] = 0.0f; m[7 ] = 0.0f;
        m[8 ] = 0.0f; m[9 ] = 0.0f; m[10] = z;    m[11] = 0.0f;
        m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;

        return m;
    }

    // ** Matrix4::scale
    inline Matrix4 Matrix4::scale( const Vec3& scale )
    {
        Matrix4 m;

        m[0 ] = scale.x;    m[1 ] = 0.0f;       m[2 ] = 0.0f;       m[3 ] = 0.0f;
        m[4 ] = 0.0f;       m[5 ] = scale.y;    m[6 ] = 0.0f;       m[7 ] = 0.0f;
        m[8 ] = 0.0f;       m[9 ] = 0.0f;       m[10] = scale.z;    m[11] = 0.0f;
        m[12] = 0.0f;       m[13] = 0.0f;       m[14] = 0.0f;       m[15] = 1.0f;

        return m;
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
        explicit		Bounds( Vec3 min = Vec3( FLT_MAX, FLT_MAX, FLT_MAX ), Vec3 max = Vec3( -FLT_MAX, -FLT_MAX, -FLT_MAX ) );

        //! Adds a new point to a bounding box (extends a bounds if needed).
        Bounds&			operator << ( const Vec3& point );

        //! Adds two bounding volumes.
        const Bounds&   operator += ( const Bounds& other );

		//! Returns a transformed bounding box.
		Bounds			operator * ( const Matrix4& transform ) const;

		//! Scales the bounding box by a specified value.
		Bounds			operator * ( f32 value ) const;

        //! Returns a bounds volume.
        f32				volume( void ) const;

        //! Returns a minimum bound point.
        const Vec3&     min( void ) const;

        //! Returns a maximum bound point.
        const Vec3&     max( void ) const;

		//! Returns the bounding box center point.
		Vec3			center( void ) const;

		//! Returns the bounding box left center point.
		Vec3			leftCenter( void ) const;

		//! Returns the bounding box right center point.
		Vec3			rightCenter( void ) const;

		//! Returns the bounding box top center point.
		Vec3			topCenter( void ) const;

		//! Returns the bounding box bottom center point.
		Vec3			bottomCenter( void ) const;

		//! Returns the bounding box near center point.
		Vec3			nearCenter( void ) const;

		//! Returns the bounding box far center point.
		Vec3			farCenter( void ) const;

        //! Returns bounds width.
        f32				width( void ) const;

        //! Returns bounds height.
        f32				height( void ) const;

        //! Returns bounds depth.
        f32				depth( void ) const;

		//! Returns true if the point is inside the bounding box.
		bool			contains( const Vec3& point ) const;

        //! Returns a random point in bounding box.
        Vec3            randomPointInside( void ) const;

		//! Constructs bounding box from an array of points.
		static Bounds	fromPoints( const Vec3* points, s32 count );

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
    inline f32 Bounds::volume( void ) const {
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

	// ** Bounds::center
	inline Vec3 Bounds::center( void ) const {
		return (m_max + m_min) * 0.5f;
	}

	// ** Bounds::leftCenter
	inline Vec3 Bounds::leftCenter( void ) const
	{
		return center() - Vec3::axisX() * width() * 0.5f;
	}

	// ** Bounds::rightCenter
	inline Vec3 Bounds::rightCenter( void ) const
	{
		return center() + Vec3::axisX() * width() * 0.5f;
	}

	// ** Bounds::topCenter
	inline Vec3 Bounds::topCenter( void ) const
	{
		return center() + Vec3::axisY() * height() * 0.5f;
	}

	// ** Bounds::bottomCenter
	inline Vec3 Bounds::bottomCenter( void ) const
	{
		return center() - Vec3::axisY() * height() * 0.5f;
	}

	// ** Bounds::nearCenter
	inline Vec3 Bounds::nearCenter( void ) const
	{
		return center() - Vec3::axisZ() * depth() * 0.5f;
	}

	// ** Bounds::farCenter
	inline Vec3 Bounds::farCenter( void ) const
	{
		return center() + Vec3::axisZ() * depth() * 0.5f;
	}

    // ** Bounds::width
    inline f32 Bounds::width( void ) const {
        return fabsf( m_max.x - m_min.x );
    }

    // ** Bounds::height
    inline f32 Bounds::height( void ) const {
        return fabsf( m_max.y - m_min.y );
    }

    // ** Bounds::depth
    inline f32 Bounds::depth( void ) const {
        return fabsf( m_max.z - m_min.z );
    }

    // ** Bounds::contains
    inline bool Bounds::contains( const Vec3& point ) const
	{
        if( point.x < m_min.x || point.x > m_max.x ) return false;
		if( point.y < m_min.y || point.y > m_max.y ) return false;
		if( point.z < m_min.z || point.z > m_max.z ) return false;

		return true;
    }

    // ** Bounds::operator <<
    inline Bounds& Bounds::operator << ( const Vec3& point ) {
        for( int i = 0; i < 3; i++ ) {
            m_min[i] = min2( m_min[i], point[i] );
            m_max[i] = max2( m_max[i], point[i] );
        }
        return *this;
    }

    // ** Bounds::operator +=
    inline const Bounds& Bounds::operator += ( const Bounds& other ) {
        *this << other.m_min;
        *this << other.m_max;
        return *this;
    }

    // ** Bounds::operator *
    inline Bounds Bounds::operator * ( const Matrix4& transform ) const {
        Vec3 vertices[8] = {
			m_min,
			m_max,
			m_min + Vec3( width(), 0.0f, 0.0f ),
			m_min + Vec3( 0.0f, height(), 0.0f ),
			m_min + Vec3( 0.0f, 0.0f, depth() ),
			m_max - Vec3( width(), 0.0f, 0.0f ),
			m_max - Vec3( 0.0f, height(), 0.0f ),
			m_max - Vec3( 0.0f, 0.0f, depth() ),
		};

		Bounds result;

		for( s32 i = 0; i < 8; i++ ) {
			result << transform * vertices[i];
		}

		return result;
    }

    // ** Bounds::operator *
    inline Bounds Bounds::operator * ( f32 value ) const
	{
		return Bounds( m_min * value, m_max * value );
    }

	// ** Bounds::fromPoints
	inline Bounds Bounds::fromPoints( const Vec3* points, s32 count )
	{
		Bounds result;

		for( s32 i = 0; i < count; i++ ) {
			result << points[i];
		}

		return result;
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
                        Plane( const Vec3& normal = Vec3( 0, 0, 0 ), f32 distance = 0.0f )
                            : m_normal( normal ), m_distance( distance ) {}

                        //! Constructs a Plane instance.
                        /*!
                         \param x Plane normal X coordinate.
						 \param y Plane normal Y coordinate.
						 \param z Plane normal Z coordinate.
                         \param point Point on plane.
                         */
                        Plane( f32 x, f32 y, f32 z, f32 distance = 0.0f )
                            : m_normal( Vec3( x, y, z ) ), m_distance( distance ) {}

		//! Returns true if the plane is valid.
						operator bool ( void ) const;

        //! Projects a point onto this plane.
        Vec3            operator * ( const Vec3& point ) const;

		//! Returns true if the bounding box is behind the plane.
		bool			isBehind( const Bounds& bounds ) const;

		//! Returns true if the sphere is behind the plane.
		bool			isBehind( const Vec3& center, f32 radius ) const;

		//! Normalizes the plane.
		void			normalize( void );

		//! Returns plane normal.
		const Vec3&		normal( void ) const;

		//! Returns plane distance.
		f32				distance( void ) const;

        //! Create plane from point and normal.
        static Plane    calculate( const Vec3& normal, const Vec3& point );

		//! Returns the XY plane that passes through a point.
		static Plane	xy( const Vec3& point = Vec3() );

		//! Returns the YZ plane that passes through a point.
		static Plane	yz( const Vec3& point = Vec3() );

		//! Returns the XZ plane that passes through a point.
		static Plane	xz( const Vec3& point = Vec3() );

    private:

        //! Plane normal.
        Vec3            m_normal;

        //! Plane distance to origin.
        f32				m_distance;
    };

    // ** Plane::calculate
    inline Plane Plane::calculate( const Vec3& normal, const Vec3& point ) {
        return Plane( normal, -(normal * point) );
    }

	// ** Plane::xy
	inline Plane Plane::xy( const Vec3& point )
	{
		return Plane::calculate( Vec3::axisZ(), point );
	}

	// ** Plane::yz
	inline Plane Plane::yz( const Vec3& point )
	{
		return Plane::calculate( Vec3::axisX(), point );
	}

	// ** Plane::xz
	inline Plane Plane::xz( const Vec3& point )
	{
		return Plane::calculate( Vec3::axisY(), point );
	}

	// ** Plane::normalize
	inline void Plane::normalize( void ) {
		f32 len = 1.0f / m_normal.length();

		m_normal   *= len;
		m_distance *= len;
	}

	// ** Plane::isBehind
	inline bool Plane::isBehind( const Bounds& bounds ) const
	{
		Vec3 point = bounds.min();
		const Vec3& max = bounds.max();

		if( m_normal.x >= 0 ) point.x = max.x;
		if( m_normal.y >= 0 ) point.y = max.y;
		if( m_normal.z >= 0 ) point.z = max.z;

		if( (m_normal * point + m_distance) < 0 ) {
			return true;
		}

		return false;
	}

	// ** Plane::isBehind
	inline bool Plane::isBehind( const Vec3& center, f32 radius ) const
	{
		return m_normal * center + m_distance <= -radius;
	}

    // ** Plane::operator bool
    inline Plane::operator bool ( void ) const {
        return m_normal.length() > 0.0f;
    }

    // ** Plane::operator *
    inline Vec3 Plane::operator * ( const Vec3& point ) const {
        f32 distanceToPoint = m_normal * point + m_distance;
        return point - m_normal * distanceToPoint;
    }

	// ** Plane::normal
	inline const Vec3& Plane::normal( void ) const
	{
		return m_normal;
	}

	// ** Plane::distance
	inline f32 Plane::distance( void ) const
	{
		return m_distance;
	}

	//! N-dimensional vector.
	template<typename T>
	class Vector {
	public:

					//! Constructs Vector instance.
					Vector( s32 size );

		//! Returns the Nth element of a vector.
		const T&	operator [] ( s32 index ) const;

		//! Returns the Nth element of a vector.
		T&			operator [] ( s32 index );

		//! Calculates the dot product between two vectors.
		T			operator * ( const Vector& other ) const;

		//! Returns the vector size.
		u32			size( void ) const;

		//! Calculates length of a vector.
		T			length( void ) const;

		//! Normalizes a vector.
		T			normalize( void );

	private:

		Array<T>	m_data;	//!< Actual vector components.
	};

	// ** Vector::Vector
	template<typename T>
	Vector<T>::Vector( s32 size )
	{
		m_data.resize( size );
		for( u32 i = 0, n = ( u32 )m_data.size(); i < n; i++ ) {
			m_data[i] = 0;
		}
	}

	// ** Vector::operator []
	template<typename T>
	const T& Vector<T>::operator [] ( s32 index ) const
	{
		DC_BREAK_IF( index < 0 || index >= size() );
		return m_data[index];
	}

	// ** Vector::operator []
	template<typename T>
	T& Vector<T>::operator [] ( s32 index )
	{
		DC_BREAK_IF( index < 0 || index >= ( s32 )size() );
		return m_data[index];
	}

	// ** Vector::operator *
	template<typename T>
	T Vector<T>::operator * ( const Vector& other ) const
	{
		DC_BREAK_IF( size() != other.size() );

		T sum = 0;

		for( u32 i = 0, n = size(); i < n; i++ ) {
			sum += m_data[i] * other[i];
		}

		return sum;
	}

	// ** Vector::size
	template<typename T>
	u32 Vector<T>::size( void ) const
	{
		return ( u32 )m_data.size();
	}

	// ** Vector::length
	template<typename T>
	T Vector<T>::length( void ) const
	{
		T sum = 0;

		for( u32 i = 0, n = size(); i < n; i++ ) {
			sum += m_data[i] * m_data[i];
		}

		return sqrt( sum );
	}

	// ** Vector::normalize
	template<typename T>
	T Vector<T>::normalize( void )
	{
		T len = length();

		if( len == 0 ) {
			return len;
		}

		for( u32 i = 0, n = size(); i < n; i++ ) {
			m_data[i] /= len;
		}

		return len;
	}

	//! The MxN matrix class.
	template<typename T>
	class Matrix {
	public:

						//! Constructs the Matrix instance.
						Matrix( s32 columns, s32 rows );
						Matrix( const Matrix& other );
						~Matrix( void );

		//! Copies the matrix.
		const Matrix<T>&	operator = ( const Matrix& other );

		//! Returns the reference to an element of a matrix.
		T&					operator () ( s32 column, s32 row );

		//! Returns the reference to an element of a matrix.
		const T&			operator () ( s32 column, s32 row ) const;

		//! Multiplies two matrices.
		Matrix				operator * ( const Matrix& other ) const;

		//! Multiplies a matrix by vector.
		Vector<T>			operator * ( const Vector<T>& v ) const;

		//! Returns the Nth row of a matrix.
		Vector<T>			row( s32 index ) const;

		//! Returns the Nth column of a matrix.
		Vector<T>			col( s32 index ) const;

	private:

		s32					m_rows;	//!< The total number of rows.
		s32					m_cols;	//!< The total number of columns.
		T*					m_data;	//!< The actual matrix data.
	};

	// ** Matrix::Matrix
	template<typename T>
	Matrix<T>::Matrix( s32 columns, s32 rows ) : m_rows( rows ), m_cols( columns )
	{
		m_data = new T[columns * rows];

		for( s32 r = 0; r < rows; r++ ) {
			for( s32 c = 0; c < columns; c++ ) {
				m_data[r * columns + c] = c == r ? 1.0f : 0.0f;
			}
		}
	}

	// ** Matrix::Matrix
	template<typename T>
	Matrix<T>::Matrix( const Matrix& other ) : m_rows( other.m_rows ), m_cols( other.m_cols )
	{
		m_data = new T[m_rows * m_cols];
		memcpy( m_data, other.m_data, m_rows * m_cols * sizeof( T ) );
	}

	// ** Matrix::~Matrix
	template<typename T>
	Matrix<T>::~Matrix( void )
	{
		delete[]m_data;
	}

	// ** Matrix::operator =
	template<typename T>
	const Matrix<T>& Matrix<T>::operator = ( const Matrix& other )
	{
		m_rows = other.m_rows;
		m_cols = other.m_cols;
		m_data = new T[m_rows * m_cols];
		memcpy( m_data, other.m_data, m_rows * m_cols * sizeof( T ) );
		return *this;
	}

	// ** Matrix::operator()
	template<typename T>
	const T& Matrix<T>::operator()( s32 column, s32 row ) const
	{
		DC_BREAK_IF( column < 0 || column >= m_cols );
		DC_BREAK_IF( row    < 0 || row    >= m_rows );

		return m_data[row * m_cols + column];
	}

	// ** Matrix::operator()
	template<typename T>
	T& Matrix<T>::operator()( s32 column, s32 row )
	{
		DC_BREAK_IF( column < 0 || column >= m_cols );
		DC_BREAK_IF( row    < 0 || row    >= m_rows );

		return m_data[row * m_cols + column];
	}

	// ** Matrix::operator *
	template<typename T>
	Matrix<T> Matrix<T>::operator * ( const Matrix& other ) const
	{
		DC_BREAK_IF( m_cols != other.m_rows );
		Matrix C( other.m_cols, m_rows );

		for( s32 i = 0; i < m_rows; i++ ) {
			for( s32 j = 0; j < other.m_cols; j++ ) {
				T sum = 0;

				for( int k = 0; k < m_cols; k++ ) {
					sum = sum + (*this)( k, i ) * other( j, k );
				}

				C( j, i ) = sum;
			}
		}

		return C;
	}

	// ** Matrix::operator *
	template<typename T>
	Vector<T> Matrix<T>::operator * ( const Vector<T>& v ) const
	{
		DC_BREAK_IF( m_cols != v.size() );
		Vector<T> C( v.size() );

		for( s32 i = 0; i < m_rows; i++ ) {
			T sum = 0;

			for( int k = 0; k < m_cols; k++ ) {
				sum = sum + (*this)( k, i ) * v[k];
			}

			C[i] = sum;
		}

		return C;
	}

	// ** Matrix::row
	template<typename T>
	Vector<T> Matrix<T>::row( s32 index ) const
	{
		DC_BREAK_IF( index < 0 || index >= m_rows );

		Vector<T> r( m_cols );

		for( s32 i = 0; i < m_cols; i++ ) {
			r[i] = (*this)( i, index );
		}

		return r;
	}

	// ** Matrix::col
	template<typename T>
	Vector<T> Matrix<T>::col( s32 index ) const
	{
		DC_BREAK_IF( index < 0 || index >= m_cols );

		Vector<T> r( m_rows );

		for( s32 i = 0; i < m_rows; i++ ) {
			r[i] = (*this)( index, i );
		}

		return r;
	}

	//! The sample set.
	template<typename T>
	class Samples {
	public:

		//! Alias the sample type.
		typedef T		Sample;

						//! Constructs Samples instance.
						Samples( void );

						//! Constructs Samples instance with an array of samples.
						Samples( const Array<T>& samples );

		//! Appends a new sample to the set.
		Samples&		operator << ( const T& sample );

		//! Returns the sample with a specified index.
		const T&		operator[]( s32 index ) const;

		//! Returns the sample at specified index.
		const T&		at( s32 index ) const;

		//! Returns the mean value of a sample set.
		T				mean( void ) const;

		//! Returns the variance of a sample set.
		T				variance( void ) const;

		//! Creates a new sample set by a subtracting the mean value from each sample.
		Samples<T>		centered( void ) const;

		//! Returns the standard deviation of a sample set.
		T				sdev( void ) const;

		//! Returns the total number of samples.
		u32				size( void ) const;

	private:

		//! The cached sample set state.
		struct State {
			//! A bitset flags that store a sample set state.
			enum {
				  RecomputeMean		= BIT( 0 )
				, RecomputeVariance	= BIT( 1 )
				, RecomputeStdDev	= BIT( 2 )
				, RecomputeAll		= RecomputeMean | RecomputeVariance | RecomputeStdDev
			};

						//! Constructs State instance.
						State( void )
							: flags( 0 ), mean( 0.0f ), variance( 0.0f ), sdev( 0.0f ) {}

			//! Sets the state flag.
			void		set( u8 mask ) { flags |= mask; }

			//! Returns true if a state flag is set.
			bool		is( u8 mask ) const { return (flags & mask) != 0; }

			//! Resets the state flag.
			void		reset( u8 mask ) { flags &= ~mask; }

			u8			flags;		//!< The sample set state flags.
			T			mean;		//!< The cached mean value of a sample set.
			T			variance;	//!< The cached variance value of a sample set.
			T			sdev;		//!< The cached standard deviation value of a sample set.
		};

		Array<T>		m_samples;	//!< The array of samples.
		mutable State	m_state;	//!< The sample set state.
	};

	// ** Samples<T>::Samples
	template<typename T>
	Samples<T>::Samples( void )
	{
	}

	// ** Samples<T>::Samples
	template<typename T>
	Samples<T>::Samples( const Array<T>& samples ) : m_samples( samples )
	{
		m_state.set( State::RecomputeAll );
	}

	// ** Samples::operator[]
	template<typename T>
	const T& Samples<T>::operator[]( s32 index ) const
	{
		return at( index );
	}

	// ** Samples::operator[]
	template<typename T>
	Samples<T>& Samples<T>::operator << ( const T& sample )
	{
		m_state.set( State::RecomputeAll );
		m_samples.push_back( sample );
		return *this;
	}

	// ** Samples::at
	template<typename T>
	const T& Samples<T>::at( s32 index ) const
	{
		DC_BREAK_IF( index < 0 || index >= ( s32 )size() );
		return m_samples[index];
	}

	// ** Samples::size
	template<typename T>
	u32 Samples<T>::size( void ) const
	{
		return ( u32 )m_samples.size();
	}

	// ** Samples::centered
	template<typename T>
	Samples<T> Samples<T>::centered( void ) const
	{
		Samples<T> result;

		T m = mean();

		for( u32 i = 0, n = size(); i < n; i++ ) {
			result << m_samples[i] - m;
		}

		return result;
	}

	// ** Samples::mean
	template<typename T>
	T Samples<T>::mean( void ) const
	{
		if( !m_state.is( State::RecomputeMean ) ) {
			return m_state.mean;
		}

		// ** Recompute the mean
		T sum = 0.0f;

		for( u32 i = 0, n = ( u32 )m_samples.size(); i < n; i++ ) {
			sum += at( i );
		}

		m_state.mean = sum / T( size() );
		m_state.reset( State::RecomputeMean );

		return m_state.mean;
	}

	// ** Samples::variance
	template<typename T>
	T Samples<T>::variance( void ) const
	{
		if( !m_state.is( State::RecomputeVariance ) ) {
			return m_state.variance;
		}

		// ** Recompute the variance.
		T sum = 0.0f;
		T u   = mean();

		for( u32 i = 0, n = ( u32 )m_samples.size(); i < n; i++ ) {
			sum += pow( at( i ) - u, 2.0f );
		}

		m_state.variance = sum / static_cast<T>( size() );
		m_state.reset( State::RecomputeVariance );

		return m_state.variance;
	}

	// ** Samples::sdev
	template<typename T>
	T Samples<T>::sdev( void ) const
	{
		if( !m_state.is( State::RecomputeStdDev ) ) {
			return m_state.sdev;
		}

		m_state.sdev = sqrtf( variance() );
		m_state.reset( State::RecomputeStdDev );

		return m_state.sdev;
	}

	//! A covariance matrix class.
	template<typename T>
	class CovMatrix : public Matrix<T> {
	public:

						//! Constructs the CovMatrix instance.
						CovMatrix( s32 size );

		//! Rotates the vector until it converges.
		Vector<T>		converge( const Vector<T>& v, s32 iterations = 10 ) const;
	};

	// ** CovMatrix::CovMatrix
	template<typename T>
	CovMatrix<T>::CovMatrix( s32 size ) : Matrix<T>( size, size )
	{
	}

	// ** CovMatrix::converge
	template<typename T>
	Vector<T> CovMatrix<T>::converge( const Vector<T>& v, s32 iterations ) const
	{
		Vector<T> r = *this * v;

		for( s32 i = 0; i < iterations - 1; i++ ) {
			r = *this * r;
		}

		return r;
	}

	namespace detail {

		//! Computes the numerator of a sample set covariance.
		template<typename T>
		T covariance( const Samples<T>& a, const Samples<T>& b )
		{
			DC_BREAK_IF( a.size() != b.size() );

			T am  = a.mean();
			T bm  = b.mean();
			T cov = 0.0f;

			for( u32 i = 0, n = a.size(); i < n; i++ ) {
				const T& sa = a[i];
				const T& sb = b[i];

				cov += (sa - am) * (sb - bm);
			}

			return cov;
		}

	} // namespace detail

	//! Computes the sample covariance of a two sample sets.
	template<typename T>
	T sampleCovariance( const Samples<T>& a, const Samples<T>& b )
	{
		return detail::covariance( a, b ) / (a.size() - 1);
	}

	//! Computes the population covariance of a two sample sets.
	template<typename T>
	T populationCovariance( const Samples<T>& a, const Samples<T>& b )
	{
		return detail::covariance( a, b ) / a.size();
	}

	//! Computes the covariance matrix of a two sample sets.
	template<typename T, typename TSamples>
	CovMatrix<T> covarianceMatrix( const TSamples& samples, u32 dimensions )
	{
		// ** The size of a sample set.
		u32 n = samples.size();

		// ** Get the sample set mean.
		typename TSamples::Sample mean = samples.mean();

		// ** The resulting covariance matrix.
		CovMatrix<T> result( dimensions );

		// ** Compute the covariance matrix.
		for( u32 row = 0; row < dimensions; row++ ) {
			for( u32 col = 0; col < dimensions; col++ ) {
				T cov = 0.0f;

				for( u32 i = 0; i < n; i++ ) {
					cov += (samples[i][row] - mean[row]) * (samples[i][col] - mean[col]);
				}

				result( col, row ) = cov / n;
			}
		}

		return result;
	}

    //! DCEL data struct to simplify access to a triangular mesh topology.
    template<typename TIndex = unsigned short>
    class DCEL {
    public:

        //! Index buffer type.
        typedef Array<TIndex> IndexBuffer;

        //! The edge struct.
        struct Edge {
            TIndex          m_face;     //!< The parent face.
            TIndex          m_vertex;   //!< The first edge vertex.
            Edge*           m_next;     //!< The next edge around the face.
            Edge*           m_twin;     //!< The pair edge.

            //! Returns true if this edge is a boundary.
            bool isBoundary( void ) const
            {
                return m_twin == NULL;
            }

            //! Returns edge twin.
            Edge* twin( void ) const
            {
                return m_twin;
            }

            //! Sets the edge twin.
            void setTwin( Edge* value )
            {
                m_twin        = value;
                value->m_twin = this;
            }
        };

                            ~DCEL( void );

        //! Returns edge count.
        int                 edgeCount( void ) const;

        //! Returns edge by index.
        const Edge*         edge( int index ) const;

        //! Constructs a half edge struct from a triangle mesh.
        static DCEL         create( const IndexBuffer& indexBuffer );

    private:

                            //! Constructs a HalfEdge instance.
                            DCEL( Edge* edges, int edgeCount );

    private:

        Edge*               m_edges;        //!< Mesh edges.
        int                 m_edgeCount;    //!< The total number of edges.
    };

    // ** DCEL::DCEL
    template<typename TIndex>
    DCEL<TIndex>::DCEL( Edge* edges, int edgeCount ) : m_edges( edges ), m_edgeCount( edgeCount )
    {
    }

    // ** DCEL::DCEL
    template<typename TIndex>
    DCEL<TIndex>::~DCEL( void )
    {
    //    delete[]m_edges;
    }

    // ** DCEL::edgeCount
    template<typename TIndex>
    int DCEL<TIndex>::edgeCount( void ) const
    {
        return m_edgeCount;
    }

    // ** DCEL::edgeCount
    template<typename TIndex>
    const typename DCEL<TIndex>::Edge* DCEL<TIndex>::edge( int index ) const
    {
        return &m_edges[index];
    }

    // ** DCEL::create
    template<typename TIndex>
    DCEL<TIndex> DCEL<TIndex>::create( const IndexBuffer& indexBuffer )
    {
        typedef std::pair<TIndex, TIndex>           EdgeVertices;
        typedef std::multimap<EdgeVertices, Edge*>  EdgeCache;

        int       faceCount  = indexBuffer.size() / 3;
        int       edgeCount  = faceCount * 3;
        Edge*     edges      = new Edge[edgeCount];
        EdgeCache cache;

        memset( edges, 0, sizeof( Edge ) * edgeCount );

        for( TIndex face = 0; face < faceCount; face++ ) {
            Edge*         faceEdges   = &edges[face * 3];
            const TIndex* faceIndices = &indexBuffer[face * 3];

            for( int i = 0; i < 3; i++ ) {
                faceEdges[i].m_face   = face;
                faceEdges[i].m_vertex = faceIndices[i];
            }

            for( int i = 0; i < 3; i++ ) {
                Edge& edge = faceEdges[i];

                edge.m_next = &faceEdges[(i + 1) % 3];
                cache.insert( std::pair<EdgeVertices, Edge*>( EdgeVertices( edge.m_vertex, edge.m_next->m_vertex ), &edge ) );
            }
        }

        for( int i = 0; i < edgeCount; i++ ) {
            Edge& e = edges[i];

            typename EdgeCache::iterator z = cache.find( EdgeVertices( e.m_next->m_vertex, e.m_vertex ) );
            if( z != cache.end() && &e != z->second ) {
                assert( &e != z->second );
                e.setTwin( z->second );
            }
        }

        return DCEL<TIndex>( edges, edgeCount );
    }

	//! Angle based chart builder.
	template<typename TMesh>
	class AngularChartifier {
	public:

		//! Container type to store face to chart mapping.
		typedef Map<int, int> ChartByFace;

		//! The result of a chart builder.
		struct Result {
			ChartByFace				m_chartByFace;	//!< A chart by face registry.
			typename TMesh::Charts	m_charts;	//!< Built charts;
		};

									//! Constructs AngularChartifier instance.
									AngularChartifier( float angle = 88.0f )
										: m_angle( angle ) {}

		//! Splits the input mesh into charts.
		Result						build( TMesh& mesh ) const;

	private:

		//! Adds faces to a chart.
		void						addToChart( Result& result, TMesh& mesh, const typename TMesh::Dcel::Edge* edge, const Vec3& axis, int index ) const;

	private:

		float						m_angle;	//!< The hard angle.
	};

    //! MeshIndexer helps to build a vertex/index buffer pair from an input stream of vertices.
    template<typename TVertex, typename TCompare, typename TIndex = unsigned short>
    class MeshIndexer {
    public:

        //! Container type to store the indices.
        typedef Array<TIndex>			IndexBuffer;

        //! Container type to store the vertices.
        typedef Array<TVertex>			VertexBuffer;

		//! Clears the mesh indexer.
		void							clear( void );

        //! Adds a new vertex and returns it's index.
        TIndex                          operator += ( const TVertex& vertex );

        //! Returns the built index buffer.
        const IndexBuffer&              indexBuffer( void ) const;
		IndexBuffer&					indexBuffer( void );

        //! Returns the built vertex buffer.
        const VertexBuffer&             vertexBuffer( void ) const;
		VertexBuffer&					vertexBuffer( void );

    private:

        //! Container type to store added vertices.
        typedef Map<TVertex, TIndex, TCompare>   VertexCache;

        VertexCache                     m_cache;            //!< Vertices added to an indexer.
        VertexBuffer                    m_vertexBuffer;     //!< Built vertex buffer.
        IndexBuffer                     m_indexBuffer;      //!< Built index buffer.
    };

    // ** MeshIndexer::clear
    template<typename TVertex, typename TCompare, typename TIndex>
    void MeshIndexer<TVertex, TCompare, TIndex>::clear( void )
    {
        m_cache.clear();
		m_vertexBuffer.clear();
		m_indexBuffer.clear();
    }

    // ** MeshIndexer::indexBuffer
    template<typename TVertex, typename TCompare, typename TIndex>
    const typename MeshIndexer<TVertex, TCompare, TIndex>::IndexBuffer& MeshIndexer<TVertex, TCompare, TIndex>::indexBuffer( void ) const
    {
        return m_indexBuffer;
    }

    // ** MeshIndexer::indexBuffer
    template<typename TVertex, typename TCompare, typename TIndex>
    typename MeshIndexer<TVertex, TCompare, TIndex>::IndexBuffer& MeshIndexer<TVertex, TCompare, TIndex>::indexBuffer( void )
    {
        return m_indexBuffer;
    }

    // ** MeshIndexer::vertexBuffer
    template<typename TVertex, typename TCompare, typename TIndex>
    const typename MeshIndexer<TVertex, TCompare, TIndex>::VertexBuffer& MeshIndexer<TVertex, TCompare, TIndex>::vertexBuffer( void ) const
    {
        return m_vertexBuffer;
    }

    // ** MeshIndexer::vertexBuffer
    template<typename TVertex, typename TCompare, typename TIndex>
    typename MeshIndexer<TVertex, TCompare, TIndex>::VertexBuffer& MeshIndexer<TVertex, TCompare, TIndex>::vertexBuffer( void )
    {
        return m_vertexBuffer;
    }

    // ** MeshIndexer::add
    template<typename TVertex, typename TCompare, typename TIndex>
    TIndex MeshIndexer<TVertex, TCompare, TIndex>::operator += ( const TVertex& vertex )
    {
        typename VertexCache::iterator i = m_cache.find( vertex );
        TIndex idx = 0;

        if( i != m_cache.end() ) {
            idx = i->second;
        } else {
            idx = m_vertexBuffer.size();
            m_cache[vertex] = idx;
            m_vertexBuffer.push_back( vertex );
        }

        m_indexBuffer.push_back( idx );
        return idx;
    }

	// ** AngularChartifier::build
	template<typename TMesh>
	typename AngularChartifier<TMesh>::Result AngularChartifier<TMesh>::build( TMesh& mesh ) const
	{
		Result result;

		typename TMesh::Dcel dcel = mesh.dcel();

		for( int i = 0, n = dcel.edgeCount(); i < n; i++ ) {
			const typename TMesh::Dcel::Edge* edge = dcel.edge( i );
			addToChart( result, mesh, edge, mesh.face( edge->m_face ).normal(), result.m_charts.size() );
		}

		return result;
	}

	// ** AngularChartifier::addToChart
	template<typename TMesh>
	void AngularChartifier<TMesh>::addToChart( Result& result, TMesh& mesh, const typename TMesh::Dcel::Edge* edge, const Vec3& axis, int index ) const
	{
		// ** Skip the processed faces.
		if( result.m_chartByFace.count( edge->m_face ) ) {
			return;
		}

		float angle = degrees( acosf( axis * mesh.face( edge->m_face ).normal() ) );

		if( angle > m_angle ) {
			return;
		}

		result.m_chartByFace[edge->m_face] = index;

		if( ( int )result.m_charts.size() <= index ) {
			result.m_charts.resize( index + 1 );
			result.m_charts[index] = typename TMesh::Chart( &mesh );
		}
		result.m_charts[index].add( edge->m_face );

		const typename TMesh::Dcel::Edge* i = edge;

		do {
			if( i->twin() ) {
				addToChart( result, mesh, i->twin(), axis, index );
			}
			i = i->m_next;
		} while( i != edge );
	}

	//! TriMesh represents an indexed triangular mesh.
	template< typename TVertex, typename TIndex = unsigned short, typename TVertexCompare = std::less<TVertex> >
	class TriMesh {
	public:

		//! Face index type.
		typedef unsigned int FaceIndex;

		//! Alias this type.
		typedef TriMesh<TVertex, TIndex, TVertexCompare> Mesh;

		//! Alias the indexer type.
		typedef MeshIndexer<TVertex, TVertexCompare> Indexer;

		//! Alias the vertex type.
		typedef TVertex Vertex;

		//! Alias the index type.
		typedef TIndex Index;

		//! Container type to store mesh vertices.
		typedef Array<TVertex> Vertices;

		//! Container type to store mesh indices.
		typedef Array<TIndex> Indices;

		//! Container type to store face indices.
		typedef Array<FaceIndex> FaceIndices;

        //! DCEL type.
        typedef DCEL<TIndex> Dcel;

		//! Triangular mesh face.
		struct Face {
							            //! Constructs Face instance.
							            Face( Vertices& vertices, Indices& indices, FaceIndex index )
								            : m_vertices( vertices ), m_indices( indices ), m_index( index ) {}

			//! Returns a vertex by index.
			TVertex&		            operator[]( TIndex index ) { return vertex( index ); }
			const TVertex&              operator[]( TIndex index ) const { return vertex( index ); }

			FaceIndex					m_index;	//!< Face index.
			Vertices&		            m_vertices;	//!< Mesh vertices.
			Indices&		            m_indices;	//!< Mesh indices.

			//! Calculates face normal.
			Vec3			            normal( void ) const;

            //! Flattens the triangle to a 2D space.
            void                        flatten( const Vec3& axis, Vec2& a, Vec2& b, Vec2& c ) const;

			//! Returns the face vertex by index.
			const TVertex&	            vertex( TIndex index ) const;
			TVertex&		            vertex( TIndex index );
		};

        //! Mesh chart.
        struct Chart {
                                        //! Constructs Chart instance.
                                        Chart( Mesh* mesh = NULL )
                                            : m_mesh( mesh ) {}

            //! Adds a new face to chart.
            void                        add( FaceIndex index );

            //! Returns face count.
            int                         faceCount( void ) const;

            //! Returns chart face by index.
            Face                        face( int index ) const;

            //! Calculates avarage chart normal.
            Vec3                        normal( void ) const;

			//! Returns chart faces.
			const FaceIndices&			faces( void ) const;

			//! Calculates the UV bounding rectangle.
			void						calculateUvRect( Vec2& min, Vec2& max, int layer = 0 ) const;

            Mesh*						m_mesh;     //!< Parent trimesh.
            FaceIndices					m_faces;    //!< Chart faces.
        };

		//! Container type to store mesh charts.
		typedef Array<Chart>	Charts;

						//! Constructs TriMesh instance.
						TriMesh( Vertices& vertices, Indices& indices );

        //! Returns a DCEL data structure for this mesh.
        Dcel            dcel( void ) const;   

		//! Returns the total number of mesh faces.
		int				faceCount( void ) const;

		//! Returns a mesh face by index.
		Face			face( int index ) const;

		//! Returns mesh vertices.
		Vertices&		vertices( void ) const;

		//! Returns mesh indices.
		Indices&		indices( void ) const;

		//! Splits the mesh into charts by a specified chart builder.
		template<typename TChartBuilder>
		typename TChartBuilder::Result charts( const TChartBuilder& chartBuilder ) { return chartBuilder.build( *this ); }

	private:

		//! Mesh vertices.
		Vertices&		m_vertices;

		//! Mesh indices.
		Indices&		m_indices;
	};

	// ** TriMesh::TriMesh
	template<typename TVertex, typename TIndex, typename TVertexCompare>
	TriMesh<TVertex, TIndex, TVertexCompare>::TriMesh( Vertices& vertices, Indices& indices ) : m_vertices( vertices ), m_indices( indices )
	{
	}

	// ** TriMesh::dcel
	template<typename TVertex, typename TIndex, typename TVertexCompare>
	typename TriMesh<TVertex, TIndex, TVertexCompare>::Dcel TriMesh<TVertex, TIndex, TVertexCompare>::dcel( void ) const
	{
		return Dcel::create( m_indices );
	}

	// ** TriMesh::faceCount
	template<typename TVertex, typename TIndex, typename TVertexCompare>
	int TriMesh<TVertex, TIndex, TVertexCompare>::faceCount( void ) const
	{
		return ( int )m_indices.size() / 3;
	}

	// ** TriMesh::vertices
	template<typename TVertex, typename TIndex, typename TVertexCompare>
	typename TriMesh<TVertex, TIndex, TVertexCompare>::Vertices& TriMesh<TVertex, TIndex, TVertexCompare>::vertices( void ) const
	{
		return m_vertices;
	}

	// ** TriMesh::indices
	template<typename TVertex, typename TIndex, typename TVertexCompare>
	typename TriMesh<TVertex, TIndex, TVertexCompare>::Indices& TriMesh<TVertex, TIndex, TVertexCompare>::indices( void ) const
	{
		return m_indices;
	}

	// ** TriMesh::faceCount
	template<typename TVertex, typename TIndex, typename TVertexCompare>
	typename TriMesh<TVertex, TIndex, TVertexCompare>::Face TriMesh<TVertex, TIndex, TVertexCompare>::face( int index ) const
	{
		return Face( m_vertices, m_indices, index );
	}

	// ** TriMesh::Chart::add
	template<typename TVertex, typename TIndex, typename TVertexCompare>
	void TriMesh<TVertex, TIndex, TVertexCompare>::Chart::add( unsigned int index )
	{
        if( std::find( m_faces.begin(), m_faces.end(), index ) != m_faces.end() ) {
            return;
        }

        m_faces.push_back( index );
	}

	// ** TriMesh::Chart::faceCount
	template<typename TVertex, typename TIndex, typename TVertexCompare>
    int TriMesh<TVertex, TIndex, TVertexCompare>::Chart::faceCount( void ) const
	{
        return ( int )m_faces.size();
	}

	// ** TriMesh::Chart::face
	template<typename TVertex, typename TIndex, typename TVertexCompare>
    typename TriMesh<TVertex, TIndex, TVertexCompare>::Face TriMesh<TVertex, TIndex, TVertexCompare>::Chart::face( int index ) const
	{
        return m_mesh->face( m_faces[index] );
	}

	// ** TriMesh::Chart::faces
	template<typename TVertex, typename TIndex, typename TVertexCompare>
	const typename TriMesh<TVertex, TIndex, TVertexCompare>::FaceIndices& TriMesh<TVertex, TIndex, TVertexCompare>::Chart::faces( void ) const
	{
		return m_faces;
	}

	// ** TriMesh::Chart::normal
	template<typename TVertex, typename TIndex, typename TVertexCompare>
	Vec3 TriMesh<TVertex, TIndex, TVertexCompare>::Chart::normal( void ) const
	{
        Vec3 n( 0.0f, 0.0f, 0.0f );

        for( int i = 0; i < faceCount(); i++ ) {
            n += face( i ).normal();
        }

        n /= ( float )m_faces.size();
        n.normalize();

        return n;
	}

	// ** TriMesh::Chart::calculateUvRect
	template<typename TVertex, typename TIndex, typename TVertexCompare>
	void TriMesh<TVertex, TIndex, TVertexCompare>::Chart::calculateUvRect( Vec2& min, Vec2& max, int layer ) const
	{
		min = Vec2(  FLT_MAX,  FLT_MAX );
		max = Vec2( -FLT_MAX, -FLT_MAX );

		for( int i = 0, n = faceCount(); i < n; i++ ) {
			const Face& f = face( i );

			for( int j = 0; j < 3; j++ ) {
				const TVertex& v = f.vertex( j );

				min.x = min2( v.uv[layer].x, min.x );
				min.y = min2( v.uv[layer].y, min.y );
				max.x = max2( v.uv[layer].x, max.x );
				max.y = max2( v.uv[layer].y, max.y );
			}
		}
	}

	// ** TriMesh::Face::flatten
	template<typename TVertex, typename TIndex, typename TVertexCompare>
	void TriMesh<TVertex, TIndex, TVertexCompare>::Face::flatten( const Vec3& axis, Vec2& a, Vec2& b, Vec2& c ) const
	{
        Plane plane( axis );

        const Vec3& v0 = vertex( 0 ).position;
        const Vec3& v1 = vertex( 1 ).position;
        const Vec3& v2 = vertex( 2 ).position;
/*
        Vec3 N = normal();
        Vec3 T = v1 - v0; T.normalize();
        Vec3 B = N % T;   B.normalize();

        Matrix4 Tr;
        Tr.setRow( 0, Vec4( T.x, T.y, T.z, 0.0f ) );
        Tr.setRow( 1, Vec4( B.x, B.y, B.z, 0.0f ) );
        Tr.setRow( 2, Vec4( N.x, N.y, N.z, 0.0f ) );
        Tr.setRow( 3, Vec4( 0, 0, 0, 1.0f ) );

        Vec3 r0 = Tr.inversed().rotate( v0 );
        Vec3 r1 = Tr.inversed().rotate( v1 );
        Vec3 r2 = Tr.inversed().rotate( v2 );

        assert( fabs( r0.z - r1.z ) < 0.1f && fabs( r1.z - r2.z ) < 0.1f && fabs( r2.z - r0.z ) < 0.1f );

        a = Vec2( r0.x, r0.y );
        b = Vec2( r1.x, r1.y );
        c = Vec2( r2.x, r2.y );
*/

        Vec3 p0 = plane * v0;
        Vec3 p1 = plane * v1;
        Vec3 p2 = plane * v2;

        if( axis.x == 1.0f ) {
            a = Vec2( p0.y, p0.z );
            b = Vec2( p1.y, p1.z );
            c = Vec2( p2.y, p2.z );
        }
        else if( axis.y == 1.0f ) {
            a = Vec2( p0.x, p0.z );
            b = Vec2( p1.x, p1.z );
            c = Vec2( p2.x, p2.z );
        }
        else {
            a = Vec2( p0.x, p0.y );
            b = Vec2( p1.x, p1.y );
            c = Vec2( p2.x, p2.y );
        }
	}

	// ** TriMesh::Face::normal
	template<typename TVertex, typename TIndex, typename TVertexCompare>
	Vec3 TriMesh<TVertex, TIndex, TVertexCompare>::Face::normal( void ) const
	{
		assert( m_index * 3 + 3 <= m_indices.size() );
		TIndex*	    indices = &m_indices[m_index * 3];
		const Vec3&	v0		= m_vertices[indices[0]].position;
		const Vec3&	v1		= m_vertices[indices[1]].position;
		const Vec3&	v2		= m_vertices[indices[2]].position;

		Vec3 normal = (v1 - v0) % (v2 - v0);
		normal.normalize();

		return normal;
	}

	// ** TriMesh::Face::vertex
	template<typename TVertex, typename TIndex, typename TVertexCompare>
	const TVertex& TriMesh<TVertex, TIndex, TVertexCompare>::Face::vertex( TIndex index ) const
	{
		assert( m_index * 3 + 3 <= m_indices.size() );
		TIndex* indices = &m_indices[m_index * 3];
		return  m_vertices[indices[index]];
	}

	// ** TriMesh::Face::vertex
	template<typename TVertex, typename TIndex, typename TVertexCompare>
	TVertex& TriMesh<TVertex, TIndex, TVertexCompare>::Face::vertex( TIndex index )
	{
		assert( m_index * 3 + 3 <= m_indices.size() );
		TIndex* indices = &m_indices[m_index * 3];
		return  m_vertices[indices[index]];
	}

	//! Generates a second UV set for a lightmapping.
	template< typename TMesh, typename TChartifier = AngularChartifier<TMesh>, typename TRectanglePacker = RectanglePacker<float> >
	class UvGenerator {
	public:

		//! Mesh UV set type.
		typedef Array<Vec2>			UvSet;

									//! Constructs an instance if UvGenerator
									UvGenerator( TMesh& mesh, u32 uvLayer, const TChartifier& chartifier = TChartifier(), const TRectanglePacker& packer = TRectanglePacker() )
										: m_mesh( mesh ), m_chartifier( chartifier ), m_packer( packer ), m_uvLayer( uvLayer ) {}

		//! Generates a new UV set inplace.
		void						generate( typename TMesh::Vertices& vertices, typename TMesh::Indices& indices );

	private:

		//! Flattens the mesh chart and returns the resulting UV set.
		UvSet						flatten( const typename TMesh::Chart& chart ) const;

		//! Tries to rotate a UV set to minimize the occupied texture space.
		void						rotate( UvSet& uv ) const;

		//! Calculates the bounding rect of a UV set.
		Vec2						calculateBoundingRect( const UvSet& uv, Vec2& min, Vec2& max ) const;

		//! Calculates the bounding rect of a specified portion of a mesh.
		Vec2						calculateBoundingRect( const typename TMesh::Indices& indices, const typename TMesh::Vertices& vertices, Vec2& min, Vec2& max ) const;

		//! Packs the chart rectangles to a minimal needed space.
		Vec2						pack( void );

	private:

		TMesh&						m_mesh;			//!< The mesh being processed. 
		TChartifier					m_chartifier;	//!< The chartifier to split the mesh into set of charts.
		TRectanglePacker			m_packer;		//!< The rectangle packer to use.
		typename TMesh::Vertices	m_vertices;		//!< Mesh vertices.
		typename TMesh::Indices		m_indices;		//!< Mesh indices.
		u32							m_uvLayer;		//!< The UV layer index to generate.
	};

	// ** UvGenerator::flatten
	template<typename TMesh, typename TChartifier, typename TRectanglePacker>
	typename UvGenerator<TMesh, TChartifier, TRectanglePacker>::UvSet UvGenerator<TMesh, TChartifier, TRectanglePacker>::flatten( const typename TMesh::Chart& chart ) const
	{
		UvSet result;

		for( int j = 0; j < chart.faceCount(); j++ ) {
			typename TMesh::Face face = chart.face( j );
			Vec3 normal = chart.normal();

			Vec2 v[3];
			face.flatten( normal.ordinal(), v[0], v[1], v[2] );

			for( int i = 0; i < 3; i++ ) {
				result.push_back( v[i] );
			}
		}

		return result;
	}

	// ** UvGenerator::calculateBoundingRect
	template<typename TMesh, typename TChartifier, typename TRectanglePacker>
	Vec2 UvGenerator<TMesh, TChartifier, TRectanglePacker>::calculateBoundingRect( const Array<Vec2>& uv, Vec2& min, Vec2& max ) const
	{
		min = Vec2(  FLT_MAX,  FLT_MAX );
		max = Vec2( -FLT_MAX, -FLT_MAX );

		for( s32 i = 0, n = ( s32 )uv.size(); i < n; i++ ) {
			min.x = min2( min.x, uv[i].x );
			max.x = max2( max.x, uv[i].x );
			min.y = min2( min.y, uv[i].y );
			max.y = max2( max.y, uv[i].y );
		}

		return max - min;
	}

	// ** UvGenerator::calculateBoundingRect
	template<typename TMesh, typename TChartifier, typename TRectanglePacker>
	Vec2 UvGenerator<TMesh, TChartifier, TRectanglePacker>::calculateBoundingRect( const typename TMesh::Indices& indices, const typename TMesh::Vertices& vertices, Vec2& min, Vec2& max ) const
	{
		min = Vec2(  FLT_MAX,  FLT_MAX );
		max = Vec2( -FLT_MAX, -FLT_MAX );

		for( u32 i = 0, n = ( u32 )indices.size(); i < n; i++ ) {
			const Vec2& uv = vertices[indices[i]].uv[m_uvLayer];
			min.x = min2( uv.x, min.x );
			min.y = min2( uv.y, min.y );
			max.x = max2( uv.x, max.x );
			max.y = max2( uv.y, max.y );
		}

		return max - min;
	}

	// ** UvGenerator::pack
	template<typename TMesh, typename TChartifier, typename TRectanglePacker>
	Vec2 UvGenerator<TMesh, TChartifier, TRectanglePacker>::pack( void )
	{
		float w = 1;
		float h = 1;
		bool expandWidth = true;

		while( !m_packer.place( w, h ) ) {
			if( expandWidth ) {
				w += 0.1f;
				expandWidth = false;
			} else {
				h += 0.1f;
				expandWidth = true;
			}
		}

		return Vec2( w, h );
	}

	// ** UvGenerator::rotate
	template<typename TMesh, typename TChartifier, typename TRectanglePacker>
	void UvGenerator<TMesh, TChartifier, TRectanglePacker>::rotate( UvSet& uv ) const
	{
		Samples<Vec2>	 samples = Samples<Vec2>( uv ).centered();
		CovMatrix<float> cov	 = covarianceMatrix<float>( samples, 2 );
		Vector<float>	 e1( 2 ), e2( 2 );

		e1[0] = 1; e1[1] = 1;
		e1 = cov.converge( e1 );
		e1.normalize();

		e2[0] = e1[1]; e2[1] = -e1[0];

		UvSet rotated = uv;

		for( s32 i = 0, n = ( s32 )rotated.size(); i < n; i++ ) {
			Vec2& v = rotated[i];
			v = Vec2( e1[0] * v.x + e1[1] * v.y, e2[0] * v.x + e2[1] * v.y );
		}

		Vec2 beforeRotationMin, beforeRotationMax;
		Vec2 afterRotationMin,  afterRotationMax;

		Vec2 beforeSize = calculateBoundingRect( uv, beforeRotationMin, beforeRotationMax );
		Vec2 afterSize  = calculateBoundingRect( rotated, afterRotationMin, afterRotationMax );

		if( (afterSize.x * afterSize.y) < (beforeSize.x * beforeSize.y) ) {
			uv = rotated;
		}
	}

	// ** UvGenerator::generate
	template<typename TMesh, typename TChartifier, typename TRectanglePacker>
	void UvGenerator<TMesh, TChartifier, TRectanglePacker>::generate( typename TMesh::Vertices& vertices, typename TMesh::Indices& indices )
	{
		m_indices  = m_mesh.indices();
		m_vertices = m_mesh.vertices();

		TMesh					mesh( m_vertices, m_indices );
		typename TMesh::Indexer	indexer;
		Vec2					min, max;

		// ** Split the mesh into the charts
		typename TChartifier::Result charts = mesh.charts( m_chartifier );

		Array<typename TMesh::Indices> chartVertices;
		chartVertices.resize( charts.m_charts.size() );

		for( s32 i = 0, n = ( s32 )charts.m_charts.size(); i < n; i++ )
		{
			// ** Get the chart.
			typename TMesh::Chart& chart = charts.m_charts[i];

			// ** Flatten the mesh chart by projecting to the ordinal axis of it's normal.
			UvSet uv = flatten( chart );

			rotate( uv );

			// ** Calculate the resulting UV bounding rect.
			Vec2 size = calculateBoundingRect( uv, min, max );

			// ** Output a new set of vertices with this UV set.
			for( s32 j = 0; j < chart.faceCount(); j++ ) {
				typename TMesh::Face face = chart.face( j );

				for( int k = 0; k < 3; k++ ) {
					typename TMesh::Vertex vtx = face.vertex( k );

					Vec2 v = uv.front();
					uv.erase( uv.begin() );

					if( size.x > size.y ) {
						vtx.uv[m_uvLayer] = Vec2( v.x - min.x, v.y - min.y );
					} else {
						vtx.uv[m_uvLayer] = Vec2( v.y - min.y, v.x - min.x );
					}
					chartVertices[i].push_back( indexer += vtx );
				}
			}
		}

		m_vertices = indexer.vertexBuffer();
		m_indices  = indexer.indexBuffer();

		for( s32 i = 0, n = ( s32 )charts.m_charts.size(); i < n; i++ )
		{
			const typename TMesh::Chart& chart = charts.m_charts[i];
			Vec2 size = calculateBoundingRect( chartVertices[i], m_vertices, min, max );

			m_packer.add( max2( size.x, size.y ), min2( size.x, size.y ) );
		}

		// ** Pack the chart rectangles.
		pack();

		indexer = TMesh::Indexer();

		for( s32 i = 0, n = ( s32 )charts.m_charts.size(); i < n; i++ )
		{
			const typename TRectanglePacker::Rect& rect = m_packer.rect( i );

			for( u32 j = 0, nv = ( u32 )chartVertices[i].size(); j < nv; j++ ) {
				typename TMesh::Vertex vtx = m_vertices[chartVertices[i][j]];

				vtx.uv[m_uvLayer] = Vec2( rect.x + vtx.uv[m_uvLayer].x, rect.y + vtx.uv[m_uvLayer].y);
				indexer += vtx;
			}
		}

		vertices = indexer.vertexBuffer();
		indices  = indexer.indexBuffer();

		// ** Normalize the UV set.
		Vec2 size = calculateBoundingRect( indices, vertices, min, max );

		for( s32 i = 0, n = ( s32 )vertices.size(); i < n; i++ ) {
			Vec2& uv = vertices[i].uv[m_uvLayer];

			f32 u = uv.x;
			f32 v = uv.y;

			DC_BREAK_IF( u < min.x || v < min.y );
			DC_BREAK_IF( u > max.x || v > max.y );

			uv.x = (u - min.x) / (max.x - min.x);
			uv.y = (v - min.y) / (max.y - min.y);

			DC_BREAK_IF( uv.x < 0.0f || uv.x > 1.0f );
			DC_BREAK_IF( uv.y < 0.0f || uv.y > 1.0f );
		}
	}

    //! Packs a set of small rectangles to a bigger one.
	template<typename T>
    class RectanglePacker {
    public:

        //! A rectangular area that will be packed.
        struct Rect {
                        //! Constructs Rect instance.
                        Rect( T x, T y, T width, T height )
                            : x( x ), y( y ), width( width ), height( height ) {}

            T			x;      //!< Left side coordinate.
            T			y;      //!< Top side coordinate.
            T			width;  //!< Rectangle width.
            T			height; //!< Rectangle height;

            //! Compares two rectangles by size.
            bool        operator == ( const Rect& other ) const { return width == other.width && height == other.height; }

            //! Compares two rectangles by size.
            bool        operator <= ( const Rect& other ) const { return width <= other.width && height <= other.height; }

            //! Compares two rectangles by size.
            bool        operator >  ( const Rect& other ) const { return width > other.width || height > other.height; }

            //! Compares two rectangles by area.
            static bool compareByArea( const Rect* a, const Rect* b ) { return a->width * a->height > b->width * b->height; }
        };

        //! Container type to store rectangles being packed.
        typedef Array<Rect> Rectangles;

        //! Rectangle sorting predicate function.
        typedef bool (*SortPredicate)( const Rect* a, const Rect* b );

								//! Constructs RectanglePacker instance.
								RectanglePacker( T padding = 0, T margin = 0 )
									: m_padding( padding ), m_margin( 0 ) {}

        //! Adds a rectangle to set.
        /*!
         \param width Rectangle width.
         \param height Rectangle height.
         \return Rectangle index.
        */
        u32                     add( T width, T height );

        //! Packs added rectangles.
        /*!
         \param width Target area width.
         \param height Target area height.
         \return true if all rectangles can be placed to a destination area.
        */
        bool                    place( T width, T height, SortPredicate predicate = NULL );

        //! Returns the total number of rectangles.
        s32                     rectCount( void ) const;

        //! Returns the rectangle by index.
        const Rect&             rect( s32 index ) const;

    private:

		T						m_padding;		//!< The pading in pixels between the rectangles.
		T						m_margin;		//!< The margin from the border of root rect.
        Rectangles              m_rectangles;   //!< Rectangles being packed.
    };

    // ** RectanglePacker::add
	template<typename T>
    u32 RectanglePacker<T>::add( T width, T height )
    {
        m_rectangles.push_back( Rect( 0, 0, width + m_padding, height + m_padding ) );
        return m_rectangles.size() - 1;
    }

    // ** RectanglePacker::rectCount
	template<typename T>
    s32 RectanglePacker<T>::rectCount( void ) const
    {
        return ( s32 )m_rectangles.size();
    }

    // ** RectanglePacker::rect
	template<typename T>
    const typename RectanglePacker<T>::Rect& RectanglePacker<T>::rect( s32 index ) const
    {
        return m_rectangles[index];
    }

    // ** RectanglePacker::place
	template<typename T>
    bool RectanglePacker<T>::place( T width, T height, SortPredicate predicate )
    {
        //! Rectangle packer node.
        struct Node {
									//! Constructs Node instance.
									Node( const RectanglePacker* packer, T x, T y, T width, T height )
										: m_packer( packer ), m_isFree( true ), m_rect( x, y, width, height ) {}

            //! Returns true if this is a leaf node.
            bool					isLeaf( void ) const { return m_left.get() == NULL && m_right.get() == NULL; }

			const RectanglePacker*	m_packer;	//!< Parent rectangle packer.
            bool					m_isFree;   //!< Flag that marks this node as free.
            AutoPtr<Node>			m_left;     //!< Left child node.
            AutoPtr<Node>			m_right;    //!< Right child node.
            Rect					m_rect;     //!< Node rectangle.

            //! Assigns a rectangle to node.
            bool assign( Rect& placed )
            {
                // ** This is not a leaf node - go down the hierarchy.
                if( !isLeaf() ) return m_left->assign( placed ) ? true : m_right->assign( placed );

                // ** The packed rectangle is too big to place it inside this node.
                if( placed > m_rect || !m_isFree ) return false;

                // ** The packed rectangle perfectly fits to this node.
                if( placed == m_rect ) {
                    placed.x = m_rect.x;
                    placed.y = m_rect.y;
                    m_isFree = false;
                    return true;
                }

                // ** Store this rectangle inside the node.
                T dw = m_rect.width  - placed.width;
                T dh = m_rect.height - placed.height;

                if( dw > dh ) {
                    m_left.reset( new Node( m_packer, m_rect.x,                m_rect.y, placed.width,                m_rect.height ) );
                    m_right.reset( new Node( m_packer, m_rect.x + placed.width, m_rect.y, m_rect.width - placed.width, m_rect.height ) );
                } else {
                    m_left.reset( new Node( m_packer, m_rect.x, m_rect.y,                 m_rect.width, placed.height ) );
                    m_right.reset( new Node( m_packer, m_rect.x, m_rect.y + placed.height, m_rect.width, m_rect.height - placed.height ) );
                }

                return assign( placed );
            }
        };

		// ** Build an array of pointer to preserve the rectangle order.
		Array<Rect*> rectangles;

		for( s32 i = 0, n = ( s32 )m_rectangles.size(); i < n; i++ ) {
			rectangles.push_back( &m_rectangles[i] );
		}

        // ** Sort rectangles.
        std::sort( rectangles.begin(), rectangles.end(), predicate ? predicate : Rect::compareByArea );

        // ** Create a root node.
        Node root( this, m_margin, m_margin, width - m_margin, height - m_margin );

        // ** Place them to a destination area.
        for( s32 i = 0, n = ( s32 )rectangles.size(); i < n; i++ ) {
            if( !root.assign( *rectangles[i] ) ) {
                return false;
            }
        }

        return true;
    }

} // namespace relight

#pragma warning( pop )

#endif	/*	!__Relight_Types_H__	*/
