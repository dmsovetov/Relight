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

// ** IndirectLightSettings::draft
IndirectLightSettings IndirectLightSettings::draft( const Color& skyColor, float photonMaxDistance, float finalGatherDistance )
{
    IndirectLightSettings settings;

    settings.m_skyColor                 = skyColor;

    settings.m_photonPassCount          = 8;
    settings.m_photonBounceCount        = 1;
    settings.m_photonEnergyThreshold    = 0.05f;
    settings.m_photonMaxDistance        = photonMaxDistance;

    settings.m_finalGatherSamples       = 32;
    settings.m_finalGatherDistance      = finalGatherDistance;
    settings.m_finalGatherRadius        = 7;
    
    return settings;
}

// ** IndirectLightSettings::fast
IndirectLightSettings IndirectLightSettings::fast( const Color& skyColor, float photonMaxDistance, float finalGatherDistance )
{
    IndirectLightSettings settings;

    settings.m_skyColor                 = skyColor;

    settings.m_photonPassCount          = 16;
    settings.m_photonBounceCount        = 3;
    settings.m_photonEnergyThreshold    = 0.05f;
    settings.m_photonMaxDistance        = photonMaxDistance;

    settings.m_finalGatherSamples       = 64;
    settings.m_finalGatherDistance      = finalGatherDistance;
    settings.m_finalGatherRadius        = 7;

    return settings;
}

// ** IndirectLightSettings::best
IndirectLightSettings IndirectLightSettings::best( const Color& skyColor, float photonMaxDistance, float finalGatherDistance )
{
    IndirectLightSettings settings;

    settings.m_skyColor                 = skyColor;

    settings.m_photonPassCount          = 32;
    settings.m_photonBounceCount        = 3;
    settings.m_photonEnergyThreshold    = 0.05f;
    settings.m_photonMaxDistance        = photonMaxDistance;

    settings.m_finalGatherSamples       = 128;
    settings.m_finalGatherDistance      = finalGatherDistance;
    settings.m_finalGatherRadius        = 7;

    return settings;
}

// ** IndirectLightSettings::production
IndirectLightSettings IndirectLightSettings::production( const Color& skyColor, float photonMaxDistance, float finalGatherDistance )
{
    IndirectLightSettings settings;

    settings.m_skyColor                 = skyColor;

    settings.m_photonPassCount          = 64;
    settings.m_photonBounceCount        = 4;
    settings.m_photonEnergyThreshold    = 0.05f;
    settings.m_photonMaxDistance        = photonMaxDistance;

    settings.m_finalGatherSamples       = 1024;
    settings.m_finalGatherDistance      = finalGatherDistance;
    settings.m_finalGatherRadius        = 7;

    return settings;
}

// ** AmbientOcclusionSettings::draft
AmbientOcclusionSettings AmbientOcclusionSettings::draft( float occludedFraction, float maxDistance, float exponent )
{
    AmbientOcclusionSettings settings;

    settings.m_samples          = 128;
    settings.m_maxDistance      = maxDistance;
    settings.m_exponent         = exponent;
    settings.m_occludedFraction = occludedFraction;

    return settings;
}

// ** AmbientOcclusionSettings::draft
AmbientOcclusionSettings AmbientOcclusionSettings::fast( float occludedFraction, float maxDistance, float exponent )
{
    AmbientOcclusionSettings settings;

    settings.m_samples          = 256;
    settings.m_maxDistance      = maxDistance;
    settings.m_exponent         = exponent;
    settings.m_occludedFraction = occludedFraction;

    return settings;
}

// ** AmbientOcclusionSettings::best
AmbientOcclusionSettings AmbientOcclusionSettings::best( float occludedFraction, float maxDistance, float exponent )
{
    AmbientOcclusionSettings settings;

    settings.m_samples          = 1024;
    settings.m_maxDistance      = maxDistance;
    settings.m_exponent         = exponent;
    settings.m_occludedFraction = occludedFraction;

    return settings;
}

// ** AmbientOcclusionSettings::production
AmbientOcclusionSettings AmbientOcclusionSettings::production( float occludedFraction, float maxDistance, float exponent )
{
    AmbientOcclusionSettings settings;

    settings.m_samples          = 2048;
    settings.m_maxDistance      = maxDistance;
    settings.m_exponent         = exponent;
    settings.m_occludedFraction = occludedFraction;

    return settings;
}

// ** Relight::bakeDirectLight
RelightStatus Relight::bakeDirectLight( const Scene* scene, const Mesh* mesh, Progress* progress, bake::BakeIterator* iterator )
{
    if( !iterator ) {
        iterator = new bake::LumelBakeIterator( 0, 1 );
    }

    TimeMeasure measure( "Direct Lighting" );
    bake::DirectLight* direct = new bake::DirectLight( scene, progress, iterator );
    RelightStatus status = direct->bakeMesh( mesh );
    delete direct;

    return status;
}

// ** Relight::bakeIndirectLight
RelightStatus Relight::bakeIndirectLight( const Scene* scene, const Mesh* mesh, Progress* progress, const IndirectLightSettings& settings, bake::BakeIterator* iterator )
{
    if( !iterator ) {
        iterator = new bake::LumelBakeIterator( 0, 1 );
    }

    RelightStatus status;

    {
        TimeMeasure measure( "Photon tracing" );

        bake::Photons* photons = new bake::Photons( scene, settings.m_photonPassCount, settings.m_photonBounceCount, settings.m_photonEnergyThreshold, settings.m_photonMaxDistance );
        status = photons->emit();
        delete photons;

        if( status != RelightSuccess ) {
            return status;
        }
    }

    {
        TimeMeasure measure( "Indirect Lighting" );

        bake::IndirectLight* indirect = new bake::IndirectLight( scene, progress, iterator, settings.m_finalGatherSamples, settings.m_finalGatherDistance, settings.m_finalGatherRadius, settings.m_skyColor );
        status = indirect->bakeMesh( mesh );
        delete indirect;
    }

    return status;
}

// ** Relight::bakeAmbientOcclusion
RelightStatus Relight::bakeAmbientOcclusion( const Scene* scene, const Mesh* mesh, Progress* progress, const AmbientOcclusionSettings& settings, bake::BakeIterator* iterator )
{
    if( !iterator ) {
        iterator = new bake::LumelBakeIterator( 0, 1 );
    }

    TimeMeasure measure( "Ambient Occlusion" );
    bake::AmbientOcclusion* ao = new bake::AmbientOcclusion( scene, progress, iterator, settings.m_samples, settings.m_occludedFraction, settings.m_maxDistance, settings.m_exponent );
    RelightStatus status = ao->bakeMesh( mesh );
    delete ao;

    return status;
}

} // namespace relight
