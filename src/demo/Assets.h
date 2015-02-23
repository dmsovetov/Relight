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

#ifndef __demo__Assets__
#define __demo__Assets__

#include <stdio.h>
#include <Relight.h>

class Yaml;

namespace uscene {

    using namespace relight;

    struct SceneMaterial {
        Material*   m_material;
        Texture*    m_texture;
    };

    struct SceneObject {
        Matrix4         m_transform;
        SceneMaterial*  m_material;  //!< Material
        Mesh*           m_mesh;      //!< Mesh
    };

    struct Scene {
        std::vector<SceneObject*>   m_objects;
    };

    struct Assets {
        std::map<std::string, SceneMaterial*>   m_materials;
        std::map<std::string, Mesh*>            m_meshes;
        std::map<std::string, Scene*>           m_prefabs;

        Mesh*           mesh( const char* fileName );
        SceneMaterial*  material( const char* fileName );
        Scene*          prefab( const char* fileName );
    };

    SceneMaterial*  parseMaterial( const char* fileName );

    Color           parseColor( const Yaml& node );
    Vec3            parseVec3( const Yaml& node );
    Matrix4         parseTransform( const Yaml& node );
    Quat            parseQuat( const Yaml& node );

    Assets*         parseAssets( const char* fileName );
    SceneObject*    parseSceneObject( Assets* assets, const Yaml& node );
    Scene*          parseScene( Assets* assets, const char* fileName );
    SceneMaterial*  createMaterialFromTexture( const char* fileName, const Color& color = Color( 1, 1, 1 ) );
/*
    class Object {
    public:
    };

    class Component : public Object {
    public:
    };

//    typedef std::

    class SceneObject : public Object {
    public:
    };
*/
}

#endif /* defined(__demo__Assets__) */
