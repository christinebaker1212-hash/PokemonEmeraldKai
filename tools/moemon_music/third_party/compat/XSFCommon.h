// Moemon Music Studio: the one helper from in_xsf's XSFCommon.h the cores use.
#pragma once
#include <cstdint>
#include <cstring>
#include <cmath>
#include <string>
#include <limits>
#include <type_traits>
template<typename T> inline typename std::enable_if_t<std::is_floating_point_v<T>, bool> fEqual(T x, T y, int N = 1)
{ T diff = std::abs(x - y); T tol = N * std::numeric_limits<T>::epsilon(); return diff <= tol * std::abs(x) && diff <= tol * std::abs(y); }
