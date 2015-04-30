// Original code by Uri Zapoev
// https://github.com/alkoholick

#ifndef __demo__FbxLoader__
#define __demo__FbxLoader__

#include <stdio.h>
#include <assert.h>
#include <fbxsdk.h>
#include <vector>

#include <math/Mesh.h>

namespace fbx {

struct Vertex {
    FbxVector4  position;
    FbxVector4  normal;
    FbxVector2  uv[2];
};

struct VertexCompare {
    bool operator()( const Vertex& a, const Vertex& b ) const
    {
        for( int i = 0; i < 3; i++ ) {
            if( a.position[i] != b.position[i] ) return a.position[i] < b.position[i];
        }
        for( int i = 0; i < 3; i++ ) {
            if( a.normal[i] != b.normal[i] ) return a.normal[i] < b.normal[i];
        }
        for( int j = 0; j < 2; j++ ) {
            for( int i = 0; i < 2; i++ ) {
                if( a.uv[j][i] != b.uv[j][i] ) return a.uv[j][i] < b.uv[j][i];
            }
        }

        return false;
    }
};

class FbxLoader {
public:

    typedef math::MeshIndexer<Vertex, VertexCompare> FbxMeshIndexer;

    //! Loads an FBX from file.
    bool            load( const char* filePath );

    const FbxMeshIndexer::VertexBuffer& vertexBuffer() const { return m_mesh.vertexBuffer(); }
    const FbxMeshIndexer::IndexBuffer&  indexBuffer() const { return m_mesh.indexBuffer(); }

private:

    void            extractObject( FbxNode* fbxNode );
    void            loadMesh( FbxNode* pNode );
    void            loadGeometryInfo( FbxMesh* pFbxMesh );

private:

    //! FBX sdk manager.
    FbxManager*     m_manager;

    //! FBX scene.
    FbxScene*       m_scene;

    //! Mesh indexer.
    FbxMeshIndexer  m_mesh;

    //! Mesh vertex buffer.
//    VertexBuffer    m_vertexBuffer;

    //! Mesh index buffer.
//    IndexBuffer     m_indexBuffer;
};

} // namespace fbx

#endif /* defined(__demo__FbxLoader__) */
