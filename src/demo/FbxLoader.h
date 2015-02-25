// Original code by Uri Zapoev
// https://github.com/alkoholick

#ifndef __demo__FbxLoader__
#define __demo__FbxLoader__

#include <stdio.h>
#include <assert.h>
#include <fbxsdk.h>
#include <vector>

namespace fbx {

struct Vertex {
    FbxVector4  position;
    FbxVector4  normal;
    FbxVector2  uv[2];
};

typedef std::vector<unsigned short> IndexBuffer;
typedef std::vector<Vertex>         VertexBuffer;

class FbxLoader {
public:

    //! Loads an FBX from file.
    bool            load( const char* filePath );

    const VertexBuffer& vertexBuffer() const { return m_vertexBuffer; }
    const IndexBuffer&  indexBuffer() const { return m_indexBuffer; }

private:

    void            extractObject( FbxNode* fbxNode );
    void            loadMesh( FbxNode* pNode );
    void            loadGeometryInfo( FbxMesh* pFbxMesh );

private:

    //! FBX sdk manager.
    FbxManager*     m_manager;

    //! FBX scene.
    FbxScene*       m_scene;

    //! Mesh vertex buffer.
    VertexBuffer    m_vertexBuffer;

    //! Mesh index buffer.
    IndexBuffer     m_indexBuffer;
};

} // namespace fbx

#endif /* defined(__demo__FbxLoader__) */
