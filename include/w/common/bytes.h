#pragma once

#include <cstring>

#include <array>
#include <bit>
#include <concepts>
#include <limits>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <w/common/stdint.h>
#include <w/common/error.h>
#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @defgroup core_bytes Binary Serialization
 * @ingroup core
 * @brief A writer and a reader of trivially copyable values in a string of bytes.
 * @details The bytes are the objects as they are in memory, in the byte order of the host and with
 * the size, alignment and padding its compiler gives them. Data is read back on a platform of the
 * same kind that wrote it, e.g. a file of an engine's own cache, and is not an exchange format. All
 * the platforms the library targets are little-endian, so code that stores the bytes for another
 * machine of those can `static_assert(std::endian::native == std::endian::little)`.
 *
 * A type that has padding bytes, e.g. a `struct { char a; int b; }`, writes whatever the padding
 * holds: prefer a type without padding, or write its members one by one.
 * @see ByteWriter
 * @see ByteReader
 * @see ByteReadError
 */

/**
 * @brief Error thrown by a ByteReader when its input is too short or states a count that cannot be
 * true.
 * @ingroup core_bytes
 * @ingroup core_errors
 * @see ByteReader
 */
class ByteReadError final : public Error
{
 public:
  using Error::Error;
};

/**
 * @brief Writes trivially copyable values, counted vectors of them and counted text into a string
 * of bytes, to read back with a ByteReader.
 * @details A count is a 32-bit unsigned integer, so a vector or a text of more than 4 294 967 295
 * items does not fit and is refused. See @ref core_bytes for the byte order and the layout.
 *
 * @code {.cpp}
 * using namespace w;
 *
 * auto out = ByteWriter();
 * out.value(u32 {2});
 * out.text("wheel");
 * out.values(std::vector<Vector3> {{0.0F, 1.0F, 2.0F}, {3.0F, 4.0F, 5.0F}});
 * write_file("part.bin", out.data());
 * @endcode
 *
 * @ingroup core_bytes
 * @see ByteReader
 */
class ByteWriter
{
 public:
  /**
   * @brief Appends the bytes of a value, as they are in memory.
   * @tparam T Trivially copyable type, which is not a pointer to anything that must be followed.
   * @param item Value to append.
   */
  template <typename T>
    requires(std::is_trivially_copyable_v<T>)
  void value(T const& item)
  {
    this->append(&item, sizeof(T));
  }

  /**
   * @brief Appends a count: a 32-bit unsigned integer, as ByteReader::count() reads it.
   * @param items Number of the items that follow.
   * @throws InvalidArgumentError If @p items does not fit in 32 bits.
   */
  void count(usize const items)
  {
    if(items > std::numeric_limits<u32>::max())
      throw InvalidArgumentError("a count of {} does not fit in 32 bits", items);
    this->value(static_cast<u32>(items));
  }

  /**
   * @brief Appends a counted vector: the number of its items, then their bytes.
   * @tparam Range Contiguous range of trivially copyable items, e.g. a `std::vector`, a
   * `std::array` or a `std::span`.
   * @param items Items to append, none or more.
   * @throws InvalidArgumentError If the range has more than 4 294 967 295 items.
   * @see ByteReader::values()
   */
  template <std::ranges::contiguous_range Range>
    requires(
      std::ranges::sized_range<Range>
      and std::is_trivially_copyable_v<std::ranges::range_value_t<Range>>
    )
  void values(Range const& items)
  {
    auto const size = std::ranges::size(items);
    this->count(size);
    this->append(std::ranges::data(items), size * sizeof(std::ranges::range_value_t<Range>));
  }

  /**
   * @brief Appends counted text: the number of its bytes, then the bytes, with no terminator.
   * @param content Text to append, which may hold any bytes, `\0` too.
   * @throws InvalidArgumentError If the text has more than 4 294 967 295 bytes.
   * @see ByteReader::text()
   */
  void text(std::string_view const content)
  {
    this->count(content.size());
    this->append(content.data(), content.size());
  }

  /**
   * @brief Returns the bytes written so far.
   */
  [[nodiscard]] std::string const& data() const noexcept { return this->m_data; }

  /**
   * @brief Returns the number of bytes written so far.
   */
  [[nodiscard]] usize size() const noexcept { return this->m_data.size(); }

  /**
   * @brief Hands over the bytes written so far and starts again from none.
   */
  [[nodiscard]] std::string release() noexcept { return std::exchange(this->m_data, {}); }

 private:
  void append(void const* const bytes, usize const size)
  {
    // note: [whs31 : 2026-11-03]
    // an empty vector's data() may be null
    if(size != 0)
      this->m_data.append(static_cast<char const*>(bytes), size);
  }

  std::string m_data;
};

