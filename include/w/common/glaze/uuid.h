#pragma once

#include <string>

#include <w/common/glaze/detail.h>
#include <w/common/uuid.h>

/**
 * @brief Glaze metadata of Uuid: its canonical string, e.g.
 * `"67e55044-10b1-426f-9247-bb680e5fe0c8"`.
 * @details Reading fails with `constraint_violated` on an invalid string.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Uuid>
{
  static auto constexpr value = glz::custom<
    [](w::common::Uuid& uuid, std::string const& text, glz::context& ctx) {
      try {
        uuid = w::common::Uuid(text);
      } catch(w::common::UuidError const&) {
        w::common::_glaze_impl::reject(ctx, "invalid uuid string");
      }
    },
    [](w::common::Uuid const& uuid) { return uuid.to_string(); }>;
};
