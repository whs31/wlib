#include <w/common/file.h>
#include <w/common/uuid.h>

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <type_traits>
#include <vector>

#include <fmt/format.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace w;

namespace
{
  // a folder of its own, removed with everything in it
  class TemporaryFolder
  {
   public:
    TemporaryFolder()
      : m_path(
          std::filesystem::temp_directory_path()
          / fmt::format("wlib_file_tests_{}", Uuid::new_v4().to_string(Uuid::Format::Simple))
        )
    {
      std::filesystem::create_directories(this->m_path);
    }

    ~TemporaryFolder()
    {
      auto ec = std::error_code();
      std::filesystem::remove_all(this->m_path, ec);
    }

    TemporaryFolder(TemporaryFolder const&) = delete;
    TemporaryFolder& operator=(TemporaryFolder const&) = delete;
    TemporaryFolder(TemporaryFolder&&) = delete;
    TemporaryFolder& operator=(TemporaryFolder&&) = delete;

    [[nodiscard]] std::filesystem::path const& path() const noexcept { return this->m_path; }

    [[nodiscard]] std::filesystem::path operator/(std::string_view const name) const
    {
      return this->m_path / name;
    }

    // the names of what is in the folder, so that a stray temporary file shows
    [[nodiscard]] std::vector<std::string> names() const
    {
      auto result = std::vector<std::string>();
      for(auto const& entry : std::filesystem::directory_iterator(this->m_path))
        result.push_back(entry.path().filename().string());
      std::ranges::sort(result);
      return result;
    }

   private:
    std::filesystem::path m_path;
  };

  // the current directory for a scope, which a failed check must not leave changed
  class CurrentPath
  {
   public:
    explicit CurrentPath(std::filesystem::path const& path)
      : m_previous(std::filesystem::current_path())
    {
      std::filesystem::current_path(path);
    }

    ~CurrentPath()
    {
      auto ec = std::error_code();
      std::filesystem::current_path(this->m_previous, ec);
    }

    CurrentPath(CurrentPath const&) = delete;
    CurrentPath& operator=(CurrentPath const&) = delete;
    CurrentPath(CurrentPath&&) = delete;
    CurrentPath& operator=(CurrentPath&&) = delete;

   private:
    std::filesystem::path m_previous;
  };

  // a pattern that shows a byte out of place, whatever the size
  [[nodiscard]] std::string pattern(std::size_t const size)
  {
    auto result = std::string(size, '\0');
    auto i = std::size_t {0};
    for(auto& byte : result) {
      byte = static_cast<char>((i * 31 + i / 251) % 256);
      ++i;
    }
    return result;
  }
}  // namespace

TEST_CASE("A file written is read back byte for byte", "[file]")
{
  auto const folder = TemporaryFolder();
  auto const file = folder / "data.bin";

  // around the 64 KiB a read takes at a time
  for(auto const size : {0uz, 1uz, 255uz, 65'535uz, 65'536uz, 65'537uz, 200'000uz}) {
    INFO("size " << size);
    auto const contents = pattern(size);
    write_file(file, contents);
    CHECK(std::filesystem::file_size(file) == size);
    CHECK(read_file(file) == contents);
  }
}

TEST_CASE("A file is read and written in binary, line ends and NULs untouched", "[file]")
{
  auto const folder = TemporaryFolder();
  auto const file = folder / "text.txt";
  auto const contents = std::string("one\r\ntwo\nthree\r\0four\xff\x80", 22);

  write_file(file, contents);
  CHECK(std::filesystem::file_size(file) == contents.size());
  CHECK(read_file(file) == contents);
}

TEST_CASE("Writing a file replaces the old contents and leaves no temporary file", "[file]")
{
  auto const folder = TemporaryFolder();
  auto const file = folder / "settings.toml";

  write_file(file, "a long first version of the settings\n");
  write_file(file, "second\n");

  CHECK(read_file(file) == "second\n");
  CHECK(folder.names() == std::vector<std::string> {"settings.toml"});

  write_file(file, "");
  CHECK(read_file(file).empty());
  CHECK(folder.names() == std::vector<std::string> {"settings.toml"});
}

TEST_CASE("A write that fails leaves the file as it was and no temporary file", "[file]")
{
  auto const folder = TemporaryFolder();
  // note: [whs31 : 2026-11-03]
  // a directory is what cannot be replaced by a file on any platform: the write goes to its
  // temporary file, then the move over the directory fails
  auto const target = folder / "target";
  std::filesystem::create_directory(target);
  {
    auto kept = std::ofstream(target / "kept.txt", std::ios::binary);
    kept << "kept";
  }

  SECTION("throwing")
  {
    CHECK_THROWS_AS(write_file(target, "new"), FileError);
    try {
      write_file(target, "new");
      FAIL("the write must throw");
    } catch(FileError const& error) {
      CHECK(static_cast<bool>(error.code()));
      CHECK_THAT(error.what(), Catch::Matchers::ContainsSubstring("cannot write the file"));
      CHECK_THAT(error.what(), Catch::Matchers::ContainsSubstring("target"));
    }
  }

  SECTION("with an error code")
  {
    auto ec = std::error_code();
    write_file(target, "new", ec);
    CHECK(static_cast<bool>(ec));
  }

  CHECK(std::filesystem::is_directory(target));
  CHECK(read_file(target / "kept.txt") == "kept");
  CHECK(folder.names() == std::vector<std::string> {"target"});
}

