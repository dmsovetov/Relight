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

#include "Mesh.h"
#include "MeshLoader.h"

namespace relight {

// ---------------------------------------------- Mesh ---------------------------------------------- //

// ** Mesh::Mesh
Mesh::Mesh( void ) : m_lightmap( NULL ), m_photonmap( NULL )
{

}

// ** Mesh::bounds
const Bounds& Mesh::bounds( void ) const
{
    return m_bounds;
}

// ** Mesh::vertexBuffer
const Vertex* Mesh::vertexBuffer( void ) const
{
    return &m_vertices[0];
}

// ** Mesh::indexBuffer
const Index* Mesh::indexBuffer( void ) const
{
    return &m_indices[0];
}

// ** Mesh::indexCount
int Mesh::indexCount( void ) const
{
    return ( int )m_indices.size();
}

// ** Mesh::index
Index Mesh::index( int index ) const
{
    assert( index >= 0 && index < indexCount() );
    return m_indices[index];
}

// ** Mesh::vertexCount
int Mesh::vertexCount( void ) const
{
    return ( int )m_vertices.size();
}

// ** Mesh::vertex
const Vertex& Mesh::vertex( int index ) const
{
    assert( index >= 0 && index < vertexCount() );
    return m_vertices[index];
}

// ** Mesh::faceCount
int Mesh::faceCount( void ) const
{
    return ( int )m_faces.size();
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

// ** Mesh::addFaces
void Mesh::addFaces( const VertexBuffer& vertices, const IndexBuffer& indices, int materialId )
{
    // ** Push indices
    for( int i = 0, n = ( int )indices.size(); i < n; i++ ) {
        m_indices.push_back( indices[i] + m_vertices.size() );
    }

    // ** Append vertices
    m_vertices.insert( m_vertices.end(), vertices.begin(), vertices.end() );

    // ** Update face array
    m_faces.clear();

    for( int i = 0, n = m_indices.size() / 3; i < n; i++ ) {
        Index* indices = &m_indices[i * 3];
        m_faces.push_back( Face( i, &m_vertices[indices[0]], &m_vertices[indices[1]], &m_vertices[indices[2]] ) );
    }

    // ** Update mesh bounds
    for( int i = 0, n = ( int )vertices.size(); i < n; i++ ) {
        m_bounds += vertices[i].m_position;
    }
}

// ** Mesh::face
const Face& Mesh::face( int index ) const
{
    assert( index >= 0 && index < faceCount() );
    assert( m_faces[index].faceIdx() == index );
    return m_faces[index];
}

// ** Mesh::lightmap
Lightmap* Mesh::lightmap( void ) const
{
    return m_lightmap;
}

// ** Mesh::setLightmap
void Mesh::setLightmap( Lightmap* value )
{
    m_lightmap = value;
}

// ** Mesh::photonmap
Photonmap* Mesh::photonmap( void ) const
{
    return m_photonmap;
}

// ** Mesh::setPhotonmap
void Mesh::setPhotonmap( Photonmap* value )
{
    m_photonmap = value;
}

// ** Mesh::transformed
Mesh* Mesh::transformed( const Matrix4& transform ) const
{
    Mesh* mesh          = new Mesh;
    mesh->m_vertices    = m_vertices;
    mesh->m_indices     = m_indices;
    mesh->m_faces       = m_faces;

    for( int i = 0, n = mesh->vertexCount(); i < n; i++ ) {
        mesh->m_vertices[i].m_position = transform * mesh->m_vertices[i].m_position;
    }

    return mesh;
}

// ----------------------------------------------- Vertex ----------------------------------------------- //

// ** Vertex::interpolate
Vertex Vertex::interpolate( const Vertex& a, const Vertex& b, float scalar )
{
    Vertex result;

    result.m_position = a.m_position * scalar + b.m_position * (1.0f - scalar);
    result.m_color    = a.m_color    * scalar + b.m_color    * (1.0f - scalar);

    for( int i = 0; i < TotalUvLayers; i++ ) {
        result.m_uv[i] = a.m_uv[i] * scalar + b.m_uv[i] * (1.0f - scalar);
    }

    result.m_normal = a.m_normal * scalar + b.m_normal * (1.0f - scalar);
    result.m_normal.normalize();

    return result;
}

// ---------------------------------------------- Triangle ---------------------------------------------- //

// ** Triangle::Triangle
Triangle::Triangle( const Face& face ) : m_a( *face.vertex( 0 ) ), m_b( *face.vertex( 1 ) ), m_c( *face.vertex( 2 ) )
{
    m_centroid.m_position = (m_a.m_position + m_b.m_position + m_c.m_position)  / 3.0f;
    m_centroid.m_color    = (m_a.m_color    + m_b.m_color    + m_c.m_color)     / 3.0f;
    m_centroid.m_normal   = (m_a.m_normal   + m_b.m_normal   + m_c.m_normal)    / 3.0f;
    m_centroid.m_normal.normalize();
}

// ** Triangle::Triangle
Triangle::Triangle( const Vertex& a, const Vertex& b, const Vertex& c ) : m_a( a ), m_b( b ), m_c( c )
{
    m_centroid.m_position = (m_a.m_position + m_b.m_position + m_c.m_position)  / 3.0f;
    m_centroid.m_color    = (m_a.m_color    + m_b.m_color    + m_c.m_color)     / 3.0f;
    m_centroid.m_normal   = (m_a.m_normal   + m_b.m_normal   + m_c.m_normal)    / 3.0f;
    m_centroid.m_normal.normalize();
}

// ** Triangle::centroid
const Vertex& Triangle::centroid( void ) const
{
    return m_centroid;
}

// ** Triangle::tesselate
void Triangle::tesselate( Triangle& center, Triangle triangles[3] ) const
{
    Vertex xA = Vertex::interpolate( m_a, m_b, 0.5f );
    Vertex xB = Vertex::interpolate( m_b, m_c, 0.5f );
    Vertex xC = Vertex::interpolate( m_c, m_a, 0.5f );

    triangles[0] = Triangle( m_a, xA,  xC  );
    triangles[1] = Triangle( xA,  m_b, xB  );
    triangles[2] = Triangle( xC,  xB,  m_c );
    center       = Triangle( xA,  xB,  xC  );
}

// ------------------------------------------------ Face ------------------------------------------------ //

// ** Face::Face
Face::Face( Index faceIdx, const Vertex* a, const Vertex* b, const Vertex* c ) : m_faceIdx( faceIdx ), m_a( a ), m_b( b ), m_c( c )
{
}

// ** Face::faceIdx
Index Face::faceIdx( void ) const
{
    return m_faceIdx;
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

} // namespace relight