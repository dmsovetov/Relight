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

#ifndef	__Relight_RadiosityBuilder_H__
#define	__Relight_RadiosityBuilder_H__

#include "Radiosity.h"

namespace relight {

	//! Radiosity builder computes a form factors between the patches for a specified scene.
	class RadiosityBuilder {
	public:

								//! Constructs RadiosityBuilder instance.
								RadiosityBuilder( Scene* scene );

		//! Initializes the patches and computes form factors for them.
		Radiosity				build( f32 formFactorThreshold = 0.1f, s32 maxFormFactors = 0 );

	private:

		//! Creates radiosity patches.
		void					createPatches( Radiosity& radiosity );

		//! Computes a form factors for each patch.
		void					computeFormFactors( Radiosity& radiosity );

		//! Refines produced form factors
		void					refineFormFactors( Radiosity::Patch& sample );

		//! Computes a distance-based form factor between patches.
		static void				distanceFormFactor( Scene* scene, Radiosity::Patch& receiver, Radiosity::Patch& sender, f32 threshold );

		//! Computes a distance-area-based form factor between patches.
		static void				distanceSqAreaFormFactor( Scene* scene, Radiosity::Patch& receiver, Radiosity::Patch& sender, f32 threshold );


	private:

		Scene*					m_scene;				//!< The source scene.
		f32						m_formFactorThreshold;	//!< The minimum weight of a formfactor.
		s32						m_maxFormFactors;		//!< The maximum amount of form factors.
		s32						m_totalFormFactors;		//!< The total number of produced form factors.
	};

} // namespace relight

#endif	/*	!__Relight_RadiosityBuilder_H__	*/