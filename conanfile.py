from conans import ConanFile, CMake, GCC
from conans.util.files import mkdir
from conans.model.values import Values
from conans.model.options import Options
from os import getcwd, rename
from os.path import join, exists, dirname, abspath

class Domains(ConanFile):
    name = "domains"
    version = "0.0.1"
    url = "https://github.com/skizzay/domains.git"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake", "ycm"
    exports = "CMakeLists.txt", "cqrs/*", "utils/*", "messaging/*", "tests/*"

    def build(self):
        self.output.info(self.options)
        cmake = CMake(self.settings)
        self._execute("cmake %s %s %s" % (self.conanfile_directory, cmake.command_line, self._extra_cmake_flags))
        self._execute("cmake --build %s %s" % (getcwd(), cmake.build_config))

        if self.options.build_tests:
            self.run("ctest")
