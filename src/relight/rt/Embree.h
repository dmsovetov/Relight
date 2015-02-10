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

#ifndef __Relight_RT_Embree_H__
#define __Relight_RT_Embree_H__

#include "Tracer.h"

#include <embree2/rtcore.h>
#include <embree2/rtcore_ray.h>

namespace relight {

namespace rt {

    // ** class Embree
    class Embree : public ITracer {
    public:

                        Embree( void );
        virtual         ~Embree( void );

        // ** ITracer
        virtual bool    traceSegment( const Vec3& start, const Vec3& end, Hit* result = NULL );
        virtual void    traceSegments( Segment segments[4] );
        virtual void    addInstance( const Instance* instance );
        virtual void    begin( void );
        virtual void    end( void );

    private:

        //! Embree vertex layout.
        struct EmVertex {
            float x, y, z, a;
        };

        //! Embree triangle layout.
        struct EmFace {
            int v0, v1, v2;
        };

        //! Embree scene.
        RTCScene                m_scene;

        //! Instance registry.
        Array<const Instance*>  m_instances;
    };

} // namespace rt

} // namespace relight

#endif  /*  !defined( __Relight_RT_Embree_H__ ) */
