/*==========================LICENSE NOTICE==========================*/
/*
 * Copyright (c) 2021 Vidcentum R&D Pvt Ltd, India.
 * License: Refer to LICENSE file of the software package.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
*/
/*========================END LICENSE NOTICE========================*/

#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include "sf-sys/utils/time_utils.h"


CTimeUtils::CTimeUtils()
{

}

CTimeUtils::~CTimeUtils()
{

}

long long CTimeUtils::getTimestampInMilliSeconds()
{

  auto now = std::chrono::system_clock::now();
  auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);

  auto value = now_ms.time_since_epoch();
  auto duration = value.count();

  return duration;
}

long CTimeUtils::getTimestampInMilliSecondsAfter24Hours()
{

  auto now = std::chrono::system_clock::now();
  auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);

  auto value = now_ms.time_since_epoch() + std::chrono::hours(24);
  auto duration = value.count();

  return duration;
}

// returns true if current time is less than t0 (expiry_at)
// returns false if current time is past the t0 (expiry_at)
bool CTimeUtils::checkTimestampExpiryInMilliSeconds(long t0)
{

  auto now = std::chrono::system_clock::now();
  auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);

  std::chrono::milliseconds future(t0);
  std::chrono::time_point<std::chrono::system_clock> expiry_at(future);

  if(expiry_at > now_ms) {
    return true;
  }
  else {
    return false;
  }
}

std::string CTimeUtils::formatTime2Readable(long ft)
{
  std::chrono::milliseconds givenTime(ft);
  std::chrono::time_point<std::chrono::system_clock> givenTimePoint(givenTime);
  std::time_t now_ft = std::chrono::system_clock::to_time_t(givenTimePoint);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&now_ft), "%F %T %Z");

  return ss.str();
}


std::string CTimeUtils::readable_ts_now()
{

  auto now = std::chrono::system_clock::now();
  auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
  std::chrono::time_point<std::chrono::system_clock> givenTimePoint(now_ms);
  std::time_t now_ft = std::chrono::system_clock::to_time_t(givenTimePoint);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&now_ft), "%Y_%m_%d_%H_%M_%S_") << now_ms.time_since_epoch().count();

  return ss.str();
}