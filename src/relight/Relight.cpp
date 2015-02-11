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

#include "BuildCheck.h"
#include "Relight.h"

#include "baker/AmbientOcclusion.h"
#include "baker/Photons.h"
#include "baker/IndirectLight.h"
#include "baker/DirectLight.h"

namespace relight {

// ** Relight::bakeDirectLight
RelightStatus Relight::bakeDirectLight( const Scene* scene, Progress* progress, bake::BakeIterator* iterator )
{
    if( !iterator ) {
        iterator = new bake::LumelBakeIterator( 0, 1 );
    }

    TimeMeasure measure( "Direct Lighting" );
    bake::DirectLight* direct = new bake::DirectLight( scene, progress, iterator );
    RelightStatus status = direct->bake();
    delete direct;

    return status;
}

// ** Relight::bakeIndirectLight
RelightStatus Relight::bakeIndirectLight( const Scene* scene, Progress* progress, bake::BakeIterator* iterator )
{
    TimeMeasure measure( "Indirect Lighting" );
    bake::Photons* photons = new bake::Photons( scene, progress, NULL, 64, 3, 0.05f, 10.0f );
    photons->bake();

    bake::IndirectLight* indirect = new bake::IndirectLight( scene, progress, iterator, 2048, 50, 7 );
//    bake::IndirectLight* indirect = new bake::IndirectLight( scene, progress, iterator, 256, 50, 7 );
    RelightStatus status = indirect->bake();

    delete photons;
    delete indirect;

    return status;
}

// ** Relight::bakeAmbientOcclusion
RelightStatus Relight::bakeAmbientOcclusion( const Scene* scene, Progress* progress, bake::BakeIterator* iterator )
{
    TimeMeasure measure( "Ambient Occlusion" );
    bake::AmbientOcclusion* ao = new bake::AmbientOcclusion( scene, progress, iterator, 4096, 0.8f, 0.6f );
    RelightStatus status = ao->bake();
    delete ao;

    return status;
}

} // namespace relight
