/*!
	\since
	v.2.0.3

	\file
	\brief Various helpers for %lexcast and %slexcast.
*/

#pragma once

#include <cpp_util_3/lexcast.hpp>
#include <cpp_util_3/at_scope_exit.hpp>

#include <iterator>

namespace	cpp_util_3
{

namespace	lexcasts
{

namespace impl
{
	template< class T >
	inline void
	ostream_lshift( std::ostream & o, const T & v )
	{
		o << v;
	}

	template<>
	inline void
	ostream_lshift( std::ostream & to, const char & v )
	{
		ostream_lshift( to, static_cast< unsigned short >(
			static_cast< unsigned char >( v ) ) );
	}

	template<>
	inline void
	ostream_lshift( std::ostream & to, const signed char & v )
	{
		ostream_lshift( to, static_cast< unsigned short >(
			static_cast< unsigned char >( v ) ) );
	}

	template<>
	inline void
	ostream_lshift( std::ostream & to, const unsigned char & v )
	{
		ostream_lshift( to, static_cast< unsigned short >( v ) );
	}

	template< class T >
	inline void
	istream_rshift( std::istream & o, T & v )
	{
		o >> v;
	}

	template<>
	inline void
	istream_rshift( std::istream & from, char & v )
	{
		unsigned short s;
		istream_rshift( from, s );
		v = static_cast< char >( s );
	}

	template<>
	inline void
	istream_rshift( std::istream & from, signed char & v )
	{
		unsigned short s;
		istream_rshift( from, s );
		v = static_cast< signed char >( s );
	}

	template<>
	inline void
	istream_rshift( std::istream & from, unsigned char & v )
	{
		unsigned short s;
		istream_rshift( from, s );
		v = static_cast< unsigned char >( s );
	}

	template<>
	inline void
	istream_rshift( std::istream & from, short & v )
	{
		unsigned short s;
		istream_rshift( from, s );
		v = static_cast< short >( s );
	}

	template<>
	inline void
	istream_rshift( std::istream & from, int & v )
	{
		unsigned int s;
		istream_rshift( from, s );
		v = static_cast< int >( s );
	}

	template<>
	inline void
	istream_rshift( std::istream & from, long & v )
	{
		unsigned long s;
		istream_rshift( from, s );
		v = static_cast< long >( s );
	}
}

//
// def_putter
//

//! Default method of storing value into ostream.
struct	def_putter
{
	template< class T >
	void
	operator()( std::ostream & o, const T & v ) const
	{
		o << v;
	}
};

//
// hex
//

/*!
	\since
	v.2.0.3

	Usage example:
\code
unsigned int ui = 2378;
std::string str_hex_ui(
	cpp_util_3::slexcast( ui, cpp_util_3::lexcasts::hex() ) );
\endcode

*/
struct hex
{
	template< class T >
	void
	operator()( std::ostream & to, const T & v ) const
	{
		std::ios::fmtflags old = to.setf(
			std::ios::hex,
			std::ios::basefield );
		auto setf = at_scope_exit( [&]{ to.setf( old ); } );

		impl::ostream_lshift( to, v );
	}
};

//
// prefixed_hex
//

/*!
	\since
	v.2.1.0

	Usage example:
\code
unsigned int ui = 2378;
std::string str_hex_ui(
	cpp_util_3::slexcast( ui, cpp_util_3::lexcasts::prefixed_hex( "hex:" ) ) );
\endcode

*/
class prefixed_hex
{
	private :
		std::string	m_prefix;

	public :
		prefixed_hex( const std::string & prefix )
		:	m_prefix( prefix )
		{}

		template< class T >
		void
		operator()( std::ostream & to, const T & v ) const
		{
			to << m_prefix;
			std::ios::fmtflags old = to.setf(
				std::ios::hex,
				std::ios::basefield );
			auto setf = at_scope_exit( [&]{ to.setf( old ); } );

			impl::ostream_lshift( to, v );
		}
};

//
// hex_0x
//

/*!
	\since v.2.1.0

	Produces hex value with 0x prefix (e.g. 0x01, 0x0001 and so on).

	Usage example:
\code
unsigned int ui = 2378;
std::string str_hex_ui( cpp_util_3::slexcast( ui,
	cpp_util_3::lexcasts::hex_0x() ) );
\endcode
*/
class	hex_0x :	public prefixed_hex
{
	public :
		hex_0x() : prefixed_hex( "0x" ) {}
};

//
// hex_bslashx
//

/*!
	\since v.2.1.0

	Produces hex value with \\x prefix (e.g. \\x01, \\x0001 and so on).

	Usage example:
\code
unsigned int ui = 2378;
std::string str_hex_ui( cpp_util_3::slexcast( ui,
	cpp_util_3::lexcasts::hex_bslashx() ) );
\endcode

*/
class	hex_bslashx : public prefixed_hex
{
	public :
		hex_bslashx() : prefixed_hex( "\\x" ) {}
};

//
// hex_getter
//

/*!
	\since
	v.2.0.3

	Gets value from hex representation.

	Usage example:
\code
unsigned int ui = cpp_util_3::lexcast< unsigned int >(
	std::string( "ff5d" ),
	cpp_util_3::lexcasts::def_putter(),
	cpp_util_3::lexcasts::hex_getter() );
\endcode

*/
struct hex_getter
{
	template< class T >
	void
	operator()( std::istream & to, T & v ) const
	{
		std::ios::fmtflags old = to.setf(
			std::ios::hex,
			std::ios::basefield );
		auto setf = at_scope_exit( [&]{ to.setf( old ); } );

		impl::istream_rshift( to, v );
	}
};

//
// all_t
//

/*!
	Helper for dumping all values between two iterators.

	\since
	v.2.0.3

	\sa	cpp_util_3::lexcasts::all().
*/
template< class It >
struct	all_t
{
	//! Begin.
	It m_b;
	//! End.
	It m_e;
	//! Values separator.
	const std::string & m_sep;

