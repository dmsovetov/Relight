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

#include "Assets.h"

#include "Yaml.h"
#include "FbxLoader.h"

namespace uscene {

Matrix4 parseTransform( const Yaml& node )
{
    Vec3 position = parseVec3( node["position"] );
    Vec3 scale    = parseVec3( node["scale"] );
    Quat rotation = parseQuat( node["rotation"] );

    return Matrix4::translation( position ) * rotation.toMatrix() * Matrix4::scale( scale );
}

Quat parseQuat( const Yaml& node )
{
    return Quat( node["x"].number(), node["y"].number(), node["z"].number(), node["w"].number() );
}

Scene* parseScene( Assets* assets, const char* fileName )
{
    Yaml yaml = Yaml::parse( fileName );
    if( !yaml ) {
        return NULL;
    }

    Scene* scene = new Scene;

    const Yaml& objects = yaml["objects"];

    for( int i = 0; i < objects.size(); i++ ) {
        scene->m_objects.push_back( parseSceneObject( assets, objects[i]["components"] ) );
    }

    return scene;
}

SceneObject* parseSceneObject( Assets* assets, const Yaml& components )
{
    SceneObject* sceneObject = new SceneObject;

    for( int i = 0; i < components.size(); i++ ) {
        const Yaml& item      = components[i];
        std::string type      = *item.keys().begin();
        const Yaml& component = item[type];

        if( type == "transform" ) {
            sceneObject->m_transform = parseTransform( component );
        }
        else if( type == "mesh" ) {
            const char* prefab = component["prefab"] ? component["prefab"].str() : component["asset"].str();

            if( strstr( prefab, ".prefab" ) ) {
                Scene* prefabScene = assets->prefab( prefab );
                assert( prefabScene->m_objects.size() == 1 );
                sceneObject->m_mesh = prefabScene->m_objects[0]->m_mesh;
            } else {
                sceneObject->m_mesh = assets->mesh( prefab );
            }
        }
        else if( type == "renderer" ) {
            sceneObject->m_material = assets->material( component["materials"][0].str() );
        }
    }

    return sceneObject;
}

Mesh* Assets::mesh( const char* fileName )
{
    if( m_meshes.count( fileName ) ) {
        return m_meshes[fileName];
    }

    Mesh* mesh = FbxLoader().load( fileName );
    m_meshes[fileName] = mesh;

    printf( "Mesh %s loaded, %d vertices %d faces\n", fileName, mesh->vertexCount(), mesh->faceCount() );

    return mesh;
}

Scene* Assets::prefab( const char* fileName )
{
    if( m_prefabs.count( fileName ) ) {
        return m_prefabs[fileName];
    }

    Scene* prefab = parseScene( this, fileName );
    m_prefabs[fileName] = prefab;

    printf( "Prefab %s loaded, %d objects\n", fileName, prefab->m_objects.size() );
    
    return prefab;
}

SceneMaterial* createMaterialFromTexture( const char* fileName, const Color& color )
{
    Texture*  texture  = Texture::createFromFile( fileName );
    Material* material = texture ? new TexturedMaterial( texture, color ) : new Material( color );

    SceneMaterial* sceneMaterial = new SceneMaterial;
    sceneMaterial->m_material = material;
    sceneMaterial->m_texture  = texture;

    return sceneMaterial;
}

SceneMaterial* Assets::material( const char* fileName )
{
    if( m_materials.count( fileName ) ) {
        return m_materials[fileName];
    }

    Yaml yaml = Yaml::parse( fileName );
    if( !yaml ) {
        return NULL;
    }

    SceneMaterial* material = createMaterialFromTexture( yaml["textures"]["diffuse"]["asset"].str(), parseColor( yaml["colors"]["diffuse"] ) );
    m_materials[fileName] = material;

    printf( "Material %s loaded\n", fileName );

    return material;
}

Assets* parseAssets( const char* fileName )
{
    Yaml yaml = Yaml::parse( fileName );
    if( !yaml ) {
        return NULL;
    }

    Assets* a = new Assets;

    const Yaml& assets = yaml["assets"];

    for( int i = 0; i < assets.size(); i++ ) {
        const Yaml& asset    = assets[i];
        const char* fileName = asset["fileName"].str();

        if( asset["model"] ) {
            float scale = asset["model"]["meshes"]["scale"].number();
            Mesh* mesh  = a->mesh( fileName );
            mesh->transform( Matrix4::scale( scale, scale, scale ) );
        }
        else if( strstr( fileName, ".mat" ) ) {
            a->material( fileName );
        }
    }

    return a;
}

Color parseColor( const Yaml& node )
{
    return Color( node["r"].number(), node["g"].number(), node["b"].number() );
}

Vec3 parseVec3( const Yaml& node )
{
    return Vec3( node["x"].number(), node["y"].number(), node["z"].number() );
}

} // namespace uscene