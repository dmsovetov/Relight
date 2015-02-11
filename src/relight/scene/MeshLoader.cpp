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

#include "../BuildCheck.h"

#include "MeshLoader.h"
#include "Mesh.h"

#include <fstream>
#include <map>

namespace relight {

// ** MeshLoader::MeshLoader
MeshLoader::MeshLoader( void ) : m_flags( 0 ), m_totalSubmeshFaces( 0 ), m_totalFaces( 0 )
{

}

// ** MeshLoader::commit
void MeshLoader::commit( void )
{
    if( m_totalSubmeshFaces == 0 ) {
        return;
    }

    IndexBuffer     indices;
    VertexBuffer    vertices;
    Indexer         indexer;
    Index           uiIndex = 0;

    for( int i = m_totalFaces - m_totalSubmeshFaces; i < m_totalFaces; i++ ) {
        int idxXyz[]	= { m_xyzIndices[i * 3 + 0],    m_xyzIndices[i * 3 + 1],	m_xyzIndices[i * 3 + 2]     };
        int idxUv[]		= { m_uvIndices[i * 3 + 0],     m_uvIndices[i * 3 + 1],     m_uvIndices[i * 3 + 2]      };
        int idxNorm[]	= { m_normIndices[i * 3 + 0],	m_normIndices[i * 3 + 1],	m_normIndices[i * 3 + 2]	};

        // ** XYZ
        for( int j = 0; j < 3; j++ ) {
            int xyzIndex = idxXyz[j];
            int nrmIndex = idxNorm[j];
            int uvIndex  = idxUv[j];

            Vertex v;

            v.position[0]	= m_vertices[ xyzIndex * 3 + 0 ];
            v.position[1]	= m_vertices[ xyzIndex * 3 + 1 ];
            v.position[2]	= m_vertices[ xyzIndex * 3 + 2 ];

            v.normal[0]		= (m_flags & HasNormals) ? m_normals[ nrmIndex * 3 + 0 ] : 0.0f;
            v.normal[1]		= (m_flags & HasNormals) ? m_normals[ nrmIndex * 3 + 1 ] : 1.0f;
            v.normal[2]		= (m_flags & HasNormals) ? m_normals[ nrmIndex * 3 + 2 ] : 0.0f;

            v.color[0] = 1.0f;
            v.color[1] = 1.0f;
            v.color[2] = 1.0f;

            if( m_flags & HasUv ) {
                v.uv[0]  = m_uv[ uvIndex * 2 + 0 ];
                v.uv[1]  = m_uv[ uvIndex * 2 + 1 ];

                if( v.uv[0] > 1.0f ) v.uv[0] = 1.0f;
                if( v.uv[1] > 1.0f ) v.uv[1] = 1.0f;
                if( v.uv[0] < 0.0f ) v.uv[0] = 0.0f;
                if( v.uv[1] < 0.0f ) v.uv[1] = 0.0f;
            }

            Indexer::iterator vi = indexer.find( v );
            Index index = -1;

            if( vi == indexer.end() ) {
                index	   = uiIndex++;
                indexer[v] = index;
            } else {
                index = vi->second;
            }
            
            indices.push_back( index );
        }
    }
    
    // ** Copy vertices
    vertices.resize( indexer.size() );
    for( Indexer::const_iterator i = indexer.begin(); i != indexer.end(); ++i ) {
        const Vertex& v = i->first;

        vertices[i->second].m_position = Vec3( v.position[0], v.position[1], v.position[2] );
        vertices[i->second].m_normal   = Vec3( v.normal[0], v.normal[1], v.normal[2] );
        vertices[i->second].m_color    = Color( v.color[0], v.color[1], v.color[2] );
        vertices[i->second].m_uv[::relight::Vertex::Lightmap] = Uv( v.uv[0], v.uv[1] );
    }

    m_mesh->addFaces( vertices, indices, 0 );

    m_totalSubmeshFaces = 0;
    m_flags = 0;
}

// ** MeshLoader::parseVertex
void MeshLoader::parseVertex( const String& line )
{
    float x, y, z;
    sscanf( line.c_str(), "%f %f %f ", &x, &y, &z );
    m_vertices.push_back( x ), m_vertices.push_back( y ), m_vertices.push_back( z );
}

// ** MeshLoader::parseNormal
void MeshLoader::parseNormal( const String& line )
{
    float x, y, z;

    sscanf( line.c_str(), "%f %f %f ", &x, &y, &z );
    m_normals.push_back( x ), m_normals.push_back( y ), m_normals.push_back( z );

    m_flags = m_flags | HasNormals;
}

// ** MeshLoader::parseUv
void MeshLoader::parseUv( const String& line )
{
    float x, y;

    sscanf( line.c_str(), "%f %f ", &x, &y );
    m_uv.push_back( x ), m_uv.push_back( y );

    m_flags = m_flags | HasUv;
}

// ** MeshLoader::parseFace
void MeshLoader::parseFace( const String& line )
{
    int xyz[4] = { 0, 0, 0 };
    int uv[4] = { 0, 0, 0 };
    int normal[4] = { 0, 0, 0 };

    if( m_flags & (HasNormals | HasUv) ) {
        sscanf( line.c_str() + 1, "%i/%i/%i %i/%i/%i %i/%i/%i",	&xyz[0], &uv[0], &normal[0], &xyz[1], &uv[1], &normal[1], &xyz[2], &uv[2], &normal[2] );
    }
    else if( m_flags & HasNormals ) {
        sscanf( line.c_str() + 1, "%i/%i %i/%i %i/%i",	&xyz[0], &normal[0], &xyz[1], &normal[1], &xyz[2], &normal[2] );
    }
    else if( m_flags & HasUv ) {
        sscanf( line.c_str() + 1, "%i/%i %i/%i %i/%i",	&xyz[0], &uv[0], &xyz[1], &uv[1], &xyz[2], &uv[2] );
    }
    else {
        sscanf( line.c_str() + 1, "%i %i %i", &xyz[0], &xyz[1], &xyz[2] );
    }

    for( int i = 0; i < 3; i++ ) {
        m_uvIndices.push_back( uv[i] - 1 );
        m_xyzIndices.push_back( xyz[i] - 1 );
        m_normIndices.push_back( normal[i] - 1 );
    }

    m_totalFaces++;
    m_totalSubmeshFaces++;
}

// ** MeshLoader::load
bool MeshLoader::load( Mesh* mesh, const String& fileName )
{
    m_mesh  = mesh;

    std::ifstream file( fileName.c_str() );

    if( !file.is_open() ) {
        return false;
    }

    String line;

    while ( !file.eof() ) {
        getline( file, line );

        char c1 = line.c_str()[0];
        char c2 = line.c_str()[1];

        if( c1 == 'v' && c2 == 't' )
        {
            commit();
            parseUv( line.c_str() + 2 );
        }
        else if(c1 == 'v' && c2 == 'n' )
        {
            commit();
            parseNormal( line.c_str() + 2 );
        }
        else if (c1 == 'v')
        {
            commit();
            parseVertex( line.c_str() + 1 );
        }

        if( c1 == 'f' )
        {
            parseFace( line.c_str() + 1 );
        }
    }
    
    file.close();

    commit();

    return true;
}

} // namespace relight