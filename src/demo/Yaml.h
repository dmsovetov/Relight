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

#ifndef __demo__Yaml__
#define __demo__Yaml__

#include "Value.h"
#include <map>
#include <vector>
#include <string>
#include <stack>
#include <set>

//! Yaml scalar type.
typedef Value                               YamlScalar;

//! Yaml mapping type.
typedef std::map<std::string, class Yaml>   YamlMapping;

//! Yaml sequence type.
typedef std::vector<class Yaml>             YamlSequence;

//! Yaml documents array type.
typedef std::vector<class Yaml>             YamlDocuments;

//! Yaml keys type.
typedef std::set<std::string>               YamlKeys;

class YamlParser;

//! Yaml object
class Yaml {
friend class YamlParser;
public:

    //! Node type enumeration
    enum Type {
        Invalid,    //!< Invalid Yaml node.
        Scalar,     //!< Scalar value.
        Mapping,    //!< Associative array.
        Sequence    //!< Array of values.
    };

                            //! Constructs a Yaml node.
                            Yaml( Type type = Invalid );

                            //! Copies a Yaml node.
                            Yaml( const Yaml& other );

    //! Checks if this node is valid.
    operator bool( void ) const;

    //! Returns a mapping property value.
    const Yaml&             operator[]( const char* key ) const;

    //! Returns a mapping property value.
    const Yaml&             operator[]( const std::string& key ) const;

    //! Returns a list element.
    const Yaml&             operator[]( int index ) const;

    //! Returns a first value in mapping.
    YamlKeys                keys( void ) const;

    //! Returns node size.
    int                     size( void ) const;

    //! Returns a node type.
    Type                    type( void ) const;

    //! Returns a number value.
    double                  number( void ) const;

    //! Returns a string value.
    const char*             str( void ) const;

    //! Parses a Yaml object from file.
    static Yaml             parse( const char* fileName );

private:

                            //! Constructs a Yaml scalar node.
                            Yaml( const YamlScalar& scalar );

private:

    //! Invalid Yaml node instance.
    static Yaml             invalid;

    //! Yaml node type.
    Type                    m_type;

    //! Yaml scalar value.
    YamlScalar              m_scalar;

    //! Yaml mapping value.
    YamlMapping             m_mapping;

    //! Yaml sequence value.
    YamlSequence            m_sequence;
};

//! Internal Yaml parser state.
class YamlParser {
public:

                            //! Constructs Yaml parser.
                            YamlParser( void* yaml );

    //! Parses all documents from a stream.
    YamlDocuments           all( void );

    //! Parses all documents from a stream, but returns only the first one.
    Yaml                    first( void );

private:

    //! Parses a next token from stream.
    void                    next( void );

    //! Checks that current token matches the type.
    bool                    check( int type ) const;

    //! Expects a token in stream of a given type than goes to a next one.
    std::string             expect( int type );

    //! Parses documents.
    YamlDocuments           parse( void );

    //! Parses a Yaml node.
    Yaml                    parseNode( void );

    //! Parses a Yaml scalar node.
    Yaml                    parseScalar( void );

    //! Parses a maping.
    Yaml                    parseMapping( void );

    //! Parses a sequence.
    Yaml                    parseSequence( void );

    //! Parses a document.
    Yaml                    parseDocument( void );

private:

    //! Yaml parser pointer.
    void*                   m_yaml;

    //! Yaml token struct.
    struct {
        int                 m_type;     //!< Token type.
        std::string         m_value;    //!< Token value.
    } m_current;
};

#endif /* defined(__demo__Yaml__) */
