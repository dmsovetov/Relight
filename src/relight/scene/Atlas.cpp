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

#include "Atlas.h"

namespace relight {

// -------------------------------------- AtlasNode -------------------------------------- //

//! Atlas node represents a binary area partioning leaf.
struct AtlasNode {
    AtlasNode*          m_left;             //!< Left child node.
    AtlasNode*          m_right;            //!< Right child node.
    Atlas::Rectangle    m_rect;             //!< Node rectangle.
    bool                m_isUsed;           //!< Flag indicating that this node has a placed item.

    //! Places a rectangle to a node.
    bool                place( Atlas::Rectangle& rect );

    //! Returns true if this is a lead node (doesn't have children).
    bool                isLeaf( void ) const;

    //! Returns true if the region can be placed into this node.
    bool                fits( const Atlas::Rectangle& rect ) const;

    //! Returns true if the region has the same dimensions as this node.
    bool                perfectFit( const Atlas::Rectangle& rect ) const;

                        //! Constructs Node instance.
                        AtlasNode( const Atlas::Rectangle& rect );
                        AtlasNode( void );
                       ~AtlasNode( void );
};

// ** AtlasNode::AtlasNode
AtlasNode::AtlasNode( const Atlas::Rectangle& rect ) : m_left( NULL ), m_right( NULL ), m_rect( rect ), m_isUsed( false )
{
}

AtlasNode::AtlasNode( void ) : m_left( NULL ), m_right( NULL ), m_isUsed( false )
{
}

AtlasNode::~AtlasNode( void )
{
    delete m_left;
    delete m_right;
}

// ** AtlasNode::place
bool AtlasNode::place( Atlas::Rectangle& rect )
{
    if( !isLeaf() ) {
        if( m_left->place( rect ) ) {
            return true;
        }

        return m_right->place( rect );
    }

    if( m_isUsed ) {
        return false;
    }

    if( !fits( rect ) ) {
        return NULL;
    }

    if( perfectFit( rect ) ) {
        m_isUsed = true;
        rect.m_x = m_rect.m_x; rect.m_y = m_rect.m_y;
        return true;
    }

    int dw = m_rect.m_width  - rect.m_width;
    int dh = m_rect.m_height - rect.m_height;

    int x = m_rect.m_x;
    int y = m_rect.m_y;
    int w = m_rect.m_width;
    int h = m_rect.m_height;

    if( dw > dh ) {
        m_left  = new AtlasNode( Atlas::Rectangle( x,                y, rect.m_width,           h ) );
        m_right = new AtlasNode( Atlas::Rectangle( x + rect.m_width, y, w - rect.m_width, h ) );
    } else {
        m_left  = new AtlasNode( Atlas::Rectangle( x, y,                 w, rect.m_height ) );
        m_right = new AtlasNode( Atlas::Rectangle( x, y + rect.m_height, w, h - rect.m_height ) );
    }
    
    return m_left->place( rect );
}

// ** AtlasNode::isLeaf
bool AtlasNode::isLeaf( void ) const {
    return (m_left == NULL && m_right == NULL);
}

// ** AtlasNode::fits
bool AtlasNode::fits( const Atlas::Rectangle& rect ) const {
    return (rect.m_width <= m_rect.m_width && rect.m_height <= m_rect.m_height);
}

// ** AtlasNode::perfectFit
bool AtlasNode::perfectFit( const Atlas::Rectangle& rect ) const {
    return (rect.m_width == m_rect.m_width && rect.m_height == m_rect.m_height);
}

// ---------------------------------------- Atlas ---------------------------------------- //

// ** Atlas::addRectangle
void Atlas::addRectangle( int width, int height )
{
    m_rectangles.push_back( Rectangle( 0, 0, width, height ) );
}

// ** Atlas::rectangleCount
int Atlas::rectangleCount( void ) const
{
    return ( int )m_rectangles.size();
}

// ** Atlas::rectangle
const Atlas::Rectangle& Atlas::rectangle( int index ) const
{
    assert( index >= 0 && index < rectangleCount() );
    return m_rectangles[index];
}

// ** Atlas::place
bool Atlas::place( int width, int height )
{
    AtlasNode root( Rectangle( 0, 0, width, height ) );

    std::sort( m_rectangles.begin(), m_rectangles.end(), Rectangle::compare );

    for( int i = 0, n = rectangleCount(); i < n; i++ ) {
        if( !root.place( m_rectangles[i] ) ) {
            return false;
        }
    }

    return true;
}

} // namespace relight