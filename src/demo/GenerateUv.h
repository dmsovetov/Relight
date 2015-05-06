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

#include <math/Mesh.h>
#include <math/DCEL.h>

// ** class GenerateUv
class GenerateUv : public platform::WindowDelegate {
public:

                        GenerateUv( renderer::Hal* hal );

    virtual void        handleUpdate( platform::Window* window );

private:

	scene::MeshPtr					createMeshFromFile( CString fileName );

private:

	typedef math::TriMesh<SceneVertex> SceneMesh;
    typedef SceneMesh::Dcel::Edge HalfEdge;
    typedef SceneMesh::Chart Chart;
    typedef SceneMesh::Face  Face;
	typedef math::MeshIndexer<SceneVertex, SceneVertexCompare> SceneMeshIndexer;

    int                             setChartIndex( const SceneMesh& mesh, const math::Vec3& axis, const HalfEdge* edge, int index );

    renderer::Hal*                  m_hal;
    renderer::VertexDeclaration*    m_meshVertexLayout;

	scene::ScenePtr					m_simpleScene;

    SceneMesh*                      m_loadedTriMesh;
    SceneMesh::Vertices             m_loadedVertices;
    SceneMesh::Indices              m_loadedIndices;

    std::vector<Chart*>             m_charts;
    std::map<int, int>              m_chartIndex;
    std::map<int, math::Vec3>       m_chartColor;
};

#endif /* defined(__Relight_Demo_GenerateUv__) */
