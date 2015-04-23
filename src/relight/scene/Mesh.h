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

#include "../Relight.h"

namespace relight {

    /*!
     A single Mesh vertex.
     */
    struct Vertex {
        //! UV coordinates layer
        enum UvLayer {
            Diffuse,
            Lightmap,
            TotalUvLayers
        };

        Vec3            m_position;
        Vec3            m_normal;
        Uv              m_uv[TotalUvLayers];
        const Material* m_material;

        //! Interpolates between two vertices.
        static Vertex   interpolate( const Vertex& a, const Vertex& b, float scalar );
    };

    /*!
     A vertex buffer helper struct
     */
    struct VertexBufferLayout {
        const void*     m_position;     //!< XYZ float coordinates
        const void*     m_normal;       //!< XYZ float normal
        const void*     m_uv0;          //!< UV float uv.
        const void*     m_uv1;          //!< UV float uv.
        int             m_vertexSize;   //!< Vertex size.

                        //! Constructs a VertexBufferLayout instance.
                        VertexBufferLayout( int vertexSize )
                            : m_position( NULL ), m_normal( NULL ), m_uv0( NULL ), m_uv1( NULL ), m_vertexSize( vertexSize ) {}
    };

    //! A helper class to tesselate faces.
    class Triangle {
    public:

                        //! Constructs a Triangle instance.
                        Triangle( void ) {}

                        //! Constructs a Triangle instance from a given Face.
                        Triangle( const Face& face );

                        //! Constructs a Triangle instance from three vertices.
                        Triangle( const Vertex& a, const Vertex& b, const Vertex& c );

        //! Returns a triangle centroid.
        const Vertex&   centroid( void ) const;

        //! Tesselates a triangle. Splits this triangle into 4 smaller ones.
        /*!
         \param center Output center triangle.
         \param triangles Three triangles on corners.
         */
        void            tesselate( Triangle& center, Triangle triangles[3] ) const;

        //! Calculates a triangle area.
        static float    area( const Vec3& a, const Vec3& b, const Vec3& c );

    private:

        //! Triangle vertices
        Vertex          m_a, m_b, m_c;

        //! Triangle centroid.
        Vertex          m_centroid;
    };

    /*!
     A single Mesh face.
     */
    class Face {
    public:

                        //! Constructs a new Face instance.
                        Face( Index faceIdx, const Vertex* a, const Vertex* b, const Vertex* c );

        //! Returns a face area.
        float           area( void ) const;

        //! Returns a face id.
        Index           faceIdx( void ) const;

        //! Returns true if a given UV coordinates are inside the face triangle.
        bool            isUvInside( const Uv& uv, Barycentric& barycentric, Vertex::UvLayer layer ) const;

        //! Returns a UV rectangle for face.
   //     void            uvRect( Uv& min, Uv& max ) const;
		Rect			uvRect( void ) const;

        //! Returns a vertex by index.
        const Vertex*   vertex( int index ) const;

        //! Returns an interpolated face normal.
        Vec3            normalAt( const Barycentric& uv ) const;

        //! Returns an interpolated face position.
        Vec3            positionAt( const Barycentric& uv ) const;

        //! Returns an interpolated face color.
        Rgba            colorAt( const Barycentric& uv ) const;

        //! Returns an interpolated face UV.
        Uv              uvAt( const Barycentric& uv, Vertex::UvLayer layer ) const;

    private:

        //! Face index.
        Index           m_faceIdx;

        //! Face vertices.
        const Vertex*   m_a;
        const Vertex*   m_b;
        const Vertex*   m_c;
    };

    /*!
     In the Relight all meshes are specified by an indexed triangle lists.
     */
    class Mesh {
    friend class Lightmap;
    friend class Photonmap;
    public:

        //! Returns a mesh bounds.
        const Bounds&       bounds( void ) const;

        //! Returns a vertex buffer pointer.
        const Vertex*       vertexBuffer( void ) const;

        //! Returns an index buffer pointer.
        const Index*        indexBuffer( void ) const;

        //! Returns a total number of vertices.
        int                 vertexCount( void ) const;

        //! Returns a mesh vertex by index.
        const Vertex&       vertex( int index ) const;

        //! Returns a total number of indices.
        int                 indexCount( void ) const;

        //! Returns a vertex index.
        Index               index( int index ) const;

        //! Returns a total number of faces.
        int                 faceCount( void ) const;

        //! Returns a face by index.
        const Face&         face( int index ) const;

		//! Returns the total mesh surface area.
		float				area( void ) const;

        //! Adds a list of faces sharing a same material.
        void                addFaces( const VertexBuffer& vertices, const IndexBuffer& indices, const Material* material = NULL );

        //! Adds a mesh vertices and faces from vertex buffer layout.
        void                addFaces( const VertexBufferLayout& vertexBuffer, int vertexCount, const Index* indexBuffer, int indexCount, const Material* material = NULL );

        //! Returns a target lightmap.
        Lightmap*           lightmap( void ) const;

        //! Returns a target photonmap.
        Photonmap*          photonmap( void ) const;

        //! Transforms a mesh with a affine matrix.
        void                transform( const Matrix4& transform );

        //! Creates a clone of this mesh with applied transform.
        Mesh*               transformed( const Matrix4& transform ) const;

        //! Sets a material for entire mesh.
        void                overrideMaterial( const Material* material );

        //! Returns a user data.
        void*               userData( void ) const;

        //! Sets a user data.
        void                setUserData( void* value );

        /*!
         Creates a mesh data from a file. Only OBJ file format is supported.
         */
        static Mesh*        createFromFile( const String& fileName );

        /*!
         Creates an empty mesh data.
         */
        static Mesh*        create( void );

    private:

                            //! Constructs a new mesh.
                            Mesh( void );

        //! Sets a target lightmap.
        void                setLightmap( Lightmap* value );

        //! Sets a target photon map.
        void                setPhotonmap( Photonmap* value );

        //! Builds a mesh faces array.
        void                buildFaces( void );

    private:

        //! Mesh bounds.
        Bounds              m_bounds;

        //! Mesh vertex buffer.
        VertexBuffer        m_vertices;

        //! Mesh index buffer.
        IndexBuffer         m_indices;

        //! Mesh faces.
        Array<Face>         m_faces;

        //! Target lightmap.
        Lightmap*           m_lightmap;

        //! Target photonmap.
        Photonmap*          m_photonmap;

        //! User data associated with this instance.
        void*               m_userData;
    };

} // namespace relight

#endif  /*  !defined( __Relight_Mesh_H__ ) */
