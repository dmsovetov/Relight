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

#ifndef __Relight_Demo_Lightmapping__
#define __Relight_Demo_Lightmapping__

#include "RelightDemo.h"
#include <uscene/src/uScene.h>

// ** struct SceneVertex
struct SceneVertex {
    float           x, y, z;
    float           nx, ny, nz;
    unsigned char   r, g, b, a;
    float           u0, v0;
    float           u1, v1;
};

// ** struct SceneMesh
struct SceneMesh {
    relight::Mesh*          m_mesh;
    renderer::Texture*      m_diffuse;
    renderer::IndexBuffer*  m_indexBuffer;
    renderer::VertexBuffer* m_vertexBuffer;
};

// ** struct SceneMeshInstance
struct SceneMeshInstance {
    const SceneMesh*        m_mesh;
    relight::Matrix4        m_transform;
    renderer::Texture2D*    m_lightmap;
    relight::Lightmap*      m_lm;
    relight::Photonmap*     m_pm;
};

// ** class Lightmapping
class Lightmapping : public platform::WindowDelegate {
public:

                        Lightmapping( renderer::Hal* hal );

    virtual void        handleUpdate( platform::Window* window );

private:

    SceneMesh*          findMesh( const uscene::Asset* asset, const uscene::Renderer* renderer );
    relight::Matrix4    affineTransform( const uscene::Transform* transform );
    void                createBuffersFromMesh( SceneMesh& mesh );
    renderer::Texture*  createTextureFromAsset( const uscene::Asset* asset );
    void                createGroundPlane( int size, relight::VertexBuffer& vertices, relight::IndexBuffer& indices );

private:

    typedef std::map<const uscene::Asset*, SceneMesh>           Meshes;
    typedef std::map<const uscene::Asset*, renderer::Texture*>  Textures;

    renderer::Hal*                  m_hal;
    renderer::VertexDeclaration*    m_meshVertexLayout;

    uscene::Assets*                 m_assets;
    uscene::Scene*                  m_scene;
    relight::Scene*                 m_relightScene;

    Meshes                          m_meshes;
    Textures                        m_textures;
};

#endif /* defined(__Relight_Demo_Lightmapping__) */
