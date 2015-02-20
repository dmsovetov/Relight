// Original code by Uri Zapoev
// https://github.com/alkoholick

#include "FbxLoader.h"

relight::Mesh* FbxLoader::load( const char * filePath )
{
    assert(filePath);

    m_manager = FbxManager::Create();
    FbxIOSettings* ioSettings = FbxIOSettings::Create(m_manager, IOSROOT);

    m_manager->SetIOSettings(ioSettings);

    FbxString lPath = FbxGetApplicationDirectory();
    m_manager->LoadPluginsDirectory(lPath.Buffer());

    FbxImporter* importer = FbxImporter::Create(m_manager, "");
    bool initialised = importer->Initialize(filePath, -1, m_manager->GetIOSettings());
    if(!initialised)
    {
        printf("Failed importer");
        return false; // failed
    }

    int lFileMajor = 0;
    int lFileMinor = 0;
    int lFileRevision = 0;
    importer->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

    // Create an empty scene
    m_scene = FbxScene::Create(m_manager, "tempScene");
    importer->Import(m_scene);
    importer->Destroy();

    FbxGeometryConverter lGeomConverter(m_manager);
    lGeomConverter.Triangulate(m_scene, true);
    lGeomConverter.SplitMeshesPerMaterial(m_scene, true);

    FbxAxisSystem as = m_scene->GetGlobalSettings().GetAxisSystem();

    //    FbxAxisSystem::DirectX.ConvertScene(m_pScene);
    FbxAxisSystem::OpenGL.ConvertScene(m_scene);
    FbxNode* pRootNode = m_scene->GetRootNode();

    m_mesh = relight::Mesh::create();

    extractObject(pRootNode);

    for (int i = 0; i < m_fbxMeshes.size(); ++i)
    {
        loadMesh( m_fbxMeshes[i] );
    }

    return m_mesh;
}

void FbxLoader::extractObject(FbxNode * fbxNode)
{
    const char * name = fbxNode->GetName();

    if (fbxNode->GetNodeAttribute() != NULL)
    {
        FbxNodeAttribute::EType lAttributeType = fbxNode->GetNodeAttribute()->GetAttributeType();

        if( lAttributeType ==  FbxNodeAttribute::eMesh)
        {
            loadMesh(fbxNode);
        }
        else if(lAttributeType == FbxNodeAttribute::eSkeleton)
        {
        }
    }

    for (int i = 0; i < fbxNode->GetChildCount(); ++i)
    {
        extractObject(fbxNode->GetChild(i));
    }
}

void FbxLoader::loadMesh(FbxNode* fbxNode)
{
    FbxMesh     * pFbxMesh     = fbxNode->GetMesh();
    FbxGeometry * pFbxGeometry = fbxNode->GetGeometry();
/*
    MeshSurface * pMeshSurface = NULL;
    std::vector<vertexPNBTWIidx>    vertexesIdx;
    std::vector<unsigned short>     indices;

    FbxSubMesh submesh;
*/
    int iAttribCount = fbxNode->GetNodeAttributeCount();
    for(int k = 0; k < iAttribCount; k++)
    {
   //     int            iDeformerCount = 0;

    //    vertexesIdx.clear();
    //    indices.clear();

        FbxNodeAttribute::EType fbxAttributeType = fbxNode->GetNodeAttributeByIndex(k)->GetAttributeType();
        if( fbxAttributeType != FbxNodeAttribute::eMesh )
        {
            continue;
        }
        pFbxMesh = (FbxMesh*) fbxNode->GetNodeAttributeByIndex(k);

        /// load mesh data
        loadGeometryInfo(pFbxMesh/*, submesh*/);
    //    m_loadedMeshes.push_back( submesh );
/*

        pMeshSurface = new MeshSurface();
        pMeshSurface->SetUserData(fbxNode);
        const char * pVertexDeclaration = NULL;
        if( iDeformerCount == 0 )
        {
            pVertexDeclaration = "P3:N3:T3:UV0:UV1";

            //            CalculateTangentsAndBinormals(vertexesIdx.size(), &vertexesIdx[0], indices.size(), &indices[0]);

            std::vector<vertexPNBT2> vertexes(vertexesIdx.size());
            for( size_t i = 0 ; i < vertexesIdx.size(); ++i )
            {
                vertexes[i].position  = vertexesIdx[i].position;
                vertexes[i].normal    = vertexesIdx[i].normal;
                vertexes[i].tangent   = vertexesIdx[i].tangent;
                vertexes[i].texcoord0 = vertexesIdx[i].texcoord0;
                vertexes[i].texcoord1 = vertexesIdx[i].texcoord1;
            }

            size_t datasize = sizeof(vertexPNBT2)* vertexes.size();
            pMeshSurface->SetVertexData(pVertexDeclaration, vertexes.size(), vertexes.data(), datasize);
        }
        pMeshSurface->SetIndexData(indices.data(), indices.size());

        pMeshSurface->SetMaterialInfo(pMaterialInfo);
        pMeshSurface->SetMeshName(fbxNode->GetName());

        m_surfaces.push_back(pMeshSurface);*/
    }
//    return pMeshSurface;
}

