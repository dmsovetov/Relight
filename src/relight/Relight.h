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

#ifdef WIN32
	#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Types.h"

namespace relight {

	typedef Vec2 Uv;
    typedef Uv	 Barycentric;

    class Scene;
    class Worker;
    class Job;
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
		class Radiancemap;
    struct Lumel;

    //! Mesh vertex index.
    typedef unsigned short Index;

    //! Mesh vertex buffer.
    typedef Array<struct Vertex>    VertexBuffer;

    //! Mesh vertex buffer.
    typedef Array<Index>            IndexBuffer;

    //! Texture pixel buffer.
    typedef Array<Rgb>              RgbPixelBuffer;

    //! Workers array.
    typedef Array<Worker*>          Workers;

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

    //! Lightmap storage file format.
    enum StorageFormat {
        RawHdr,
        TgaDoubleLdr,
        TgaRgbm,
    };

    //! Baker progress callback.
    class Progress {
    public:

                                //! Constructs a Progress instance.
                                Progress( void ) {}
        virtual                 ~Progress( void ) { assert( false ); }

        //! Notifies about a task progress.
        virtual void            notify( const Mesh* instance, int step, int stepCount ) {}
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

        Rgb                             m_skyColor;                 //!< A sky color is used when the ray didn't hit anything.
        Rgb                             m_ambientColor;             //!< Ambient color for any point in scene.

        //! Returns a fast quality settings.
        static IndirectLightSettings    fast( const Rgb& skyColor = Rgb( 0.0f, 0.0f, 0.0f ), const Rgb& ambientColor = Rgb( 0.0f, 0.0f, 0.0f ), float photonMaxDistance = 10.0f, float finalGatherDistance = 50.0f );

        //! Returns a draft quality settings.
        static IndirectLightSettings    draft( const Rgb& skyColor = Rgb( 0.0f, 0.0f, 0.0f ), const Rgb& ambientColor = Rgb( 0.0f, 0.0f, 0.0f ), float photonMaxDistance = 10.0f, float finalGatherDistance = 50.0f );

        //! Returns a best quality settings.
        static IndirectLightSettings    best( const Rgb& skyColor = Rgb( 0.0f, 0.0f, 0.0f ), const Rgb& ambientColor = Rgb( 0.0f, 0.0f, 0.0f ), float photonMaxDistance = 10.0f, float finalGatherDistance = 50.0f );

        //! Returns a production quality settings.
        static IndirectLightSettings    production( const Rgb& skyColor = Rgb( 0.0f, 0.0f, 0.0f ), const Rgb& ambientColor = Rgb( 0.0f, 0.0f, 0.0f ), float photonMaxDistance = 10.0f, float finalGatherDistance = 50.0f );
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

        //! Creates a new lightmap instance.
        Lightmap*               createLightmap( int width, int height ) const;

        //! Creates a new photonmap instance.
        Photonmap*              createPhotonmap( int width, int height ) const;

        //! Creates a new scene.
        Scene*                  createScene( void ) const;

        //! Performs a full scene bake.
        void                    bake( const Scene* scene, Job* job, Worker* root, const Workers& workers );

        //! Bakes direct lighting.
        RelightStatus           bakeDirectLight( const Scene* scene, const Mesh* mesh, Progress* progress, bake::BakeIterator* iterator = NULL );

        //! Bakes indirect light to a lightmap.
        RelightStatus           bakeIndirectLight( const Scene* scene, const Mesh* mesh, Progress* progress, const IndirectLightSettings& settings, bake::BakeIterator* iterator = NULL );

        //! Bakes ambient occlusion to a lightmap.
        RelightStatus           bakeAmbientOcclusion( const Scene* scene, const Mesh* mesh, Progress* progress, const AmbientOcclusionSettings& settings, bake::BakeIterator* iterator = NULL );

        //! Emits photons from all lights to scene.
        RelightStatus           emitPhotons( const Scene* scene, const IndirectLightSettings& settings );

        //! Creates a new relight instance.
        static Relight*         create( void );

    private:

                                //! Constructs relight instance
                                Relight( void );
    };

} // namespace relight

#ifndef RELIGHT_BUILD_LIBRARY
    #include "scene/Scene.h"
    #include "scene/Mesh.h"
    #include "scene/Light.h"
    #include "scene/Material.h"
    #include "baker/Baker.h" 
    #include "Lightmap.h"
    #include "Worker.h"
#endif

#endif  /*  !defined( Relight ) */
