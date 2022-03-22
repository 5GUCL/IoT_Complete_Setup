/*!
	\file
	\brief Compiler-dependent macros.

	Several macros with same names but with different content
	for different compilers. These macros intended to cope with
	non-standard compiler extensions.
*/

#pragma once

/*!
	\def CPP_UTIL_3_EXPORT
	\brief A specifier for exported entity.

	This specifier must be used for function or class which is
	exported from dinamic-link library.
*/

/*!
	\def CPP_UTIL_3_IMPORT
	\brief A specifier for imported entity.

	This specifier must be used for function or class which is
	imported from dinamic-link library.
*/

/*!
	\def CPP_UTIL_3_WIN32
	\brief A marker for Win32 platform.

	\note This marker is also defined for Win64 platform.
*/

/*!
	\def CPP_UTIL_3_WIN64
	\brief A marker for Win64 platform.
*/

/*!
 * \def CPP_UTIL_3_WINDOWS
 * \brief A marker for Windows platform.
 *
 * Defined if CPP_UTIL_3_WIN32 or CPP_UTIL_3_WIN64 are defined.
 */

/*!
 * \def CPP_UTIL_3_UNIX
 * \brief A marker for Unix platforms.
 *
 * \since
 * v.3.0.0
 */

/*!
	\def CPP_UTIL_3_WIN32_DLL_ENTRY_POINT
	\brief Name for Dll-entry poing.

	Optional entry point function in Win32-DLL has different
	names in different compilers:

	\par Visual C++
	DllMain

	\par Borland C++
	DllEntryPoint

	\par Watcom C++
	LibMain
*/

/*
	Delete compilers differences
*/
#if defined( _MSC_VER )
/*
** Visual C++
*/
#define CPP_UTIL_3_EXPORT __declspec(dllexport)
#define CPP_UTIL_3_IMPORT __declspec(dllimport)

#if defined( _WIN64 )
 # define CPP_UTIL_3_WIN64
#endif

#if defined( _WIN32 )

#define CPP_UTIL_3_WIN32

#define CPP_UTIL_3_WIN32_DLL_ENTRY_POINT DllMain

#endif /* defined _WIN32 */

#endif /* _MSC_VER */

#if defined( __MINGW32__ ) || defined( __WINGW64__ )
/*
** MinGW C++
*/
#define CPP_UTIL_3_EXPORT __declspec(dllexport)
#define CPP_UTIL_3_IMPORT __declspec(dllimport)

#if defined( __WIN64__ )

#define CPP_UTIL_3_WIN64

#endif

#if defined( __WIN32__ )

#define CPP_UTIL_3_WIN32

#define CPP_UTIL_3_WIN32_DLL_ENTRY_POINT DllMain

#endif /* defined __WIN32__ */

#endif /* __MINGW32__ */


#if defined( __WATCOMC__ )
/*
** WATCOM C++
*/
#define CPP_UTIL_3_EXPORT _export
#define CPP_UTIL_3_IMPORT

#if defined( __NT__)

#define CPP_UTIL_3_WIN32

#define CPP_UTIL_3_WIN32_DLL_ENTRY_POINT LibMain

#endif /* __NT__ */

#endif /* __WATCOMC__ */


#if defined( __BORLANDC__ )
/*
** Borland C++
*/
#define CPP_UTIL_3_EXPORT _export
#define CPP_UTIL_3_IMPORT 

#if defined( __WIN32__ )

#define CPP_UTIL_3_WIN32

#define CPP_UTIL_3_WIN32_DLL_ENTRY_POINT DllEntryPoint

#endif /* __WIN32__ */

#endif /* __BORLANDC__ */


#if defined( __TANDEM )
/*
** Tandem C++
*/
#define CPP_UTIL_3_EXPORT /*export$*/
#define CPP_UTIL_3_IMPORT /*import$*/

#define CPP_UTIL_3_TANDEM

#endif /* __TANDEM */


#if !defined( CPP_UTIL_3_EXPORT )
#define CPP_UTIL_3_EXPORT
#endif

#if !defined( CPP_UTIL_3_IMPORT )
#define CPP_UTIL_3_IMPORT
#endif

#if defined( CPP_UTIL_3_WIN32 ) || defined( CPP_UTIL_3_WIN64 )
#define CPP_UTIL_3_WINDOWS
#endif

#if defined(unix) || defined(__unix__) || defined(__unix)
#define CPP_UTIL_3_UNIX
#endif

