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

#ifndef __demo__Value__
#define __demo__Value__

#include <string>
#include <assert.h>

//! The Value class acts like a union for the primitive data types.
class Value {

    //! Data type enumeration.
    enum Type {
        Void,   //!< Uninitialized value.
        Number, //!< Number value.
        String, //!< String value.
    };

public:

                        //! Constructs a void value.
                        Value( void );

                        //! Constructs a string value.
                        Value( const char* value );

                        //! Constructs a string value.
                        Value( const std::string& value );

                        //! Constructs a number value.
                        Value( double value );

                        //! Copies the value.
                        Value( const Value& other );

    //! Returns a numeric value.
    double              number( void ) const;

    //! Returns a string value.
    const std::string&  str( void ) const;

private:

    //! Data type.
    Type                m_type;

    //! Holds number.
    double              m_number;

    //! Holds string.
    std::string         m_string;
};

// ** Value::Value
inline Value::Value( void ) : m_type( Void ) {

}

// ** Value::Value
inline Value::Value( const Value& other ) : m_type( other.m_type ), m_number( other.m_number ), m_string( other.m_string ) {

}

// ** Value::Value
inline Value::Value( const char* value ) : m_type( String ), m_string( value ) {

}

// ** Value::Value
inline Value::Value( const std::string& value ) : m_type( String ), m_string( value ) {

}

// ** Value::Value
inline Value::Value( double value ) : m_type( Number ), m_number( value ) {

}

// ** Value::number
inline double Value::number( void ) const {
    assert( m_type == Number );
    return m_number;
}

// ** Value::str
inline const std::string& Value::str( void ) const {
    assert( m_type == String );
    return m_string;
}

#endif /* defined(__demo__Value__) */
