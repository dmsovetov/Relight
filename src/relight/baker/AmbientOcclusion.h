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

#ifndef __Relight_Bake_AmbientOcclusion_H__
#define __Relight_Bake_AmbientOcclusion_H__

#include "Baker.h"

namespace relight {

namespace bake {

    //! Bakes ambient occlusion
    class AmbientOcclusion : public Baker {
    public:

                            //! Constructs a AmbientOcclusion instance.
                            /*!
                             \param samples Amount of samples to be taken.
                             \param progress Progress callback.
                             \param iterator Bake iterator.
                             \param occludedFraction Fraction of samples taken that must be occluded in order to reach full occlusion.
                             \param maxDistance Maximum distance for an object to cause occlusion on another object.
                             */
                            AmbientOcclusion( const Scene* scene, Progress* progress, BakeIterator* iterator, int samples, float occludedFraction, float maxDistance );

    protected:

        //! Bakes an ambient occlusion to a single lightmap pixel.
        virtual void        bakeLumel( Lumel& lumel );

    private:

        //! Amount of samples.
        int                 m_samples;

        //! Occluded fraction.
        float               m_occludedFraction;

        //! Max occlusion distance.
        float               m_maxDistance;
    };

} // namespace bake

} // namespace relight

#endif  /*  !defined( __Relight_Bake_AmbientOcclusion_H__ ) */
