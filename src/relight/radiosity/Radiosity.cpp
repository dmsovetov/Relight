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

#include "../BuildCheck.h"

#include "Radiosity.h"

namespace relight {

// ** Radiosity::createPatches
void Radiosity::addPatches( const Mesh* mesh, const Patches& patches )
{
	DC_BREAK_IF( m_patchRanges.count( mesh ) != 0 );

	// ** Register the patch range.
	PatchRange range( m_patches.size(), patches.size() );
	m_patchRanges[mesh] = range;

	// ** Add patches.
	m_patches.insert( m_patches.end(), patches.begin(), patches.end() );
}

// ** Radiosity::patchRangeForMesh
bool Radiosity::patchRangeForMesh( const Mesh* mesh, u32& first, u32& count ) const
{
	PatchRangeByMesh::const_iterator i = m_patchRanges.find( mesh );
	
	if( i == m_patchRanges.end() ) {
		return false;
	}

	first = i->second.m_first;
	count = i->second.m_count;
}


} // namespace relight
