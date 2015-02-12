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

#ifndef __relight__MeshLoader__
#define __relight__MeshLoader__

#include "../Relight.h"

namespace relight {

    //! A mesh loader used to parse mesh data from file.
    class MeshLoader {
    public:

                        MeshLoader( void );

        //! Loads a Mesh from OBJ file format.
        /*!
         \param mesh Target mesh.
         \param fileName Source file name.
         \return True if file is loaded, otherwise false.
         */
        bool            load( Mesh* mesh, const String& fileName );

    private:

        //! Commits a parsed submesh.
        void            commit( void );

        //! Parses a vertex from line.
        void            parseVertex( const String& line );

        //! Parses a uv from line.
        void            parseUv( const String& line );

        //! Parses a normal from line.
        void            parseNormal( const String& line );

        //! Parses a face from line
        void            parseFace( const String& line );

    private:

        //! Mesh data mask.
        enum MeshDataFlags {
            HasVertices = 0x1,
            HasUv       = 0x2,
            HasNormals  = 0x4
        };

        //! Mesh loader vertex.
        struct Vertex {
            enum { ElementCount = 11 };

            union {
                struct
                {
                    float		position[3];
                    float		normal[3];
                    float		color[3];
                    float		uv[2];
                };
                float v[ElementCount];
            };

            int		lightmapIndex;

            // ** sMeshVertex
            bool operator < ( const Vertex& other ) const {
                for( int i = 0; i < ElementCount; ++i ) {
                    if( v[i] != other.v[i] ) {
                        return v[i] < other.v[i]; 
                    }
                }
                
                return false;
            }
        };

        //! A helper type to reindex a mesh.
        typedef std::map<Vertex, int> Indexer;

        //! Current mesh.
        Mesh*           m_mesh;

        //! Mesh data flags.
        int             m_flags;

        //! Current submesh faces.
        int             m_totalSubmeshFaces;

        //! Total faces.
        int             m_totalFaces;

        //! Parsed mesh data.
        Array<float>	m_normals, m_uv, m_vertices;

        //! Parsed indices.
        Array<int>		m_xyzIndices, m_uvIndices, m_normIndices;
    };

} // namespace relight

#endif /* defined(__relight__MeshLoader__) */
