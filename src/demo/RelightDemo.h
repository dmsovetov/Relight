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

#ifndef __Relight_RelightDemo__
#define __Relight_RelightDemo__


#include <Relight.h>
#include <Dreemchest.h>

DC_USE_DREEMCHEST

// ** struct SceneVertex
struct SceneVertex {
    Vec3		position;
    Vec3		normal;
    Vec2      uv[2];
};

// ** SceneVertexCompare 
struct SceneVertexCompare {
    bool operator()( const SceneVertex& a, const SceneVertex& b ) const
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

// ** class RelightDemo
class RelightDemo : public platform::ApplicationDelegate {
private:

    virtual void handleLaunched( platform::Application* application );

private:

    platform::Window*		m_window;
    renderer::Hal*			m_hal;
    renderer::RenderView*	m_view;
};

#endif /* defined(__Relight_RelightDemo__) */
