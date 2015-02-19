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

#include "Yaml.h"
#include <yaml/yaml.h>
#include <math.h>

// ** Yaml::invalid
Yaml Yaml::invalid( Invalid );

// ** Yaml::Yaml
Yaml::Yaml( Type type ) : m_type( type )
{

}

// ** Yaml::Yaml
Yaml::Yaml( const Yaml& other ) : m_type( other.m_type ), m_scalar( other.m_scalar ), m_mapping( other.m_mapping ), m_sequence( other.m_sequence )
{

}

// ** Yaml::Yaml
Yaml::Yaml( const YamlScalar& scalar ) : m_type( Scalar ), m_scalar( scalar )
{

}

// ** Yaml::operator bool
Yaml::operator bool( void ) const
{
    return m_type != Invalid;
}

// ** Yaml::operator[]
const Yaml& Yaml::operator[]( const char* key ) const
{
    assert( m_type == Mapping );
    YamlMapping::const_iterator i = m_mapping.find( key );
    return i != m_mapping.end() ? i->second : invalid;
}

// ** Yaml::operator[]
const Yaml& Yaml::operator[]( const std::string& key ) const
{
    assert( m_type == Mapping );
    YamlMapping::const_iterator i = m_mapping.find( key );
    return i != m_mapping.end() ? i->second : invalid;
}

// ** Yaml::operator[]
const Yaml& Yaml::operator[]( int index ) const
{
    assert( m_type == Sequence );
    return m_sequence[index];
}

// ** Yaml::type
Yaml::Type Yaml::type( void ) const
{
    return m_type;
}

// ** Yaml::keys
YamlKeys Yaml::keys( void ) const
{
    YamlKeys keys;

    for( YamlMapping::const_iterator i = m_mapping.begin(), end = m_mapping.end(); i != end; ++i ) {
        keys.insert( i->first );
    }

    return keys;
}

// ** Yaml::size
int Yaml::size( void ) const
{
    assert( m_type == Sequence );
    return ( int )m_sequence.size();
}

// ** Yaml::number
double Yaml::number( void ) const
{
    assert( m_type == Scalar );
    return m_scalar.number();
}

// ** Yaml::str
const char* Yaml::str( void ) const
{
    assert( m_type == Scalar );
    return m_scalar.str().c_str();
}

// ** Yaml::parse
Yaml Yaml::parse( const char* fileName )
{
    yaml_parser_t yaml;

    FILE* source = fopen( fileName, "rb" );
    if( !source ) {
        return Yaml();
    }

    yaml_parser_initialize( &yaml );
    yaml_parser_set_input_file( &yaml, source );

    Yaml result = YamlParser( &yaml ).first();
    
    yaml_parser_delete( &yaml );
    fclose( source );

    return result;
}

// ** YamlParser::YamlParser
YamlParser::YamlParser( void* yaml ) : m_yaml( yaml )
{

}

// ** YamlParser::first
Yaml YamlParser::first( void )
{
    return parse()[0];
}

// ** YamlParser::all
YamlDocuments YamlParser::all( void )
{
    return parse();
}

// ** YamlParser::next
void YamlParser::next( void )
{
    yaml_event_t event;
    yaml_parser_parse( reinterpret_cast<yaml_parser_t*>( m_yaml ), &event );

    m_current.m_type  = event.type;
    m_current.m_value = event.type == YAML_SCALAR_EVENT ? ( const char* )event.data.scalar.value : "";
}

// ** YamlParser::parseScalar
Yaml YamlParser::parseScalar( void )
{
    std::string value = expect( YAML_SCALAR_EVENT );

    double number = atof( value.c_str() );
    if( number == 0.0 && (value.length() != 1 && value[0] != '0') ) {
        return YamlScalar( value );
    }

    return YamlScalar( number );
}

// ** YamlParser::parseMapping
Yaml YamlParser::parseMapping( void )
{
    Yaml result( Yaml::Mapping );

    expect( YAML_MAPPING_START_EVENT );
    while( !check( YAML_MAPPING_END_EVENT ) ) {
        std::string key = expect( YAML_SCALAR_EVENT );
        result.m_mapping[key] = parseNode();
    }
    expect( YAML_MAPPING_END_EVENT );

    return result;
}

// ** YamlParser::parseSequence
Yaml YamlParser::parseSequence( void )
{
    Yaml result( Yaml::Sequence );

    expect( YAML_SEQUENCE_START_EVENT );
    while( !check( YAML_SEQUENCE_END_EVENT ) ) {
        result.m_sequence.push_back( parseNode() );
    }
    expect( YAML_SEQUENCE_END_EVENT );

    return result;
}

// ** YamlParser::parseDocument
Yaml YamlParser::parseDocument( void )
{
    Yaml result;

    expect( YAML_DOCUMENT_START_EVENT );
    assert( check( YAML_MAPPING_START_EVENT ) );

    result = parseMapping();

    expect( YAML_DOCUMENT_END_EVENT );

    return result;
}

// ** YamlParser::check
bool YamlParser::check( int type ) const
{
    return m_current.m_type == type;
}

// ** YamlParser::expect
std::string YamlParser::expect( int type )
{
    assert( type == m_current.m_type );
    std::string value = m_current.m_value;
    next();
    return value;
}

// ** YamlParser::parseNode
Yaml YamlParser::parseNode( void )
{
    switch( m_current.m_type ) {
    case YAML_MAPPING_START_EVENT:  return parseMapping();
    case YAML_SEQUENCE_START_EVENT: return parseSequence();
    case YAML_SCALAR_EVENT:         return parseScalar();
    }

    assert( false );

    return Yaml();
}

// ** YamlParser::parse
YamlDocuments YamlParser::parse( void )
{
    YamlDocuments documents;

    next();
    expect( YAML_STREAM_START_EVENT );

    while( check( YAML_DOCUMENT_START_EVENT ) ) {
        documents.push_back( parseDocument() );
    }

    expect( YAML_STREAM_END_EVENT );

    return documents;
}
