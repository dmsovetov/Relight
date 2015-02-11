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

#ifndef __Relight_Light_H__
#define __Relight_Light_H__

#include "Relight.h"
#include "Vector.h"

namespace relight {

    /*!
     A supported light type enumeration.
     */
    enum LightType {
        PointLightType,
        MeshLightType,

        TotalLightTypes
    };

    /*!
     Light distance attenuation model.
     */
    class LightAttenuation {
    public:

                            //! Constructs a LightAttenuation instance.
                            LightAttenuation( const Light* light );
        virtual             ~LightAttenuation( void ) {}

        //! Calculates light attenuation by distance.
        /*!
         An attenuation factor is a value in range [0;1], where 0 means
         that light is attenuated to zero, and 1 means no attenuation at all.
         */
        virtual float       calculate( float distance ) const { return 0.0f; }

    protected:

        //! Parent light source.
        const Light*        m_light;
    };

    /*!
     Linear light attenuation model.
     */
    class LinearLightAttenuation : public LightAttenuation {
    public:

                            //! Constructs a LinearLightAttenuation instance.
                            LinearLightAttenuation( const Light* light );

        // ** LinearLightAttenuation
        virtual float       calculate( float distance ) const;
    };

    /*!
     Photon emitter is used to determine an amount of photons to be emitted by a given light.
     */
    class PhotonEmitter {
    public:

                            //! Constructs a new PhotonEmitter instance.
                            PhotonEmitter( const Light* light );
        virtual             ~PhotonEmitter( void ) {}

        //! Calculates an amount of photons to be emitted.
        virtual int         photonCount( void ) const;

        //! Emits a new photon.
        virtual Vec3        emit( void ) const;

    private:

        //! Parent light source.
        const Light*        m_light;
    };

    /*!
     A base class for all light types in Relight.
     */
    class Light {
    public:

        virtual             ~Light( void );

        //! Returns a light type.
        virtual LightType   type( void ) const;

        //! Returns an attenuation model.
        LightAttenuation*   attenuation( void ) const;

        //! Sets an attenuation model.
        void                setAttenuation( LightAttenuation* value );

        //! Returns a photon emitter.
        PhotonEmitter*      photonEmitter( void ) const;

        //! Sets a photon emitter.
        /*!
         By default each light has a photon emitter, light sources with no photon emitter
         set won't make any influence to a global illumination.
         */
        void                setPhotonEmitter( PhotonEmitter* value );

        //! Returns a light position.
        const Vec3&         position( void ) const;

        //! Sets a light position.
        void                setPosition( const Vec3& value );

        //! Returns a light color.
        const Color&        color( void ) const;

        //! Sets a light color.
        void                setColor( const Color& value );

        //! Returns a light intensity.
        float               intensity( void ) const;

        //! Sets a light intensity.
        void                setIntensity( float value );

        //! Returns true if this light casts shadow otherwise false.
        bool                castsShadow( void ) const;

        //! Sets a castShadow flag.
        void                setCastsShadow( bool value );

    protected:

                            //! Constructs a Light instance.
                            Light( LightType type = TotalLightTypes );

    private:

        //! Light type.
        LightType           m_type;

        //! Light position.
        Vec3                m_position;

        //! Light color.
        Color               m_color;

        //! Light intensity.
        float               m_intensity;

        //! Casts shadow flag.
        bool                m_castsShadow;

        //! Light attenuation model.
        LightAttenuation*   m_attenuation;

        //! Light source photon emitter.
        PhotonEmitter*      m_photonEmitter;
    };

    /*!
     A point light (omni-directional).
     */
    class PointLight : public Light {
    public:

        //! Returns a light radius.
        float               radius( void ) const;

        //! Sets a light radius.
        void                setRadius( float value );

        //! Creates a new PointLight instance.
        static PointLight*  create( const Vec3& position, float radius, const Color& color = Color( 1.0f, 1.0f, 1.0f ), float intensity = 1.0f, bool castsShadow = true );

    private:

                            //! Constructs a new PointLight instance.
                            PointLight( void );

    private:

        //! A light radius.
        float               m_radius;
    };

    //! A light vertex is a single light point used with mesh light sources.
    struct LightVertex {
        Vec3                m_position; //!< Point position.
        Vec3                m_normal;   //!< Point normal.
    };

    typedef Array<LightVertex> LightVertexBuffer;

    /*!
     A LightVertexGenerator used to generate a set of light points for mesh light sources.
     */
    class LightVertexGenerator {
    public:

                                    //! Constructs a LightVertexGenerator instance.
                                    LightVertexGenerator( const Mesh* mesh );
        virtual                     ~LightVertexGenerator( void ) {}

        //! Returns an array of light vertices.
        const LightVertexBuffer&    vertices( void ) const;

        //! Generates a set of light vertices based on mesh vertices.
        virtual void                generate( void );

        //! Clears a previously generated data.
        void                        clear( void );

        //! Returns a total number of light vertices.
        int                         vertexCount( void ) const;

    protected:

        //! Adds a new light vertex.
        void                        push( const Vertex& vertex );

    protected:

        //! Source mesh.
        const Mesh*                 m_mesh;

        //! A set of generated light vertices.
        LightVertexBuffer           m_vertices;
    };

    /*!
     A FaceLightVertexGenerator generates a set of vertices based on mesh vertices & face centroids.
     Face based generator can also perform a mesh tesselation.
     */
    class FaceLightVertexGenerator : public LightVertexGenerator {
    public:

                                    //! Constructs a FaceLightVertexGenerator instance.
                                    /*!
                                     \param mesh Source light mesh.
                                     \param excludeVertices The flag indicating that we should skip mesh vertices.
                                     \param maxSubdivisions Maximum subdivision steps per mesh face (0 means no tesselation).
                                     */
                                    FaceLightVertexGenerator( const Mesh* mesh, bool excludeVertices, int maxSubdivisions );

        //! Generates a set of light vertices.
        virtual void                generate( void );

    private:

        //! Generates a set of light vertices from triangle.
        virtual void                generateFromTriangle( const Triangle& triangle, int subdivision );

    private:

        //! The flag indicating that we should skip mesh vertices.
        bool                        m_excludeVertices;

        //! Max subdivision depth.
        int                         m_maxSubdivisions;
    };

    /*!
     A mesh area light.
     */
    class MeshLight : public Light {
    public:

                                ~MeshLight( void );

        //! Returns a mesh used as a light source.
        const Mesh*             mesh( void ) const;

        //! Returns true if a mesh light takes into account only upper-hemisphere rays.
        bool                    isHemisphere( void ) const;

        //! Sets heimisphere flag.
        void                    setHemisphere( bool value );

        //! Sets a light vertex generator.
        void                    setVertexGenerator( LightVertexGenerator* value );

        //! Returns a light vertex generator.
        LightVertexGenerator*   vertexGenerator( void ) const;

        //! Creates a new MeshLight instance.
        static MeshLight*       create( const Mesh* mesh, const Vec3& position, const Color& color = Color( 1.0f, 1.0f, 1.0f ), float intensity = 1.0f, bool castsShadow = true, bool hemisphere = false );

    private:

                                //! Constructs a new MeshLight instance.
                                MeshLight( const Mesh* mesh );

    private:

        //! A mesh used as a light source volume.
        const Mesh*             m_mesh;

        //! Hemisphere lighting.
        bool                    m_isHemisphere;

        //! A light vertex generator.
        LightVertexGenerator*   m_vertexGenerator;
    };

} // namespace relight

#endif  /*  !defined( __Relight_Light_H__ ) */
