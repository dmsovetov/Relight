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

#ifndef __Relight_Mesh_H__
#define __Relight_Mesh_H__

#include "Relight.h"
#include "Vector.h"

namespace relight {

    /*!
     A single Mesh vertex.
     */
    struct Vertex {
        //! UV coordinates layer
        enum UvLayer {
            Lightmap,
            Diffuse,
            TotalUvLayers
        };

        Vec3    m_position;
        Vec3    m_normal;
        Color   m_color;
        Uv      m_uv[TotalUvLayers];
    };

    /*!
     A single Mesh face.
     */
    class Face {
    public:

                        //! Constructs a new Face instance.
                        Face( const Vertex* a, const Vertex* b, const Vertex* c );

        //! Returns true if a given UV coordinates are inside the face triangle.
        bool            isUvInside( const Uv& uv, Barycentric& barycentric, Vertex::UvLayer layer ) const;

        //! Returns a UV rectangle for face.
        void            uvRect( Uv& min, Uv& max ) const;

        //! Returns a vertex by index.
        const Vertex*   vertex( int index ) const;

        //! Returns an interpolated face normal.
        Vec3            normalAt( const Barycentric& uv ) const;

        //! Returns an interpolated face position.
        Vec3            positionAt( const Barycentric& uv ) const;

        //! Returns an interpolated face color.
        Color           colorAt( const Barycentric& uv ) const;

        //! Returns an interpolated face UV.
        Uv              uvAt( const Barycentric& uv, Vertex::UvLayer layer ) const;

    private:

        //! Face vertices.
        const Vertex*   m_a;
        const Vertex*   m_b;
        const Vertex*   m_c;
    };

    //! A submesh data structure.
    struct SubMesh {
        int             m_totalFaces;   //!< Total submesh faces.
        VertexBuffer    m_vertices;     //!< Submesh vertex buffer.
        IndexBuffer     m_indices;      //!< Submesh index buffer.
    };

    /*!
     In the Relight all meshes are specified by an indexed triangle lists. Note, that a Mesh
     is just a triangle data and not an instance. To actually place it in the world you have
     to create an Instance, which is a Mesh + Transform + instance specific rendering settings.
     */
    class Mesh {
    public:

        //! Returns a total number of vertices.
        int                 vertexCount( void ) const;

        //! Returns a total number of faces.
        int                 faceCount( void ) const;

        //! Returns a face by index.
        Face                face( int index ) const;

        //! Returns a total number of submeshes.
        int                 submeshCount( void ) const;

        //! Returns a submesh by index.
        const SubMesh&      submesh( int index ) const;

        //! Adds a submesh.
        void                addSubmesh( const VertexBuffer& vertices, const IndexBuffer& indices, int totalFaces );

        /*!
         Creates a mesh data from a file. Only OBJ file format is supported.
         */
        static Mesh*        createFromFile( const String& fileName );

    private:

                            //! Constructs a new mesh.
                            Mesh( void );

    private:

        //! Mesh data.
        Array<SubMesh>      m_meshes;
    };

    /*!
     Instance class represents a mesh instance on the scene.
     */
    class Instance {
    friend class Lightmap;
    friend class Photonmap;
    public:

        //! Returns a mesh data for this instance.
        const Mesh*         mesh( void ) const;

        //! Returns an instance affine transform.
        const Matrix4&      transform( void ) const;

        //! Returns a target lightmap.
        Lightmap*           lightmap( void ) const;

        //! Returns a target photonmap.
        Photonmap*          photonmap( void ) const;

        /*!
         Creates a new mesh Instance.
         \param mesh A mesh data instance.
         \param transform Instance affine transform.
         \return New mesh Instance.
         */
        static Instance*    create( const Mesh* mesh, const Matrix4& transform );

    private:

                            //! Constructs a new mesh instance.
                            Instance( const Mesh* mesh, const Matrix4& transform );

        //! Sets a target lightmap.
        void                setLightmap( Lightmap* value );

        //! Sets a target photon map.
        void                setPhotonmap( Photonmap* value );

    private:

        //! A weak Mesh instance pointer.
        const Mesh*         m_mesh;

        //! Instance affine transform.
        Matrix4             m_transform;

        //! Target lightmap.
        Lightmap*           m_lightmap;

        //! Target photonmap.
        Photonmap*          m_photonmap;
    };

} // namespace relight

#endif  /*  !defined( __Relight_Mesh_H__ ) */
