/*
 * cpp_util_3
 */

/*!
 * \file
 * \since
 * v.2.3.0
 *
 * \brief Tools for optimized dumps of std::string in hex format.
 */

#pragma once

#if defined( _MSC_VER )
	#if !defined( _CRT_SECURE_NO_WARNINGS )
		#define _CRT_SECURE_NO_WARNINGS
		#define CPP_UTIL_2_MSC_CRT_SECURE_NO_WARNINGS_ON
	#endif
#endif

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include <cpp_util_3/string_piece.hpp>

namespace cpp_util_3 {

namespace hex_dumps {

/*!
 * \since
 * v.2.3.0
 *
 * \brief Helper class for dumping string value to ostream.
 *
 * Usage example:
 * \code
 * std::string s( "hello, world" );
 * std::cout << cpp_util_3::hex_dumps::string_dumper( s ) << std::endl;
 * \endcode
 *
 * \par v.2.3.1
 * Since v.2.3.1 uses string_piece_t. This allows to dump raw C-strings
 * without transforming them into std::string.
 */
class	string_dumper
	{
	public :
		string_dumper( const string_piece_t & s )
			:	m_what( s )
			{}

		const string_piece_t &
		what() const { return m_what; }

	private :
		string_piece_t	m_what;
	};

/*!
 * \since
 * v.2.3.0
 *
 * \brief Actual implementation of string dumper.
 *
 * \note Allocates memory for temporary buffer.
 */
inline std::ostream &
operator<<( std::ostream & to, const string_dumper & s )
	{
		const string_piece_t & what = s.what();
		const size_t need_space = what.size() * 3;
		if( need_space )
			{
				std::vector< std::string::value_type > buf( need_space, 0 );

				std::string::value_type * pos = &buf[ 0 ];
				for( const char * p = what.data(),
						* p_end = what.data() + what.size();
						p != p_end; )
					{
						auto l = std::sprintf( pos, "%02x",
								static_cast< unsigned int >(
										static_cast< unsigned char >( *p ) ) );
						pos += l;
						if( (++p) != p_end )
							*(pos++) = ' ';
					}
				to << &buf[ 0 ];
			}

		return to;
	}

/*!
 * \since
 * v.2.3.1
 *
 * \brief Another string dumper with hex-escaped for non-printable symbols.
 *
 * Symbols with codes 0..31 are dumped as hex-escaped sequences
 * (like \\x00, \\x01 and so on).
 *
 * Usage example:
 * \code
 * std::string s( "hello, world" );
 * std::cout << cpp_util_3::hex_dumps::hex_escaped_string_dumper( s )
 * 		<< std::endl;
 * \endcode
 */
class	hex_escaped_string_dumper
	{
	public :
		hex_escaped_string_dumper( const string_piece_t & s )
			:	m_what( s )
			{}

		const string_piece_t &
		what() const { return m_what; }

	private :
		string_piece_t	m_what;
	};

/*!
 * \since
 * v.2.3.1
 *
 * \note Allocates memory for temporary buffer.
 */
inline std::ostream &
operator<<( std::ostream & to, const hex_escaped_string_dumper & s )
	{
		const string_piece_t & what = s.what();
		// Allocate memory for worst case.
		const size_t need_space = what.size() * 4;
		if( need_space )
			{
				std::vector< char > buf( need_space + 1, 0 );

				char * pos = &buf[ 0 ];
				for( const char * p = what.data(),
						* end = what.data() + what.size();
						p != end;
						++p )
					{
						if( 0 <= *p && *p < ' ' )
							{
								auto l = std::sprintf( pos, "\\x%02x",
										static_cast< unsigned int >(
												static_cast< unsigned char >( *p ) ) );
								pos += l;
							}
						else
							{
								*pos = *p;
								++pos;
							}
					}
				to << &buf[ 0 ];
			}

		return to;
	}

/*!
 * \since
 * v.2.6.0
 *
 * \brief Helper class for dumping hex representation without spaces.
 *
 * Usage example:
 * \code
 * std::string s( "hello, world" );
 * std::cout << cpp_util_3::hex_dumps::string_dumper_without_spaces( s )
 * 		<< std::endl;
 * \endcode
 */
class	string_dumper_without_spaces
	{
	public :
		string_dumper_without_spaces( const string_piece_t & s )
			:	m_what( s )
			{}

		const string_piece_t &
		what() const { return m_what; }

	private :
		string_piece_t	m_what;
	};

/*!
 * \since
 * v.2.6.0
 *
 * \note Allocates memory for temporary buffer.
 */
inline std::ostream &
operator<<( std::ostream & to, const string_dumper_without_spaces & s )
	{
		const string_piece_t & what = s.what();
		const size_t need_space = what.size() * 2;
		if( need_space )
			{
				std::vector< std::string::value_type > buf( need_space + 1, 0 );

				std::string::value_type * pos = &buf[ 0 ];
				for( const char * p = what.data(),
						* p_end = what.data() + what.size();
						p != p_end;
						++p )
					{
						auto l = std::sprintf( pos, "%02x",
								static_cast< unsigned int >(
										static_cast< unsigned char >( *p ) ) );
						pos += l;
					}
				to << &buf[ 0 ];
			}

		return to;
	}

} /* namespace hex_dumps */

} /* namespace cpp_util_3 */

#if defined( CPP_UTIL_2_MSC_CRT_SECURE_NO_WARNINGS_ON )
	#undef CPP_UTIL_2_MSC_CRT_SECURE_NO_WARNINGS_ON
	#undef _CRT_SECURE_NO_WARNINGS
#endif

