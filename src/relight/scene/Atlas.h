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


#ifndef __Relight_Scene_Atlas_H__
#define __Relight_Scene_Atlas_H__

#include "../Relight.h"

namespace relight {

    //! Atlas class is used to pack small rectangles to a big rectangle region.
    class Atlas {
    public:

        //! Atlas area rectangle.
        struct Rectangle {
            int                 m_x;        //!< Left corner.
            int                 m_y;        //!< Top corner.
            int                 m_width;    //!< Rectangle width.
            int                 m_height;   //!< Rectangle height;

                                //! Constructs a Rectangle instance.
                                Rectangle( int x = 0, int y = 0, int width = 0, int height = 0 )
                                    : m_x( x ), m_y( y ), m_width( width ), m_height( height ) {}

            //! Compares two rectangles
            static bool         compare( const Rectangle& a, const Rectangle& b ) { return a.m_width * a.m_height > b.m_width * b.m_height; }
        };

    public:

        //! Places all rectangles into the bigger area with given dimensions.
        bool                    place( int width, int height );

        //! Adds a new rectangle.
        void                    addRectangle( int width, int height );

        //! Returns a rectangle by index.
        const Rectangle&        rectangle( int index ) const;

        //! Returns a total number of rectangles.
        int                     rectangleCount( void ) const;

    private:

        typedef Array<Rectangle>    RectangleArray;

        //! Rectangles that should be placed into the atlas.
        RectangleArray              m_rectangles;
    };

} // namespace relight

#endif  /*  !defined(__Relight_Scene_Atlas_H__) */
