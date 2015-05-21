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

#ifndef __Relight_Radiosity_H__
#define __Relight_Radiosity_H__

#include "../Relight.h"

namespace relight {

	//! A radiosity data is represented by a set of patches and it's form factors for each mesh in scene.
	class Radiosity {
	public:

		struct Patch;

		//! A form factor between two patches.
		/*!
		 Form factor is a fraction of light leaving element A arriving at element B.
		*/
		struct FormFactor {
			const Patch*	m_patch;
			f32				m_weight;

							FormFactor( void ) {}
							FormFactor( const Patch* patch, f32 weight )
								: m_patch( patch ), m_weight( weight ) {}
		};

		//! An array of form factors.
		typedef Array<FormFactor>	FormFactors;

		//! Radiosity patch.
		struct Patch {
			Rgb					m_injected;		//!< The injected incoming radiance (used for a zero iteration of radiosity solver).
			Rgb					m_diffuse;		//!< The diffuse color (reflectance) of a patch.
			Rgb					m_indirect;		//!< The computed incoming radiance.
			FormFactors			m_ff;			//!< An array of form factors (list of patches this one is influenced by and their weights).

			const Mesh*			m_mesh;			//!< The parent mesh for a patch.
			const Lumel*		m_lumel;		//!< The linked lightmap pixel.

								//! Constructs the RadiosityPatch instance.
								Patch( const Mesh* mesh = NULL, const Lumel* lumel = NULL )
									: m_mesh( mesh ), m_lumel( lumel ), m_injected( 0.0f, 0.0f, 0.0f ), m_diffuse( 0.0f, 0.0f, 0.0f ), m_indirect( 0.0f, 0.0f, 0.0f ) {}
		};

		//! An array of radiosity patches.
		typedef Array<Patch>	Patches;

		//! Returns the total number of patches.
		s32						patchCount( void ) const;

		//! Returns the patch by index.
		Patch&					patch( s32 index );

		//! Returns the patch by index.
		const Patch&			patch( s32 index ) const;

		//! Adds patches for a specified scene mesh.
		void					addPatches( const Mesh* mesh, const Patches& patches );

		//! Returns an set of patches associated with a specified mesh.
		bool					patchRangeForMesh( const Mesh* mesh, u32& first, u32& count ) const;

	private:

		//! A helper struct to store the range of mesh patches in global sample buffer.
		struct PatchRange {
			u32			m_first;	//!< The first patch index.
			u32			m_count;	//!< Total patches for a mesh.

						//! Constructs the PatchRange instance.
						PatchRange( u32 first = 0, u32 count = 0 )
							: m_first( first ), m_count( count ) {}
		};

		//! A container type to store the mapping from mesh to it's radiosity patches.
		typedef Map<const Mesh*, PatchRange>	PatchRangeByMesh;

		PatchRangeByMesh	m_patchRanges;	//!< Patch ranges for each mesh.
		Patches				m_patches;
	};

} // namespace relight

#endif /* defined(__Relight_Radiosity_H__) */
