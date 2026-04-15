#pragma once

#include <cstdint>

#define ANVIL_NO_COPY_NO_MOVE( Class )                                          \
  Class( const Class & )             = delete;                                  \
  Class & operator=( const Class & ) = delete;                                  \
  Class( Class && )                  = delete;                                  \
  Class & operator=( Class && )      = delete

// Fixed-width types
using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

using c8  = char;
using c16 = wchar_t;

using usize = std::size_t;
