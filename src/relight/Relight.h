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
#define sColor relight::Color

namespace relight {

    template<typename T>
    class Array : public std::vector<T> {};

    typedef std::string String;

    class Scene;
    class LightAttenuation;
    class LightCutoff;
    class LightInfluence;
    class LightVertexGenerator;
    class Light;
        class PointLight;
        class MeshLight;
    struct SubMesh;
    class Face;
    class Triangle;
    class Mesh;
    class Material;
    class Texture;
    class Lightmap;
        class Photonmap;
    struct Lumel;

    class Matrix4;
    class Vec3;
    class Color;
    class Bounds;
    class Uv;

    //! Mesh vertex index.
    typedef unsigned short Index;

    //! Mesh vertex buffer.
    typedef Array<struct Vertex>    VertexBuffer;

    //! Mesh vertex buffer.
    typedef Array<Index>            IndexBuffer;

    //! Texture pixel buffer.
    typedef Array<Color>            PixelBuffer;

    namespace rt {
        class ITracer;
    }

    namespace bake {
        class BakeIterator;
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

    //! Indirect light settings.
    struct IndirectLightSettings {
        int                             m_photonPassCount;          //!< Number of photon passes.
        int                             m_photonBounceCount;        //!< Maximum photon tracing depth (number of light bounces).
        float                           m_photonEnergyThreshold;    //!< The minimum energy that photon should have to continue tracing.
        float                           m_photonMaxDistance;        //!< The reflected light maximum distance. All intersections above this value will be ignored.

        int                             m_finalGatherSamples;       //!< Number of final gather samples.
        float                           m_finalGatherDistance;      //!< Maximum distance to gather photons at.
        int                             m_finalGatherRadius;        //!< A radius of circle in which samples are gathered from photon map.

        //! Returns a fast quality settings.
        static IndirectLightSettings    fast( float photonMaxDistance = 10.0f, float finalGatherDistance = 50.0f );

        //! Returns a draft quality settings.
        static IndirectLightSettings    draft( float photonMaxDistance = 10.0f, float finalGatherDistance = 50.0f );

        //! Returns a best quality settings.
        static IndirectLightSettings    best( float photonMaxDistance = 10.0f, float finalGatherDistance = 50.0f );

        //! Returns a production quality settings.
        static IndirectLightSettings    production( float photonMaxDistance = 10.0f, float finalGatherDistance = 50.0f );
    };

    //! Ambient occlusion settings.
    struct AmbientOcclusionSettings {
        int                             m_samples;          //!< Number of ambient occlusion samples.
        float                           m_occludedFraction; //!< Fraction of samples taken that must be occluded in order to reach full occlusion.
        float                           m_maxDistance;      //!< Maximum distance for an object to cause occlusion on another object.
        float                           m_exponent;         //!< Final occlusion value exponent.

        //! Returns a fast quality settings.
        static AmbientOcclusionSettings fast( float occludedFraction = 0.8f, float maxDistance = 0.6f, float exponent = 1.0f );

        //! Returns a draft quality settings.
        static AmbientOcclusionSettings draft( float occludedFraction = 0.8f, float maxDistance = 0.6f, float exponent = 1.0f );

        //! Returns a best quality settings.
        static AmbientOcclusionSettings best( float occludedFraction = 0.8f, float maxDistance = 0.6f, float exponent = 1.0f );

        //! Returns a production quality settings.
        static AmbientOcclusionSettings production( float occludedFraction = 0.8f, float maxDistance = 0.6f, float exponent = 1.0f );
    };

    //! Relight class.
    class Relight {
    public:

        //! Bakes direct lighting.
        static RelightStatus    bakeDirectLight( const Scene* scene, const Mesh* mesh, Progress* progress, bake::BakeIterator* iterator = NULL );

        //! Bakes indirect light to a lightmap.
        static RelightStatus    bakeIndirectLight( const Scene* scene, const Mesh* mesh, Progress* progress, const IndirectLightSettings& settings, bake::BakeIterator* iterator = NULL );

        //! Bakes ambient occlusion to a lightmap.
        static RelightStatus    bakeAmbientOcclusion( const Scene* scene, const Mesh* mesh, Progress* progress, const AmbientOcclusionSettings& settings, bake::BakeIterator* iterator = NULL );
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
    #include "scene/Scene.h"
    #include "scene/Mesh.h"
    #include "scene/Light.h"
    #include "scene/Material.h"
    #include "baker/Baker.h" 
#endif

#endif  /*  !defined( Relight ) */
