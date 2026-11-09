#pragma once

#include <array>
#include <bit>
#include <concepts>

#include <w/common/stdint.h>
#include <w/common/bit.h>
#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief The results of a pure function for the arguments it was asked for last, `Size` of them,
 * each in the one place the hash of its arguments picks: a newer key of the same place takes it
 * over.
 *
 * Keys are the bit patterns of the arguments (bits_of()), so a hit returns exactly what the
 * function would, signed zeros included. Not synchronized: keep one per thread, e.g. for a lookup
 * that several systems of a step ask for the same point.
 *
 * @tparam Words 64-bit words of a key.
 * @tparam Size Places, a power of two above one.
 */
template <usize Words, typename Value, usize Size>
  requires(
    Words > 0 and Size > 1 and std::has_single_bit(Size) and std::default_initializable<Value>
  )
class Memo
{
 public:
  using Key = std::array<u64, Words>;

  /**
   * @brief The value kept for `key`, null if its place holds another key or none.
   */
  [[nodiscard]] Value const* find(Key const& key) const noexcept
  {
    // place_of() masks the place below Size
    // NOLINTNEXTLINE(*-constant-array-index,*-avoid-unchecked-container-access)
    auto const& entry = this->m_entries[place_of(key)];
    return entry.used and entry.key == key ? &entry.value : nullptr;
  }

  /**
   * @brief Keeps `value` for `key`, in place of whatever its place held; the value kept.
   */
  Value const& store(Key const& key, Value const& value) noexcept
  {
    // place_of() masks the place below Size
    // NOLINTNEXTLINE(*-constant-array-index,*-avoid-unchecked-container-access)
    auto& entry = this->m_entries[place_of(key)];
    entry.key = key;
    entry.value = value;
    entry.used = true;
    return entry.value;
  }

 private:
  struct Entry
  {
    Key key = {};
    Value value = {};
    bool used = false;
  };

  [[nodiscard]] static usize place_of(Key const& key) noexcept
  {
    auto constexpr multiplier = 0x9E3779B97F4A7C15_u64;
    auto hash = 0_u64;
    for(auto const word : key)
      hash = hash ^ word * multiplier;
    return static_cast<usize>(hash >> (64 - std::countr_zero(Size)))  // NOLINT(*-signed-bitwise)
         & (Size - 1);
  }

  std::array<Entry, Size> m_entries = {};
};

W_COMMON_NAMESPACE_END