	/*!
	 * \note The reference to \a sep must remains valid for all
	 * time while this object live.
	 */
	all_t( It b, It e, const std::string & sep )
	:	m_b( b )
	,	m_e( e )
	,	m_sep( sep )
	{}
};

template< class It >
std::ostream &
operator<<( std::ostream & to, const all_t< It > & what )
{
	It b( what.m_b ), e( what.m_e );
	while( b != e )
	{
		to << *b;
		if( (++b) != e )
			to << what.m_sep;
	}
	return to;
}

//
// all
//

/*!
	\since
	v.2.0.3

	Usage example:
\code
// Print all values from array (with space as a separator).
unsigned int ui[ 64 ] = { ... };
std::cout << cpp_util_3::lexcasts::all( std::begin(ui), std::end(ui) );

// Print all values from array with new line as separator.
std::set< std::string >	ss;
...
std::cout << cpp_util_3::lexcasts::all( ss.begin(), ss.end(), "\n" );
\endcode

	\note Separator is not printed after last item.
*/
template< class It >
all_t< It >
all( It b, It e, const std::string & sep = " " )
{
	return all_t< It >( b, e, sep );
}

//
// all_cont_t
//

/*!
	\since
	v.2.0.3

	\sa	cpp_util_3::lexcasts::all().
*/
template< class Cont >
struct	all_cont_t
{
	const Cont &	m_c;
	const std::string & m_sep;

	all_cont_t( const Cont & c, const std::string & sep )
	:	m_c( c )
	,	m_sep( sep )
	{}
};

/*!
 * \since
 * v.2.0.3
 *
 * \note Separator is not printed after last item.
 */
template< class Cont >
std::ostream &
operator<<( std::ostream & to, const all_cont_t< Cont > & what )
{
	auto b = std::begin(what.m_c);
	auto e = std::end(what.m_c);
	while( b != e )
	{
		to << *b;
		if( (++b) != e )
			to << what.m_sep;
	}
	return to;
}

//! Put all values from a container into ostream.
/*!
	\since v.2.0.3

	Usage example:
\code
std::set< std::string >	ss;
...
std::cout << cpp_util_3::lexcasts::all( ss, "\n" ) );
\endcode

	\note Separator is not printed after last item.
*/
template< class Cont >
all_cont_t< Cont >
all( const Cont & c, const std::string & sep = " " )
{
	return all_cont_t< Cont >( c, sep );
}

//
// all_using_t
//

/*!
	\since
	v.2.0.3

	\sa	cpp_util_3::lexcasts::all().
*/
template< class It, class Putter >
struct	all_using_t
{
	It m_b;
	It m_e;
	const std::string & m_sep;
	Putter	m_putter;

	all_using_t( It b, It e, const std::string & sep, Putter putter )
	:	m_b( b )
	,	m_e( e )
	,	m_sep( sep )
	,	m_putter( putter )
	{}
};

template< class It, class Putter >
std::ostream &
operator<<( std::ostream & to, const all_using_t< It, Putter > & what )
{
	It b( what.m_b ), e( what.m_e );
	while( b != e )
	{
		what.m_putter( to, *b );
		if( (++b) != e )
			to << what.m_sep;
	}
	return to;
}

//
// all_using
//

//! Put all values between two iterators into ostream.
/*!
	\since
	v.2.0.3

	Usage example:
\code
unsigned int ui[ 64 ] = { ... };
std::cout << cpp_util_3::lexcasts::all( std::begin(ui), std::end(ui),
		" ", cpp_util_3::lexcasts::hex_0x() );
\endcode

	\note Separator is not printed after last item.
*/
template< class It, class Putter >
all_using_t< It, Putter >
all( It b, It e, const std::string & sep, Putter putter )
{
	return all_using_t< It, Putter >( b, e, sep, putter );
}

//! Put all values from a container into ostream.
/*!
	\since
	v.2.0.3

	Usage example:
\code
// Print hex value of every string symbol as hex-escape sequence.
std::string hello( "Hello!" );
std::cout << cpp_util_3::lexcasts::all( hello, "",
		cpp_util_3::lexcasts::hex_bslashx() ) );
\endcode

	\note Separator is not printed after last item.
*/
template< class Cont, class Putter >
all_using_t< typename Cont::const_iterator, Putter >
all( const Cont &c, const std::string & sep, Putter putter )
{
	return all_using_t< typename Cont::const_iterator, Putter >(
		c.begin(), c.end(), sep, putter );
}

} /* namespace lexcasts */

} /* namespace cpp_util_3 */

