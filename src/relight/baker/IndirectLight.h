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

#ifndef __Relight_Bake_IndirectLight_H__
#define __Relight_Bake_IndirectLight_H__

#include "Baker.h"

namespace relight {

namespace bake {

    // ** class IndirectLight
    class IndirectLight : public Baker {
    public:

                                //! Constructs a new IndirectLight instance.
                                /*!
                                 \param scene Scene to be baked.
                                 \param samples Amount of final gather samples.
                                 \param maxDistance Maximum distance to gather photons at.
                                 \param radius Final gather radius.
                                 */
                                IndirectLight( const Scene* scene, int samples, float maxDistance, int radius );

        //! Bakes an indirect lighting to a texture using a final gather.
        virtual RelightStatus   bake( void );

    protected:

        //! Bakes an indirect light to a given lumel.
        virtual void            bakeLumel( Lumel& lumel );

        //! Geathers photons at a given point with radius.
        Color                   geather( const Lightmap* lightmap, int x, int y ) const;

    private:

        //! Final gather samples.
        int                     m_samples;

        //! Final gather distance.
        float                   m_maxDistance;

        //! Final gather radius.
        int                     m_radius;
    };

} // namespace bake

} // namespace relight

#endif   /*  !defined( __Relight_Bake_IndirectLight_H__ ) */
