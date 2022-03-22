/*!
	\file
	\brief Macros which are not compiler-dependent.
*/

#pragma once

/*!
	\def CPP_UTIL_2_ASIZE(array)

	Returns size of \a array in items.

	\note This macro exists from pre C++98 times. More modern and
	robust alternatives must be used now.
*/
#define CPP_UTIL_2_ASIZE(array) (sizeof(array)/sizeof(array[0]))

/*!
	\def CPP_UTIL_2_SIZEOFATTR( t, a )
	Returns size of field \a attr from type \a type in bytes.
*/
#define CPP_UTIL_2_SIZEOFATTR( t, a ) (sizeof(((t *)0)->a))

