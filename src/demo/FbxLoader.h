// Original code by Uri Zapoev
// https://github.com/alkoholick

#ifndef __demo__FbxLoader__
#define __demo__FbxLoader__

#include <stdio.h>
#include <assert.h>
#include <fbxsdk.h>
#include <vector>
#include <Relight.h>

//struct FbxSubMesh {
//    relight::VertexBuffer   m_vertices;
//    relight::IndexBuffer    m_indices;
//};

class FbxLoader {
public:

    //! Loads an FBX from file.
    relight::Mesh*  load( const char* filePath );

private:

    void            extractObject(FbxNode * fbxNode);
    void            loadMesh(FbxNode* pNode);
    void            loadGeometryInfo( FbxMesh* pFbxMesh/*, FbxSubMesh& mesh */);

private:

    //! FBX sdk manager.
    FbxManager*     m_manager;

    //! FBX scene.
    FbxScene*       m_scene;

    relight::Mesh*  m_mesh;
    std::vector<FbxNode*> m_fbxMeshes;
};

#endif /* defined(__demo__FbxLoader__) */
