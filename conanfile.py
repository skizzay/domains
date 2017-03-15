from conans import ConanFile, CMake
from os import getcwd

class Domains(ConanFile):
    name = "domains"
    version = "0.0.1"
    url = "https://github.com/skizzay/domains.git"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake", "txt", "env", "ycm"
    exports = "CMakeLists.txt", "domains/*"
    dev_requires = 'catch/1.5.0@TyRoXx/stable', 'kerchow/1.0.1@skizzay/stable'

    def build(self):
        cmake = CMake(self.settings)
        self._execute("cmake %s %s %s" % (self.conanfile_directory, cmake.command_line, self._build_tests))
        self._execute("cmake --build %s %s" % (getcwd(), cmake.build_config))

        if self.scope.dev:
            self.run("ctest")

    @property
    def _build_tests(self):
        if self.scope.dev:
            return "-DBUILD_TESTS=1"
        return ""

    def _execute(self, command):
        self.output.info(command)
        self.run(command)
