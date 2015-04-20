// Original code by Uri Zapoev
// https://github.com/alkoholick

#include "FbxLoader.h"

namespace fbx {

bool  FbxLoader::load( const char * filePath )
{
    m_manager = FbxManager::Create();
    FbxIOSettings* ioSettings = FbxIOSettings::Create(m_manager, IOSROOT);

    m_manager->SetIOSettings(ioSettings);

    FbxString lPath = FbxGetApplicationDirectory();
    m_manager->LoadPluginsDirectory(lPath.Buffer());

    FbxImporter* importer = FbxImporter::Create(m_manager, "");
    bool initialised = importer->Initialize(filePath, -1, m_manager->GetIOSettings());
    if(!initialised)
    {
        return false;
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
    //FbxAxisSystem::OpenGL.ConvertScene(m_scene);
    FbxNode* pRootNode = m_scene->GetRootNode();

    extractObject(pRootNode);

    return true;
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

    int iAttribCount = fbxNode->GetNodeAttributeCount();
    for(int k = 0; k < iAttribCount; k++)
    {
        FbxNodeAttribute::EType fbxAttributeType = fbxNode->GetNodeAttributeByIndex(k)->GetAttributeType();
        if( fbxAttributeType != FbxNodeAttribute::eMesh )
        {
            continue;
        }
        pFbxMesh = (FbxMesh*) fbxNode->GetNodeAttributeByIndex(k);

        loadGeometryInfo(pFbxMesh);
    }
}

void FbxLoader::loadGeometryInfo( FbxMesh* pFbxMesh )
{
    int iConrolPointsCount      = pFbxMesh->GetControlPointsCount();
    FbxVector4* pControlPoints  = pFbxMesh->GetControlPoints();
    int lPolygonCount           = pFbxMesh->GetPolygonCount();

    FbxStringList UVSetNameList;
    pFbxMesh->GetUVSetNames( UVSetNameList );// Get the name of each set of UV coords

    const char *pUVLayerName = UVSetNameList.GetCount()?UVSetNameList.GetStringAt(0):"";

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


            Vertex vidx;
            vidx.position = vPos;
            vidx.normal   = fbxNormal;
            vidx.uv[0]    = fbxTexCoord0;
            vidx.uv[1]    = fbxTexCoord1;

            m_vertexBuffer.push_back( vidx );
            m_indexBuffer.push_back(m_vertexBuffer.size() - 1);
        }
    }
}

} // namespace fbx