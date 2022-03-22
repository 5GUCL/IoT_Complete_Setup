/*!
	\since
	v.2.0.2

	\file
	\brief %lexcast and %slexcast functions family.
*/

#pragma once

#include <sstream>

namespace	cpp_util_3
{

/*!
	\since
	v.2.0.2

	\brief A very simple alternative of boost::lexical_cast.

	Usage example:
\code
const std::string &
my_string_vector_t::operator[]( unsigned int index ) const
{
	if( index >= size() )
		throw std::out_of_range(
			std::string( "index is too big: " ) +
			cpp_util_3::lexcast< std::string >( index ) );
	...
}
\endcode
	Or more simple way:
\code
const std::string &
my_string_vector_t::operator[]( unsigned int index ) const
{
	if( index >= size() )
		throw std::out_of_range(
			std::string( "index is too big: " ) +
			cpp_util_3::slexcast( index ) );
	...
}
\endcode

	\note Allows to convert numbers to strings. And other way: strings
	to numbers.

	\par Exceptions
	Function lexcast() do not throw any exception by itselt.
	It also do not control the success of value transformations.
	But these transformation can raise some exceptions: lexcast() do
	not catch them.

	\par Requirements for type From.
	There should be operator<<() for std::ostream.

	\par Requirements for type To.
	There should be operator>>() for std::istream.
	Type \c To must be DefaultConstructible.

	\sa cpp_util_3::slexcast()
*/
template< class To, class From >
To
lexcast(
	//! Value to transform.
	const From & f )
{
	std::stringstream	ss;
	ss << f;
	To t;
	ss.seekg( 0 );
	ss >> t;

	return t;
}

/*!
	\since
	v.2.1.0

	The same as lexcast() but uses \a putter for storing \a f
	into temporary ostream.

	Type \a Putter must have operator() with the following format:
\code
void
operator( std::ostream & to, const From & what ) const;
\endcode
*/
template< class To, class From, class Putter >
To
lexcast(
	//! What to transform.
	const From & f,
	//! How to transform.
	const Putter & putter )
{
	std::stringstream	ss;
	putter( ss, f );
	To t;
	ss.seekg( 0 );
	ss >> t;

	return t;
}

/*!
	\since v.2.1.0
	\brief A version of %lexcast() with custom putter and getter objects.

	Usage example:
\code
// Convert number from hex.
int i = cpp_util_3::lexcast< int >( "ffab",
	cpp_util_3::lexcasts::def_putter(),
	cpp_util_3::lexcasts::hex_getter() );
\endcode

	Type \a Putter must have operator() with the following format:
\code
void
operator( std::ostream & to, const From & what ) const;
\endcode

	Type \a Getter must have operator() with the following format:
\code
void
operator( std::istream & to, To & what ) const;
\endcode
*/
template< class To, class From, class Putter, class Getter >
To
lexcast(
	//! What to transform.
	const From & f,
	//! How to put source value into std::ostream.
	const Putter & putter,
	//! How to get result value from std::istream.
	const Getter & getter )
{
	std::stringstream	ss;
	putter( ss, f );
	To t;
	ss.seekg( 0 );
	getter( ss, t );

	return t;
}

/*!
	\since v.2.1.0
	\brief A variant of %lexcast for transforming value into string.

	Usage example:
\code
class	A
{
	int	m_a;
	float	m_b;
	public :
		std::ostream &
		dump_to( std::ostream & to ) const
		{
			return ( to << "m_a: " << a << ", m_b: " << m_b );
		}
};
inline std::ostream &
operator<<( std::ostream & to, const A & a )
{
	return a.dump_to( to );
}

void f()
{
	A a;
	std::string str_a( cpp_util_3::slexcast( a ) );
}
\endcode
*/
template< class From >
std::string
slexcast(
	//! What to transform.
	const From & f )
{
	std::stringstream	ss;
	ss << f;

	return ss.str();
}

/*!
	\since v.2.1.0
	\brief A special form of %slexcast with custom putter into ostream.

	Type \a Putter must have operator() with the following format:
\code
void
operator( std::ostream & to, const From & what ) const;
\endcode
*/
template< class From, class Putter >
std::string
slexcast(
	//! What to transform.
	const From & f,
	//! How to put source value into stream.
	const Putter & putter )
{
	std::stringstream	ss;
	putter( ss, f );

	return ss.str();
}

} /* namespace cpp_util_3 */

