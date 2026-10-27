from conan import ConanFile
from conan.tools.build import check_min_cppstd


class WLibRecipe(ConanFile):
  name = "wlib"
  version = "8.0.0"
  description = "Common C++23 utility library"
  author = "whs31 <whs31@github.io>"
  settings = "os", "arch", "compiler", "build_type"
  license = "MIT"
  url = "https://git.whs31.com/shared/support/wlib"
  exports = "CMakeLists.txt"
  exports_sources = "CMakeLists.txt", "cmake/*", "config/*", "src/*", "include/*", "tests/*"
  options = {
    "shared": [True, False],
    "fPIC": [True, False],
    "test": [True, False],
    "with_glaze": [True, False],
  }
  default_options = {
    "shared": True,
    "fPIC": True,
    "test": False,
    "with_glaze": False,
  }

  user = "whs31"
  channel = "dev"
  python_requires = "conan_helpers/1.1@whs31/dev"
  python_requires_extend = "conan_helpers.Base"
  cmake_package_prune = ("lib/cmake", "lib/pkgconfig", "res", "share")

  def config_options(self):
    if self.settings.os == "Windows":
      del self.options.fPIC

  def configure(self):
    if self.options.shared:
      self.options.rm_safe("fPIC")

  def validate(self):
    check_min_cppstd(self, "23")

  def requirements(self):
    self.req(
      "spdlog",
      options={"shared": bool(self.options.shared)},
      transitive_headers=True,
      transitive_libs=True,
    )
    self.req("fmt",
      options={"shared": bool(self.options.shared)},
      transitive_headers=True,
      transitive_libs=True
    )
    if self.options.with_glaze:
      self.req("glaze", transitive_headers=True)
    if self.options.test:
      self.req("catch2", test=True)
      self.req("glm", test=True)

  def configure_cmake_toolchain(self, toolchain):
    toolchain.shared = self.options.shared
    toolchain.cache_variables["BUILD_TESTING"] = bool(self.options.test)
    toolchain.cache_variables["WLIB_WITH_GLAZE"] = bool(self.options.with_glaze)

  def package_info(self):
    self.cpp_info.set_property("cmake_file_name", "WLib")
    lib = self.cpp_info.components["lib"]
    lib.set_property("cmake_target_name", "w::lib")
    lib.libs = ["wlib"]
    if not self.options.shared:
      lib.defines = ["W_COMMON_STATIC_LIBRARY"]
    if self.settings.os == "Windows":
      lib.system_libs = ["ws2_32"]
    lib.requires = [
      "spdlog::spdlog",
      "fmt::fmt",
    ]

    if self.options.with_glaze:
      glaze = self.cpp_info.components["glaze"]
      glaze.set_property("cmake_target_name", "w::lib::glaze")
      glaze.requires = [
        "lib",
        "glaze::glaze",
      ]
