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

#ifndef __Relight_Demo_GenerateUv__
#define __Relight_Demo_GenerateUv__

#include "RelightDemo.h"

typedef TriMesh<SceneVertex> SceneTriMesh;
typedef SceneTriMesh::Dcel::Edge HalfEdge;
typedef SceneTriMesh::Chart Chart;
typedef SceneTriMesh::Face  Face;
typedef MeshIndexer<SceneVertex, SceneVertexCompare> SceneMeshIndexer;

// ** class GenerateUv
class GenerateUv : public platform::WindowDelegate {
public:

                        GenerateUv( renderer::Hal* hal );

    virtual void        handleUpdate( platform::Window* window );

private:

	scene::MeshPtr					createMeshFromFile( CString fileName );

private:

    renderer::Hal*                  m_hal;
    renderer::VertexDeclaration*    m_meshVertexLayout;

	scene::ScenePtr					m_simpleScene;

    SceneTriMesh*					m_loadedTriMesh;
    SceneTriMesh::Vertices			m_loadedVertices;
    SceneTriMesh::Indices			m_loadedIndices;
};

#endif /* defined(__Relight_Demo_GenerateUv__) */
