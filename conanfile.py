from conans import ConanFile, CMake, tools
from os import getcwd

class Domains(ConanFile):
    name = "domains"
    version = "0.0.1"
    url = "https://github.com/skizzay/domains.git"
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake", "txt"
    exports = "CMakeLists.txt", "domains/*"
    build_requires = 'catch/1.5.0@TyRoXx/stable'

    def build(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.configure()
        cmake.build()

    def configure(self):
        tools.check_min_cppstd(self, "20")