/**
 * @brief Reads what a ByteWriter wrote, from the front, and refuses what is not there.
 * @details The reader does not own the bytes, which must outlive it. Any input is safe to read:
 * a read past the end throws ByteReadError instead of overrunning, and a count, as in values(),
 * can never claim more items than the bytes left can hold, so a corrupt count cannot make the
 * reader allocate more than the input is long. The bytes of a value are not checked beyond its
 * size, except for a `bool`, which is read from one byte that is 0 or 1; a type that has invalid
 * bit patterns, e.g. a `bool` in a struct, is the caller's to check. See @ref core_bytes for the
 * byte order and the layout.
 *
 * @code {.cpp}
 * using namespace w;
 *
 * auto const bytes = read_file("part.bin");
 * auto in = ByteReader(bytes);
 * auto const version = in.value<u32>();
 * auto const name = in.text();
 * auto const points = in.values<Vector3>();
 * if(not in.done())
 *   throw ByteReadError("part.bin has bytes after its part");
 * @endcode
 *
 * @ingroup core_bytes
 * @see ByteWriter
 * @see ByteReadError
 */
class ByteReader
{
 public:
  /**
   * @brief Creates a reader of @p data.
   * @param data Bytes to read, which must outlive the reader.
   */
  explicit ByteReader(std::string_view const data) noexcept
    : m_data(data)
  {}

  /**
   * @brief A reader of a temporary string would read freed memory.
   */
  template <typename Temporary>
    requires(std::same_as<Temporary, std::string>)
  explicit ByteReader(Temporary&&) = delete;

  /**
   * @brief Reads the next value, from as many bytes as its type has.
   * @tparam T Trivially copyable type.
   * @return Value, from the bytes as they are in memory.
   * @throws ByteReadError If fewer bytes are left, or for a `bool` if its byte is not 0 or 1.
   */
  template <typename T>
    requires(std::is_trivially_copyable_v<T>)
  [[nodiscard]] T value()
  {
    if constexpr(std::same_as<T, bool>) {
      auto const byte = this->value<u8>();
      if(byte > 1)
        throw ByteReadError("a bool is stored as the byte {}, not 0 or 1", byte);
      return byte == 1;
    }
    else {
      auto bytes = std::array<byte, sizeof(T)>();
      this->read(bytes.data(), bytes.size());
      return std::bit_cast<T>(bytes);
    }
  }

  /**
   * @brief Reads a counted vector, as ByteWriter::values() wrote it.
   * @tparam T Trivially copyable type with a default constructor, other than `bool`.
   * @return Items, none or more.
   * @throws ByteReadError If the count is more than the bytes left can hold, or they end early.
   */
  template <typename T>
    requires(
      std::is_trivially_copyable_v<T> and std::default_initializable<T>
      and not std::same_as<T, bool>
    )
  [[nodiscard]] std::vector<T> values()
  {
    auto result = std::vector<T>(this->count(sizeof(T)));
    this->read(result.data(), result.size() * sizeof(T));
    return result;
  }

  /**
   * @brief Reads a count, as ByteWriter::count() wrote it, of items of at least @p size bytes each.
   * @details The count of a corrupt input is whatever its bytes say, and a caller that sizes a
   * vector of items by it must not let it exceed what the input can hold. With the size of an item
   * here, it cannot exceed what is left of the input.
   *
   * @param size Least number of bytes an item takes, one or more; a count of items that take no
   * bytes cannot be checked.
   * @return Number of the items that follow, at most the bytes left divided by @p size.
   * @throws ByteReadError If the count does not fit in the bytes left, or they end early.
   */
  [[nodiscard]] u32 count(usize const size = 1)
  {
    auto const items = this->value<u32>();
    if(auto const each = size == 0 ? 1uz : size; items > this->m_data.size() / each)
      throw ByteReadError(
        "a count of {} items of {} bytes or more does not fit in the {} bytes left",
        items,
        each,
        this->m_data.size()
      );
    return items;
  }

  /**
   * @brief Reads counted text, as ByteWriter::text() wrote it.
   * @return Text, which may hold any bytes.
   * @throws ByteReadError If the text is longer than the bytes left, or they end before its size.
   */
  [[nodiscard]] std::string text()
  {
    auto const size = this->value<u32>();
    return std::string(this->take(size));
  }

  /**
   * @brief Returns the number of bytes not read yet.
   */
  [[nodiscard]] usize remaining() const noexcept { return this->m_data.size(); }

  /**
   * @brief Returns true if every byte has been read.
   */
  [[nodiscard]] bool done() const noexcept { return this->m_data.empty(); }

 private:
  [[nodiscard]] std::string_view take(usize const size)
  {
    if(size > this->m_data.size())
      throw ByteReadError("input ends early: {} bytes needed, {} left", size, this->m_data.size());
    auto const result = this->m_data.substr(0, size);
    this->m_data.remove_prefix(size);
    return result;
  }

  void read(void* const bytes, usize const size)
  {
    auto const taken = this->take(size);
    // note: [whs31 : 2026-11-03]
    // an empty vector's data() may be null
    if(size != 0)
      std::memcpy(bytes, taken.data(), size);
  }

  std::string_view m_data;
};

W_COMMON_NAMESPACE_END
