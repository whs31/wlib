#pragma once

#include <string>
#include <string_view>

namespace test
{
  /**
   * @brief Returns @p text with every `^` replaced by the degree sign in UTF-8.
   * @details MSVC reads sources in the code page of the system unless built with `/utf-8`, so the
   * tests spell the degree sign out instead of writing it into a literal.
   */
  [[nodiscard]] inline std::string with_degree_signs(std::string_view const text)
  {
    auto result = std::string();
    result.reserve(text.size() + 8);
    for(auto const character : text)
      if(character == '^')
        result += "\xc2\xb0";
      else
        result += character;
    return result;
  }
}  // namespace test
