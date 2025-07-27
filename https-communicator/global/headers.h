

#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <cmath>
#include <algorithm>
#include <thread>
#include <format>
#include <print>
#include <expected>

#include <asio.hpp>

#ifdef DEBUG
constexpr bool DEBUG_STATUS = true;
#elif defined(NDEBUG)
constexpr bool DEBUG_STATUS = false;
#else
constexpr bool DEBUG_STATUS = false;
#endif

#define DEBUG_LN if (DEBUG_STATUS)

#define DEBUG(x) if (DEBUG_STATUS) { x }

