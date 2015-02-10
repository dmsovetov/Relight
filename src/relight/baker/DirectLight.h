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

#ifndef __Relight_Bake_DirectLight_H__
#define __Relight_Bake_DirectLight_H__

#include "Baker.h"
#include "../Vector.h"

namespace relight {

namespace bake {

    //! Bakes a direct light to a lightmap texture.
    class DirectLight : public Baker {
    public:

                                //! Constructs a DirectLight instance.
                                DirectLight( const Scene* scene, Progress* progress );

        //! Calculates a light influence by a Lambert's cosine law.
        static float            lambert( const Vec3& direction, const Vec3& normal );

    protected:

        // ** Baker
        virtual void            bakeLumel( Lumel& lumel );

    private:

        Color                   lightFromSource( const Vec3& position, const Vec3& normal, const PointLight* light ) const;
    };

} // namespace bake

} // namespace relight

#endif  /*  !defined( __Relight_Bake_DirectLight_H__ ) */