TEST_CASE("Writing into a folder that is not there fails and creates nothing", "[file]")
{
  auto const folder = TemporaryFolder();
  auto const file = folder / "missing" / "data.bin";

  SECTION("throwing")
  {
    CHECK_THROWS_AS(write_file(file, "x"), FileError);
    try {
      write_file(file, "x");
      FAIL("the write must throw");
    } catch(FileError const& error) {
      CHECK(error.code() == std::errc::no_such_file_or_directory);
      CHECK_THAT(error.what(), Catch::Matchers::ContainsSubstring("missing/data.bin"));
    }
  }

  SECTION("with an error code")
  {
    auto ec = std::error_code();
    write_file(file, "x", ec);
    CHECK(ec == std::errc::no_such_file_or_directory);
  }

  CHECK(folder.names().empty());
}

TEST_CASE("Reading a file that is not there fails", "[file]")
{
  auto const folder = TemporaryFolder();
  auto const file = folder / "missing.txt";

  SECTION("throwing")
  {
    CHECK_THROWS_AS(read_file(file), FileError);
    try {
      static_cast<void>(read_file(file));
      FAIL("the read must throw");
    } catch(FileError const& error) {
      CHECK(error.code() == std::errc::no_such_file_or_directory);
      CHECK_THAT(error.what(), Catch::Matchers::ContainsSubstring("cannot read the file"));
      CHECK_THAT(error.what(), Catch::Matchers::ContainsSubstring("missing.txt"));
    }
  }

  SECTION("with an error code")
  {
    auto ec = std::error_code();
    CHECK(read_file(file, ec).empty());
    CHECK(ec == std::errc::no_such_file_or_directory);
  }
}

TEST_CASE("Reading a directory fails", "[file]")
{
  auto const folder = TemporaryFolder();

  CHECK_THROWS_AS(read_file(folder.path()), FileError);
  auto ec = std::error_code();
  CHECK(read_file(folder.path(), ec).empty());
  CHECK(ec == std::errc::is_a_directory);
}

TEST_CASE("An empty file is not an error", "[file]")
{
  auto const folder = TemporaryFolder();
  auto const file = folder / "empty";
  write_file(file, "");

  auto ec = std::make_error_code(std::errc::io_error);
  CHECK(read_file(file, ec).empty());
  CHECK_FALSE(ec);
}

TEST_CASE("The error code of a call is cleared when the call succeeds", "[file]")
{
  auto const folder = TemporaryFolder();
  auto const file = folder / "data.txt";

  auto ec = std::make_error_code(std::errc::io_error);
  write_file(file, "data", ec);
  CHECK_FALSE(ec);

  ec = std::make_error_code(std::errc::io_error);
  CHECK(read_file(file, ec) == "data");
  CHECK_FALSE(ec);
}

TEST_CASE("A file is written through a relative path too", "[file]")
{
  auto const folder = TemporaryFolder();
  {
    auto const current = CurrentPath(folder.path());
    write_file("relative.txt", "here");
    CHECK(read_file("relative.txt") == "here");
  }
  CHECK(folder.names() == std::vector<std::string> {"relative.txt"});
}

TEST_CASE("Writers of one file at the same time each write a whole file", "[file]")
{
  auto const folder = TemporaryFolder();
  auto const file = folder / "shared.bin";
  auto constexpr writers = 4;
  auto constexpr writes = 20;
  auto constexpr size = std::size_t {200'000};

  {
    auto threads = std::vector<std::jthread>();
    for(auto writer = 0; writer < writers; ++writer) {
      threads.emplace_back([&file, writer]() {
        auto const contents = std::string(size, static_cast<char>('a' + writer));
        for(auto i = 0; i < writes; ++i) {
          // note: [whs31 : 2026-11-03]
          // Windows may refuse a move onto a file another move has just replaced; the writer
          // tries again as the documentation tells, and what is checked is what is left
          auto ec = std::error_code();
          write_file(file, contents, ec);
        }
      });
    }
  }

  auto const contents = read_file(file);
  REQUIRE(contents.size() == size);
  CHECK(std::ranges::all_of(contents, [&contents](char const c) { return c == contents.front(); }));
  CHECK(contents.front() >= 'a');
  CHECK(contents.front() < 'a' + writers);
  CHECK(folder.names() == std::vector<std::string> {"shared.bin"});
}

TEST_CASE("A file error is an Error with the code of its cause", "[file][error]")
{
  STATIC_CHECK(std::is_base_of_v<Error, FileError>);

  auto const message = FileError("cannot do {}", 42);
  CHECK(std::string_view(message.what()) == "cannot do 42");
  CHECK_FALSE(message.code());

  auto const
    caused = FileError::cannot_read("a/b.txt", std::make_error_code(std::errc::permission_denied));
  CHECK(caused.code() == std::errc::permission_denied);
  CHECK_THAT(caused.what(), Catch::Matchers::StartsWith("cannot read the file 'a/b.txt': "));
}
