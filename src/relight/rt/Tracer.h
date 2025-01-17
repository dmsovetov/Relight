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


#ifndef __Relight_RT_Tracer_H__
#define __Relight_RT_Tracer_H__

#include "../Relight.h"

namespace relight {

namespace rt {

    //! Ray tracing result.
    struct Hit {
                        //! Constructs a new Hit instance.
                        Hit( void ) : m_mesh( NULL ) {}

                        operator bool() const { return m_mesh != NULL; }

        Vec3            m_point;    //!< Intersection point.
        Vec3            m_normal;   //!< Intersection normal.
        Rgba            m_color;    //!< Intersection color.
        Uv              m_uv;       //!< Intersection texture coordinates.
        const Mesh*     m_mesh;     //!< Intersected mesh instance.
    };

    //! A ray tracer segment.
    struct Segment {

        //! Segment start point.
        Vec3            m_start;

        //! Segment end point.
        Vec3            m_end;

        //! Ray tracing result.
        Hit             m_hit;
    };

    //! Enumeration of flags for trace results.
    enum HitFlags {
        HitPoint    = 1 << 0,
        HitUv       = 1 << 1,
        HitColor    = 1 << 2,
        HitNormal   = 1 << 3,
        HitUseAlpha = 1 << 4,
        HitAll      = HitPoint | HitUv | HitColor | HitNormal
    };

    //! A base class for all scene ray tracers.
    class ITracer {
    public:

        virtual                 ~ITracer( void ) {}

        //! Traces a given segment.
        /*!
         \param start Segment start point.
         \param end Segment end point.
         \param result Ray tracing result.
         \return True if a ray intersects a scene, otherwise false.
         */
        virtual Hit            traceSegment( const Vec3& start, const Vec3& end, int flags = HitAll, int step = 0 ) = 0;

        //! Test a given segment for intersection with scene.
        /*!
         \param start Segment start point.
         \param end Segment end point.
         \return True if a ray intersects a scene, otherwise false.
         */
        virtual bool            test( const Vec3& start, const Vec3& end ) = 0;

        //! Traces a batch of 4 segments.
        virtual void            traceSegments( Segment segments[4] ) = 0;

        //! Adds a new mesh instance to scene.
        virtual void            addMesh( const Mesh* mesh ) = 0;

        //! Begins a scene construction.
        virtual void            begin( void ) = 0;

        //! Ends a scene construction.
        virtual void            end( void ) = 0;
    };

} // namespace rt

} // namespace relight

#endif  /*  !defined( __Relight_RT_Tracer_H__ )  */
