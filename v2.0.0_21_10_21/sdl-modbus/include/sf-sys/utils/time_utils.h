/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#ifndef _CS_EDM_TIME_UTILS_H_
#define _CS_EDM_TIME_UTILS_H_

#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>

/* Begin of the 'extern "C"' block */
#ifdef __cplusplus
extern "C" {
#endif

class CTimeUtils
{
  public:
    CTimeUtils();
    ~CTimeUtils();
    long long getTimestampInMilliSeconds();
    long getTimestampInMilliSecondsAfter24Hours();
    bool checkTimestampExpiryInMilliSeconds(long t0);

    static std::string formatTime2Readable(long ft);

    std::string readable_ts_now();
};

#ifdef __cplusplus
} /* end of the 'extern "C"' block */
#endif

#endif /* _CS_EDM_TIME_UTILS_H_ */