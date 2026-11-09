#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <w/common/error.h>
#include <w/common/stdint.h>
#include <w/common/detail/export.h>
#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

static_assert(
  std::endian::native == std::endian::little,
  "byte data is little-endian; big-endian hosts are not supported"
);

/**
 * @defgroup core_bytes Binary Serialization
 * @ingroup core
 * @brief A writer and a reader of trivially copyable values in little-endian byte buffers.
 */

/**
 * @brief Error thrown when a ByteReader cannot read the requested bytes.
 * @ingroup core_bytes
 * @ingroup core_errors
 * @see ByteReader
 */
class W_COMMON_API ByteReadError final : public Error
{
 public:
  using Error::Error;
};

/**
 * @brief Reads values from the front of a span of bytes and refuses reads past its end.
 * @details The reader does not own the bytes, which must outlive it. Values use their in-memory
 * layout on the supported little-endian platforms. Types with padding should be written member by
 * member.
 * @ingroup core_bytes
 * @see ByteWriter
 * @see ByteReadError
 */
class W_COMMON_API ByteReader final
{
 public:
  /**
   * @brief Creates a reader of @p data.
   * @param data Bytes to read, which must outlive the reader.
   */
  explicit ByteReader(std::span<byte const> data) noexcept;

  /**
   * @brief Reads the next value from as many bytes as its type has.
   * @tparam T Trivially copyable type.
   * @throws ByteReadError If fewer bytes remain, or a `bool` is not stored as 0 or 1.
   */
  template <typename T>
    requires std::is_trivially_copyable_v<T>
  [[nodiscard]] T read()
  {
    if constexpr(std::is_same_v<T, bool>) {
      auto const value = this->read<u8>();
      if(value > 1)
        throw ByteReadError("a bool is stored as the byte {}, not 0 or 1", value);
      return value == 1;
    }
    else {
      auto bytes = std::array<byte, sizeof(T)>();
      auto const source = this->take(sizeof(T));
      std::ranges::copy(source, bytes.begin());
      return std::bit_cast<T>(bytes);
    }
  }

  /**
   * @brief Takes the next @p count bytes without copying them.
   * @throws ByteReadError If fewer than @p count bytes remain.
   */
  [[nodiscard]] std::span<byte const> take(usize count);

  /**
   * @brief Skips zero padding up to the next multiple of @p Alignment from the input start.
   * @throws ByteReadError If the padding extends past the remaining bytes.
   */
  template <usize Alignment>
    requires(Alignment > 0)
  void align()
  {
    auto const offset = this->m_size - this->m_data.size();
    std::ignore = this->take((Alignment - offset % Alignment) % Alignment);
  }

  /**
   * @brief Refuses input with bytes left after the expected data.
   * @throws ByteReadError If any bytes remain.
   */
  void expect_end() const;

  /**
   * @brief Returns the number of bytes not read yet.
   */
  [[nodiscard]] usize remaining() const noexcept;

 private:
  std::span<byte const> m_data;
  usize m_size;
};

/**
 * @brief Writes trivially copyable values and byte spans into a little-endian byte buffer.
 * @details Values use their in-memory layout on the supported little-endian platforms. Types with
 * padding should be written member by member.
 * @ingroup core_bytes
 * @see ByteReader
 */
class W_COMMON_API ByteWriter final
{
 public:
  /**
   * @brief Appends the bytes of @p value as it is in memory.
   */
  template <typename T>
    requires std::is_trivially_copyable_v<T>
  ByteWriter& write(T const& value)
  {
    return this->append(std::as_bytes(std::span(&value, 1)));
  }

  /**
   * @brief Appends @p bytes unchanged.
   */
  ByteWriter& append(std::span<byte const> bytes);

  /**
   * @brief Zero-fills the buffer up to its next multiple of @p Alignment.
   */
  template <usize Alignment>
    requires(Alignment > 0)
  ByteWriter& align()
  {
    auto const padding = (Alignment - this->m_data.size() % Alignment) % Alignment;
    this->m_data.resize(this->m_data.size() + padding);
    return *this;
  }

  /**
   * @brief Hands over the completed byte buffer.
   */
  [[nodiscard]] std::vector<byte> finish() && noexcept;

 private:
  std::vector<byte> m_data;
};

W_COMMON_NAMESPACE_END
