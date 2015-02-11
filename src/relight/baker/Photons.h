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

#ifndef __Relight_Bake_Photons_H__
#define __Relight_Bake_Photons_H__

#include "Baker.h"

namespace relight {

namespace bake {

    // ** class Photons
    class Photons : public Baker {
    public:

                                //! Constructs a Photons instance.
                                /*!
                                 \param scene Scene to be baked.
                                 \param progress Progress callback.
                                 \param iterator Bake iterator.
                                 \param passCount Amount of photon passes.
                                 \param maxDepth Maximum photon tracing depth (number of light bounces).
                                 \param energyThreshold The minimum energy that photon should have to continue tracing.
                                 \param maxDistance The reflected light maximum distance. All intersections above this value will be ignored.
                                 */
                                Photons( const Scene* scene, Progress* progress, BakeIterator* iterator, int passCount, int maxDepth, float energyThreshold, float maxDistance );
        // ** Baker
        virtual RelightStatus   bake( void );

    private:

        //! Emits photons from a given light.
        void                    emitPhotons( const Light* light );

        //! Traces a photon path for a given depth.
        /*!
         Traces a photon path for a given depth. Each time the photon bounces the
         reflected light is stored to a photon map.

         \param attenuation Light attenuation model.
         \param position Photon's start position.
         \param direction Photon's direction.
         \param color Photon's color.
         \param energy Photon's energy.
         \param depth Current trace depth.
         */
        void                    trace( const LightAttenuation* attenuation, const Vec3& position, const Vec3& direction, const Color& color, float energy, int depth );

        //! Stores a photon bounce.
        void                    store( Photonmap* photonmap, const Color& color, const Uv& uv );

    private:

        //! Amount of photon passes, at each pass there will be emitted N photons.
        int                     m_passCount;

        //! Maximum tracing depth.
        int                     m_maxDepth;

        //! Photon energy threshold.
        float                   m_energyThreshold;

        //! Maximum intersection distance
        float                   m_maxDistance;
    };

} // namespace bake

} // namespace relight

#endif  /*  !defined(__Relight_Bake_Photons_H__) */
