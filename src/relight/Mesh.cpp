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

#include "BuildCheck.h"

#include "Mesh.h"
#include "MeshLoader.h"

namespace relight {

// ** Mesh::Mesh
Mesh::Mesh( void )
{

}

// ** Mesh::vertexCount
int Mesh::vertexCount( void ) const
{
    int result = 0;

    for( int i = 0; i < submeshCount(); i++ ) {
        result += submesh( i ).m_vertices.size();
    }

    return result;
}


// ** Mesh::faceCount
int Mesh::faceCount( void ) const
{
    int result = 0;

    for( int i = 0; i < submeshCount(); i++ ) {
        result += submesh( i ).m_totalFaces;
    }

    return result;
}

// ** Mesh::submeshCount
int Mesh::submeshCount( void ) const
{
    return ( int )m_meshes.size();
}

// ** Mesh::submesh
const SubMesh& Mesh::submesh( int index ) const
{
    assert( index >= 0 && index < submeshCount() );
    return m_meshes[index];
}

// ** Mesh::createFromFile
Mesh* Mesh::createFromFile( const String& fileName )
{
    Mesh*       mesh = new Mesh;
    MeshLoader  loader;

    if( !loader.load( mesh, fileName ) ) {
        delete mesh;
        return NULL;
    }

    return mesh;
}

// ** Mesh::addSubmesh
void Mesh::addSubmesh( const VertexBuffer& vertices, const IndexBuffer& indices, int totalFaces )
{
    SubMesh sub;
    sub.m_vertices   = vertices;
    sub.m_indices    = indices;
    sub.m_totalFaces = totalFaces;
    m_meshes.push_back( sub );
}

// ** Mesh::face
Face Mesh::face( int index ) const
{
    assert( index >= 0 && index < faceCount() );

    int faces = 0;

    for( int i = 0; i < submeshCount(); i++ ) {
        const SubMesh& sub = submesh( i );

        if( index >= sub.m_totalFaces + faces ) {
            faces += sub.m_totalFaces;
            continue;
        }

        int   idx       = index - faces;
        Index indices[] = { sub.m_indices[idx * 3 + 0], sub.m_indices[idx * 3 + 1], sub.m_indices[idx * 3 + 2] };

        return Face( &sub.m_vertices[indices[0]], &sub.m_vertices[indices[1]], &sub.m_vertices[indices[2]] );
    }
}

// ---------------------------------------------- Face ---------------------------------------------- //

// ** Face::Face
Face::Face( const Vertex* a, const Vertex* b, const Vertex* c ) : m_a( a ), m_b( b ), m_c( c )
{
}

// ** Face::uvRect
void Face::uvRect( Uv& min, Uv& max ) const
{
    min.u = std::min( m_a->m_uv[Vertex::Lightmap].u, std::min( m_b->m_uv[Vertex::Lightmap].u, m_c->m_uv[Vertex::Lightmap].u ) );
    max.u = std::max( m_a->m_uv[Vertex::Lightmap].u, std::max( m_b->m_uv[Vertex::Lightmap].u, m_c->m_uv[Vertex::Lightmap].u ) );
    min.v = std::min( m_a->m_uv[Vertex::Lightmap].v, std::min( m_b->m_uv[Vertex::Lightmap].v, m_c->m_uv[Vertex::Lightmap].v ) );
    max.v = std::max( m_a->m_uv[Vertex::Lightmap].v, std::max( m_b->m_uv[Vertex::Lightmap].v, m_c->m_uv[Vertex::Lightmap].v ) );
}

// ** Face::isUvInside
bool Face::isUvInside( const Uv& uv, Barycentric& barycentric, Vertex::UvLayer layer ) const
{
    // ** Compute vectors
    Uv v0 = m_c->m_uv[layer] - m_a->m_uv[layer];
    Uv v1 = m_b->m_uv[layer] - m_a->m_uv[layer];
    Uv v2 = uv               - m_a->m_uv[layer];

    // ** Compute dot products
    float dot00 = v0 * v0;
    float dot01 = v0 * v1;
    float dot02 = v0 * v2;
    float dot11 = v1 * v1;
    float dot12 = v1 * v2;

    // ** Compute barycentric coordinates
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float _u		= (dot11 * dot02 - dot01 * dot12) * invDenom;
    float _v		= (dot00 * dot12 - dot01 * dot02) * invDenom;

    barycentric = Uv( _u, _v );

    // ** Check if point is in triangle
    return (_u >= 0.0f) && (_v >= 0.0f) && (_u + _v < 1.0f);
}

// ** Face::vertex
const Vertex* Face::vertex( int index ) const
{
    switch( index ) {
    case 0: return m_a;
    case 1: return m_c;
    case 2: return m_b;
    default: break;
    }

    return NULL;
}

// ** Face::normalAt
Vec3 Face::normalAt( const Barycentric& uv ) const
{
    const Vec3& a = m_a->m_normal;
    const Vec3& b = m_b->m_normal;
    const Vec3& c = m_c->m_normal;

    return Vec3( a.x + (b.x - a.x) * uv.v + (c.x - a.x) * uv.u, a.y + (b.y - a.y) * uv.v + (c.y - a.y) * uv.u, a.z + (b.z - a.z) * uv.v + (c.z - a.z) * uv.u );
}

// ** Face::positionAt
Vec3 Face::positionAt( const Barycentric& uv ) const
{
    const Vec3& a = m_a->m_position;
    const Vec3& b = m_b->m_position;
    const Vec3& c = m_c->m_position;

    return Vec3( a.x + (b.x - a.x) * uv.v + (c.x - a.x) * uv.u, a.y + (b.y - a.y) * uv.v + (c.y - a.y) * uv.u, a.z + (b.z - a.z) * uv.v + (c.z - a.z) * uv.u );
}

// ** Face::colorAt
Color Face::colorAt( const Barycentric& uv ) const
{
    const Color& a = m_a->m_color;
    const Color& b = m_b->m_color;
    const Color& c = m_c->m_color;

    return Color( a.r + (b.r - a.r) * uv.v + (c.r - a.r) * uv.u, a.g + (b.g - a.g) * uv.v + (c.g - a.g) * uv.u, a.b + (b.b - a.b) * uv.v + (c.b - a.b) * uv.u );
}

// ** Face::uvAt
Uv Face::uvAt( const Barycentric& uv, Vertex::UvLayer layer ) const
{
    const Uv& a = m_a->m_uv[layer];
    const Uv& b = m_b->m_uv[layer];
    const Uv& c = m_c->m_uv[layer];

    return Uv( a.u + (b.u - a.u) * uv.v + (c.u - a.u) * uv.u, a.v + (b.v - a.v) * uv.v + (c.v - a.v) * uv.u );
}

// -------------------------------------------- Instance -------------------------------------------- //

// ** Instance::Instance
Instance::Instance( const Mesh* mesh, const Matrix4& transform ) : m_mesh( mesh ), m_transform( transform ), m_lightmap( NULL ), m_photonmap( NULL )
{

}

// ** Instance::mesh
const Mesh* Instance::mesh( void ) const
{
    return m_mesh;
}

// ** Instance::transform
const Matrix4& Instance::transform( void ) const
{
    return m_transform;
}

// ** Instance::lightmap
Lightmap* Instance::lightmap( void ) const
{
    return m_lightmap;
}

// ** Instance::setLightmap
void Instance::setLightmap( Lightmap* value )
{
    m_lightmap = value;
}

// ** Instance::photonmap
Photonmap* Instance::photonmap( void ) const
{
    return m_photonmap;
}

// ** Instance::setPhotonmap
void Instance::setPhotonmap( Photonmap* value )
{
    m_photonmap = value;
}

// ** Instance::create
Instance* Instance::create( const Mesh* mesh, const Matrix4& transform )
{
    return new Instance( mesh, transform );
}

} // namespace relight