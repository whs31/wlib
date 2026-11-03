#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include <w/common/error.h>
#include <w/common/detail/namespace.h>
#include <w/common/detail/export.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Error raised when a whole file cannot be read or written.
 * @ingroup core_errors
 * @ingroup core_filesystem
 * @see read_file()
 * @see write_file()
 */
class W_COMMON_API FileError final : public Error
{
 public:
  /**
   * @brief Constructs a file error with a formatted message.
   * @tparam Args Format argument types.
   * @param fmt Format string.
   * @param args Format arguments.
   */
  template <typename... Args>
  explicit FileError(fmt::format_string<Args...> fmt, Args&&... args)
    : Error(fmt, std::forward<Args>(args)...)
  {}

  [[nodiscard]] static FileError cannot_read(
    std::filesystem::path const& path,
    std::error_code error_code
  );
  [[nodiscard]] static FileError cannot_write(
    std::filesystem::path const& path,
    std::error_code error_code
  );

  /**
   * @brief Returns the error code of the failure, e.g. `std::errc::no_such_file_or_directory` for
   * a file that is not there.
   * @details Empty for an error built from a message.
   */
  [[nodiscard]] std::error_code const& code() const noexcept { return this->m_code; }

 private:
  std::error_code m_code;
};

/**
 * @brief Reads the whole of a file, byte for byte.
 * @details The file is read in binary mode: no line end is translated and the bytes may be
 * anything, `\0` too. What they mean, e.g. a TOML document or the bytes of a ByteWriter, is the
 * caller's to read.
 *
 * @code {.cpp}
 * using namespace w;
 *
 * auto const text = read_file(folder / "scene.toml");
 *
 * auto error = std::error_code();
 * auto const key = read_file(folder / "import.key", error);
 * if(error == std::errc::no_such_file_or_directory)
 *   return import_again();
 * @endcode
 *
 * @param path File to read.
 * @return Contents of the file, empty for an empty file.
 * @throws FileError If the file does not exist, is a directory or cannot be read.
 * @ingroup core_filesystem
 * @see write_file()
 */
[[nodiscard]] W_COMMON_API std::string read_file(std::filesystem::path const& path);

/**
 * @brief Reads the whole of a file, byte for byte.
 * @param path File to read.
 * @param ec Receives an error code on failure and is cleared on success.
 * @return Contents of the file, or an empty string on failure; `ec` tells it from an empty file.
 * @see read_file(std::filesystem::path const&)
 */
[[nodiscard]] W_COMMON_API std::string read_file(
  std::filesystem::path const& path,
  std::error_code& ec
) noexcept;

/**
 * @brief Writes a whole file so that a failed write leaves the file as it was.
 * @details The bytes go to a new file in the directory of @p path, named after it and a random
 * suffix, `<path>.<8 hex digits>.tmp`, and that file is moved over @p path with
 * `std::filesystem::rename()`, which replaces an existing file. A reader of @p path therefore sees
 * the old contents or the new ones, never a part of them, and a write that fails, e.g. on a full
 * disk, removes its temporary file and leaves @p path untouched. Writers of one path at the same
 * time do not share a temporary file, and the last to finish wins. The file is written in binary
 * mode: no line end is translated.
 *
 * Constraints:
 * - The directory of @p path must exist; nothing is created.
 * - The new file is a new file: it has the default permissions and owner, not those of a file it
 *   replaces, and replaces a symbolic link at @p path rather than the file it points to.
 * - The bytes are not forced to the disk before the move (no `fsync`), so a crash of the program
 *   keeps the old or the new file, but a loss of power may lose the write on some file systems.
 * - A program killed between the write and the move leaves the temporary file behind.
 * - On Windows `std::filesystem::rename()` replaces an existing file as on POSIX, but not one that
 *   another process holds open without sharing deletion (an indexer or a virus scanner may, for a
 *   moment). That write fails with a permission error and leaves the old file; a caller that must
 *   not lose the write retries.
 *
 * @code {.cpp}
 * using namespace w;
 *
 * write_file(folder / "scene.toml", text);
 *
 * auto error = std::error_code();
 * write_file(folder / "import.key", key, error);
 * if(error)
 *   W_LOG(warn, "cannot write the key: {}", error.message());
 * @endcode
 *
 * @param path File to write, made or replaced.
 * @param contents Bytes of the file, none or more.
 * @throws FileError If the file cannot be written; the file at @p path is then as it was.
 * @ingroup core_filesystem
 * @see read_file()
 */
W_COMMON_API void write_file(std::filesystem::path const& path, std::string_view contents);

/**
 * @brief Writes a whole file so that a failed write leaves the file as it was.
 * @param path File to write, made or replaced.
 * @param contents Bytes of the file, none or more.
 * @param ec Receives an error code on failure and is cleared on success.
 * @see write_file(std::filesystem::path const&, std::string_view)
 */
W_COMMON_API void write_file(
  std::filesystem::path const& path,
  std::string_view contents,
  std::error_code& ec
) noexcept;

W_COMMON_NAMESPACE_END
