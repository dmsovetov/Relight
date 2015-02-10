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

#ifndef __Relight_H__
#define __Relight_H__

#include <vector>
#include <string>
#include <map>
#include <chrono>
#include <assert.h>
#include <float.h>

#define lmTYPE float
#define sVector2 relight::Uv
#define sVector3 relight::Vec3

namespace relight {

    template<typename T>
    class Array : public std::vector<T> {};

    typedef std::string String;

    class Scene;
    class Light;
        class PointLight;
    struct SubMesh;
    class Face;
    class Mesh;
    class Lightmap;
        class Photonmap;
    struct Lumel;

    class Matrix4;
    class Vec3;
    class Color;
    class Uv;

    //! Mesh vertex index.
    typedef unsigned short Index;

    //! Face material.
    typedef unsigned int Material;

    //! Mesh vertex buffer.
    typedef Array<struct Vertex>    VertexBuffer;

    //! Mesh vertex buffer.
    typedef Array<Index>            IndexBuffer;

    namespace rt {
        class ITracer;
    }

    //! Relight status codes.
    enum RelightStatus {
        RelightSuccess,         //!< Everything is OK.
        RelightInvalidCall,     //!< Invalid call.
        RelightNotImplemented   //!< The method is not implemented.
    };

    //! Baker progress callback.
    class Progress {
    public:

        //! Notifies about a task progress.
        virtual void            notify( int step, int stepCount ) {}
    };

    // ** TimeMeasure
    struct TimeMeasure {
        TimeMeasure( const char* label ) : m_label( label ), m_time( std::chrono::steady_clock::now() ) {}
        ~TimeMeasure( void ) {
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            printf( "%s %2.2f seconds\n", m_label, std::chrono::duration_cast<std::chrono::milliseconds>( end - m_time ).count() * 0.001f );
        }

        const char* m_label;
        std::chrono::steady_clock::time_point m_time;
    };

} // namespace relight

#ifndef RELIGHT_BUILD_LIBRARY
    #include "Scene.h"
    #include "Mesh.h"
    #include "Light.h"
#endif

#endif  /*  !defined( Relight ) */
