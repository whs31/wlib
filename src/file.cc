#include <w/common/file.h>

#include <cerrno>

#include <array>
#include <filesystem>
#include <fstream>
#include <ios>
#include <new>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

#include <fmt/format.h>

#include <w/common/functional/defer.h>
#include <w/common/uuid.h>
#include <w/common/stdint.h>

namespace
{
  // note: [whs31 : 2026-11-03]
  // the standard streams only say that they failed, so what the C library under them left in errno
  // is the reason; it sets it on every platform the library targets
  [[nodiscard]] std::error_code last_error() noexcept
  {
    if(errno == 0)
      return std::make_error_code(std::errc::io_error);
    return {errno, std::generic_category()};
  }

  // note: [whs31 : 2026-11-03]
  // runs `operation`, which reports a failure in `ec`, and reports what it throws there too: the
  // functions with an error code do not throw
  template <typename Operation>
  void attempt(std::error_code& ec, Operation&& operation) noexcept
  {
    ec.clear();
    try {
      operation();
    } catch(std::bad_alloc const&) {
      ec = std::make_error_code(std::errc::not_enough_memory);
    } catch(std::length_error const&) {
      ec = std::make_error_code(std::errc::file_too_large);
    } catch(...) {
      ec = std::make_error_code(std::errc::io_error);
    }
  }

  [[nodiscard]] std::string read_all(std::filesystem::path const& path, std::error_code& ec)
  {
    // note: [whs31 : 2026-11-03]
    // a stream opens a directory on some platforms and fails to read it only
    auto probe = std::error_code();
    if(std::filesystem::is_directory(path, probe)) {
      ec = std::make_error_code(std::errc::is_a_directory);
      return {};
    }

    errno = 0;
    auto stream = std::ifstream(path, std::ios::binary);
    if(not stream) {
      ec = last_error();
      return {};
    }

    auto contents = std::string();
    // note: [whs31 : 2026-11-03]
    // the size is a hint only: a file may grow or, like those of /proc, claim none
    if(auto const size = std::filesystem::file_size(path, probe); not probe)
      contents.reserve(static_cast<w::usize>(size));

    auto buffer = std::array<char, w::usize {64} * 1'024>();
    while(true) {
      stream.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
      contents.append(buffer.data(), static_cast<w::usize>(stream.gcount()));
      if(not stream)
        break;
    }
    // note: [whs31 : 2026-11-03]
    // the end of the file sets failbit and eofbit; a failed read sets badbit
    if(stream.bad()) {
      ec = last_error();
      return {};
    }
    return contents;
  }

  // note: [whs31 : 2026-11-03]
  // beside `path`, so that it is on the same file system, which the move needs to replace the file
  // in one step; random, so that writers of one file at the same time do not write one temporary
  [[nodiscard]] std::filesystem::path temporary_path_of(std::filesystem::path const& path)
  {
    auto const id = w::Uuid::new_v4().to_string(w::Uuid::Format::Simple).substr(0, 8);
    auto temporary = path;
    temporary += fmt::format(".{}.tmp", id);
    return temporary;
  }

  void write_all(
    std::filesystem::path const& path,
    std::string_view const contents,
    std::error_code& ec
  )
  {
    auto const temporary = temporary_path_of(path);
    auto cleanup = w::defer([&temporary]() noexcept {
      auto ignored = std::error_code();
      std::filesystem::remove(temporary, ignored);
    });

    {
      errno = 0;
      auto stream = std::ofstream(temporary, std::ios::binary | std::ios::trunc);
      if(not stream) {
        ec = last_error();
        return;
      }
      stream.write(contents.data(), static_cast<std::streamsize>(contents.size()));
      // note: [whs31 : 2026-11-03]
      // Windows cannot move a file that is still open, and closing is what flushes the last bytes
      stream.close();
      if(not stream) {
        ec = last_error();
        return;
      }
    }

    std::filesystem::rename(temporary, path, ec);
    if(not ec)
      cleanup.dismiss();
  }
}  // namespace

W_COMMON_NAMESPACE_BEGIN

FileError FileError::cannot_read(
  std::filesystem::path const& path,
  std::error_code const error_code
)
{
  auto
    error = FileError("cannot read the file '{}': {}", path.generic_string(), error_code.message());
  error.m_code = error_code;
  return error;
}

FileError FileError::cannot_write(
  std::filesystem::path const& path,
  std::error_code const error_code
)
{
  auto error = FileError(
    "cannot write the file '{}': {}",
    path.generic_string(),
    error_code.message()
  );
  error.m_code = error_code;
  return error;
}

std::string read_file(std::filesystem::path const& path)
{
  auto ec = std::error_code();
  auto contents = read_file(path, ec);
  if(ec)
    throw FileError::cannot_read(path, ec);
  return contents;
}

std::string read_file(std::filesystem::path const& path, std::error_code& ec) noexcept
{
  auto contents = std::string();
  attempt(ec, [&]() { contents = read_all(path, ec); });
  if(ec)
    contents.clear();
  return contents;
}

void write_file(std::filesystem::path const& path, std::string_view const contents)
{
  auto ec = std::error_code();
  write_file(path, contents, ec);
  if(ec)
    throw FileError::cannot_write(path, ec);
}

void write_file(
  std::filesystem::path const& path,
  std::string_view const contents,
  std::error_code& ec
) noexcept
{
  attempt(ec, [&]() { write_all(path, contents, ec); });
}

W_COMMON_NAMESPACE_END
