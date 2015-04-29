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

namespace relight {

    //! A single lightmap pixel data.
    struct Lumel {
        //! Lumel state flags
        enum Flags {
            Valid = 0x1,
        };

        Index   m_faceIdx;      //!< Lumel face idx.

        Vec3    m_position;     //!< Lumel world space position.
        Vec3    m_normal;       //!< Lumel world space normal.
        Rgb     m_color;        //!< Baked color.

        int     m_photons;      //!< Amount of photons stored (valid only for photon maps).
        Rgb     m_gathered;     //!< Gathered photons color.

        int     m_flags;

                Lumel( void ) : m_faceIdx( -1 ), m_photons( 0 ), m_flags( 0 ) {}

                operator bool() const { return m_flags & Valid; }
    };

    //! Holds a rendered lightmap data.
    class Lightmap {
    friend class Relight;
    public:

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

		//! Returns a pixel rect by a UV bounds.
		void					rect( const Rect& uv, int& x1, int& y1, int& x2, int& y2 ) const;

        //! Adds an instance to this lightmap.
        /*!
         This function initializes all lumels corresponding to a given instance.
         \param mesh Mesh to be added.
         */
        virtual RelightStatus   addMesh( const Mesh* mesh );

        //! Fills all illegal pixels with a nearest valid color.
        void                    expand( void );

        //! Blurs a lightmap
        void                    blur( void );

        //! Saves a lightmap to file.
        bool                    save( const String& fileName, StorageFormat format ) const;

        //! Converts a lightmap to buffer with 8-bit RGB color with double LDR encoding.
        /*!
         In double LDR encoding each pixel component is divided by 2 and stored in puffer.
         When rendering the final image with a dLDR-encoded lightmap it should be multiplied
         by 2 to restore the original data.

         Warning: you should free the resulting buffer by yourself.
         */
        unsigned char*          toDoubleLdr( void ) const;

        //! Converts a lightmap to buffer with 8-bit RGBA color with RGBM encoding.
        /*!
         In RGBM encoding each pixel is represented by an LDR RGB color and a multiplier
         to convert it to HDR at a runtime.

         Warning: you should free the resulting buffer by yourself.
        */
        unsigned char*          toRgbmLdr( void ) const; 

        //! Converts a lightmap to buffer with 32-bit floating point color HDR.
        /*!
         Warning: you should free the resulting buffer by yourself.
         */
        float*                  toHdr( void ) const;

    protected:

                                //! Constructs a new Lightmap instance.
                                Lightmap( int width, int height );

        //! Initializes all lumels corresponding to a given mesh.
        void                    initializeLumels( const Mesh* mesh );

        //! Initializes all lumels corresponsing to a given face.
        void                    initializeLumels( const Face& face );

        //! Initializes a given face lumel.
        void                    initializeLumel( Lumel& lumel, const Face& face, const Uv& barycentric );

        //! Fills invalid lumel.
        void                    fillInvalidAt( int x, int y, const Rgb& color );

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
    friend class Relight;
    public:

        //! Does a gathering of photons for all lumels.
        void                    gather( int radius );

        //! Adds an instance to this photonmap.
        virtual RelightStatus   addMesh( const Mesh* mesh, bool copyVertexColor = false );

    private:

                                //! Constructs a new Photonmap instance
                                Photonmap( int width, int height );

        //! Gathers surrounding photons to lumel
        Rgb                     gather( int x, int y, int radius ) const;
    };

} // namespace relight

#endif  /*  !defined( __Relight_Lightmap_H__ ) */
