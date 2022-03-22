/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_CPP_PROG_UTILS_H_
#define _CS_EDM_CPP_PROG_UTILS_H_

#include <memory>

/*
  Next we introduce a simple class template whose sole job is to execute some code when the object
  goes out of scope. This little utility is useful for when dealing with C-style API’s, since the
  onus is typically on the programmer to remember to release resources, free memory, etc.
  This on_scope_exit object is designed to call a lambda at scope exit, so put any cleanup code you
  want to execute inside of it, and you will be exception-safe!
  Ref: https://bryanstamour.com/post/2017-03-12-sqlite-with-cpp/
 */

template <typename Func> struct on_scope_exit {
    explicit on_scope_exit(Func f) : f_{f} {}
    ~on_scope_exit()
    {
      f_();
    }
  private:
    Func f_;
};


#endif /* _CS_EDM_CPP_PROG_UTILS_H_ */