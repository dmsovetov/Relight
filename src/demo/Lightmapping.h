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
    float           u0, v0;
    float           u1, v1;
};

// ** struct SceneMesh
struct SceneMesh {
    relight::Mesh*          m_mesh;
    relight::Material*      m_material;
    
    renderer::Texture*      m_diffuse;
    renderer::IndexBuffer*  m_indexBuffer;
    renderer::VertexBuffer* m_vertexBuffer;
};

// ** struct SceneMeshInstance
struct SceneMeshInstance {
    const SceneMesh*            m_mesh;

    relight::Matrix4            m_transform;
    renderer::Texture2D*        m_lightmap;
    relight::Lightmap*          m_lm;
    relight::Photonmap*         m_pm;
    bool                        m_dirty;
};

//! Relight background worker.
class ThreadWorker : public relight::Worker {
public:

                    //! Constructs a ThreadWorker instance.
                    ThreadWorker( void );

    //! Pushes a new job to this worker.
    virtual void    push( relight::Job* job, relight::JobData* data );

    //! Waits for completion of this worker.
    virtual void    wait( void );

    virtual void    notify( const relight::Mesh* instance, int step, int stepCount );

private:

    //! Thread worker callback.
    static void     worker( void* userData );

private:

    //! Thread worker data
    struct ThreadData {
        relight::JobData*   m_data;
        relight::Progress*  m_progress;
    };

    //! Thread handle.
	thread::Thread*	m_thread;
};

// ** class Lightmapping
class Lightmapping : public platform::WindowDelegate {
public:

                        Lightmapping( renderer::Hal* hal );

    virtual void        handleUpdate( platform::Window* window );

private:

    SceneMesh*          findMesh( const uscene::Asset* asset, const uscene::Renderer* renderer, bool solid );
    relight::Texture*   findTexture( const uscene::Asset* asset, bool solid );
    relight::Matrix4    affineTransform( const uscene::Transform* transform );
    void                createBuffersFromMesh( SceneMesh& mesh );
    renderer::Texture*  createTexture( const relight::Texture* texture );
	void                renderObjects( renderer::Shader* shader, const uscene::SceneObjectArray& objects );
	void				renderBasis( const math::Vec3& origin = math::Vec3( 0, 0, 0 ), const math::Vec3& front = math::Vec3( 0, 0, 1 ), const math::Vec3& up = math::Vec3( 0, 1, 0 ), const math::Vec3& right = math::Vec3( 1, 0, 0 ) );

private:

    typedef std::map<const uscene::Asset*,      SceneMesh>           Meshes;
    typedef std::map<const relight::Texture*,   renderer::Texture*>  Textures;
    typedef std::map<const uscene::Asset*,      relight::Texture*>   RelightTextures;

    renderer::Hal*                  m_hal;
    renderer::VertexDeclaration*    m_meshVertexLayout;
	renderer::Shader*				m_shaderLightmaped;
	renderer::Shader*				m_shaderColored;
	renderer::Shader*				m_shaderNormals;

	relight::Matrix4				m_matrixView;
	relight::Matrix4				m_matrixProj;

    uscene::Assets*                 m_assets;
    uscene::Scene*                  m_scene;
    uscene::SceneObjectArray        m_solidRenderList, m_transparentRenderList, m_additiveRenderList;

    relight::Relight*               m_relight;
    relight::Scene*                 m_relightScene;
    relight::Worker*                m_rootWorker;
    relight::Workers                m_relightWorkers;

    Meshes                          m_meshes;
    Textures                        m_textures;
    RelightTextures                 m_relightTextures;
};

#endif /* defined(__Relight_Demo_Lightmapping__) */
