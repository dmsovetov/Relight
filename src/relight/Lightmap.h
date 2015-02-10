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

#ifndef __Relight_Lightmap_H__
#define __Relight_Lightmap_H__

#include "Relight.h"
#include "Vector.h"

namespace relight {

    //! A single lightmap pixel data.
    struct Lumel {
        //! Lumel state flags
        enum Flags {
            Valid = 0x1,
        };

        Vec3    m_position;     //!< Lumel world space position.
        Vec3    m_normal;       //!< Lumel world space normal.
        Color   m_color;        //!< Baked color.

        int     m_photons;      //!< Amount of photons stored (valid only for photon maps).
        Color   m_gathered;     //!< Gathered photons color.

        int     m_flags;

                Lumel( void ) : m_photons( 0 ), m_flags( 0 ) {}

                operator bool() const { return m_flags & Valid; }
    };

    //! Holds a rendered lightmap data.
    class Lightmap {
    public:

                                //! Constructs a new Lightmap instance.
                                Lightmap( int width, int height );

        //! Returns a lightmap width
        int                     width( void ) const;

        //! Returns a lightmap height
        int                     height( void ) const;

        //! Returns a lumel data.
        Lumel*                  lumels( void );

        //! Returns a lumel at a given UV coordinates.
        Lumel&                  lumel( const Uv& uv );
        const Lumel&            lumel( const Uv& uv ) const;

        //! Returns a lumel at a given buffer coordinates.
        Lumel&                  lumel( int x, int y );
        const Lumel&            lumel( int x, int y ) const;

        //! Adds an instance to this lightmap.
        /*!
         This function initializes all lumels corresponding to a given instance.
         \param instance Instance to be added.
         */
        virtual RelightStatus   addInstance( const Instance* instance, bool copyVertexColor = false );

        //! Saves a lightmap to file.
        bool                    save( const String& fileName ) const;

        //! Converts a lightmap to buffer with 8-bit color.
        /*!
         Warning: you should free the resulting buffer by yourself.
         */
        unsigned char*          toRgb8( void ) const;

        //! Converts a lightmap to buffer with 32-bit floating point color.
        /*!
         Warning: you should free the resulting buffer by yourself.
         */
        float*                  toRgb32F( void ) const;

    protected:

        //! Initializes all lumels corresponding to a given mesh.
        void                    initializeLumels( const Mesh* mesh, bool copyVertexColor );

        //! Initializes all lumels corresponsing to a given face.
        void                    initializeLumels( const VertexBuffer& vertices, Index v0, Index v1, Index v2, bool copyVertexColor );

        //! Initializes a given face lumel.
        void                    initializeLumel( Lumel& lumel, const Face& face, const Uv& barycentric, bool copyVertexColor );

    protected:

        //! Lightmap width.
        int                     m_width;

        //! Lightmap height.
        int                     m_height;

        //! Lightmap data.
        Array<Lumel>            m_lumels;
    };

    // ** class Photonmap
    class Photonmap : public Lightmap {
    public:

                                //! Constructs a new Photonmap instance
                                Photonmap( int width, int height );

        //! Does a gathering of photons for all lumels.
        void                    gather( int radius );

        //! Adds an instance to this photonmap.
        virtual RelightStatus   addInstance( const Instance* instance, bool copyVertexColor = false );

    private:

        //! Gathers surrounding photons to lumel
        Color                   gather( int x, int y, int radius ) const;
    };

} // namespace relight

#endif  /*  !defined( __Relight_Lightmap_H__ ) */