void FbxLoader::loadGeometryInfo(FbxMesh* pFbxMesh/*, FbxSubMesh& mesh*/)
{
    if(pFbxMesh)
    {
        int iConrolPointsCount      = pFbxMesh->GetControlPointsCount();
        FbxVector4* pControlPoints  = pFbxMesh->GetControlPoints();
        int lPolygonCount           = pFbxMesh->GetPolygonCount();

        FbxStringList UVSetNameList;
        pFbxMesh->GetUVSetNames( UVSetNameList );// Get the name of each set of UV coords

        const char *pUVLayerName = UVSetNameList.GetCount()?UVSetNameList.GetStringAt(0):"";

        relight::VertexBuffer vertices;
        relight::IndexBuffer indices;

        for (int i = 0; i < lPolygonCount; i++)
        {
            int lPolygonSize = pFbxMesh->GetPolygonSize(i);
            for (int j = 0; j < lPolygonSize; j++)
            {
                int lControlPointIndex = pFbxMesh->GetPolygonVertex(i, j);
                if (lControlPointIndex > iConrolPointsCount ) {
                    continue;
                }
                bool pUnmapped = true;
                FbxVector2 fbxTexCoord0;    // main uv
                FbxVector2 fbxTexCoord1;    // maybe lightmap uv
                FbxVector4 fbxNormal;

                FbxVector4 vPos = pControlPoints[lControlPointIndex];

                // Get the UV coords for this vertex in this poly which belong to the first UV set
                // Note: Using 0 as index into UV set list as this example supports only one UV set
                pFbxMesh->GetPolygonVertexUV( i, j, pUVLayerName, fbxTexCoord0, pUnmapped );
                pFbxMesh->GetPolygonVertexNormal(i, j, fbxNormal);

                if (UVSetNameList.GetCount() > 1)
                {
                    const char *secondUVName = UVSetNameList.GetStringAt(1);
                    pFbxMesh->GetPolygonVertexUV( i, j, secondUVName, fbxTexCoord1, pUnmapped );
                }
                pFbxMesh->GetElementTangent();


                relight::Vertex vidx;
            //    vidx.idx        = lControlPointIndex;
                vidx.m_position = relight::Vec3((float)vPos[0], (float)vPos[1], (float)vPos[2]);
                vidx.m_normal   = relight::Vec3((float)fbxNormal[0], (float)fbxNormal[1], (float)fbxNormal[2]);
                vidx.m_uv[0]    = relight::Uv((float)fbxTexCoord0[0], (float)fbxTexCoord0[1]);
                vidx.m_uv[1]    = relight::Uv((float)fbxTexCoord1[0], (float)fbxTexCoord1[1]);
                vidx.m_color    = relight::Color( 1, 1, 1 );

            //    unsigned short idx = addVertexGetIdx(vidx, vertexes);
                vertices.push_back( vidx );
                indices.push_back(vertices.size() - 1);/**/
            }
        }/**/

        m_mesh->addFaces( vertices, indices );
    }
}