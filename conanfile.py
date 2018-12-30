from conans import ConanFile, CMake


class DomainsConan(ConanFile):
    name = "domains"
    version = "0.0.1"
    license = "MIT"
    url = "https://github.com/skizzay/domains"
    description = "Modern C++ library to facilitate CQRS, event sourcing, and CQRS+ES architectures"
    requires = "fsm/0.0.1@skizzay/stable", "utilz/0.0.4@skizzay/stable"
    no_copy_source = True
    exports_sources = "src/*"
    exports = 'LICENSE', 'README.md'
    settings = 'cppstd', 'build_type'
    build_requires = "catch2/2.2.2@bincrafters/stable", "cmake_installer/3.11.2@conan/stable", "kerchow/2.0.1@skizzay/testing"

    def package(self):
        self.copy("*.h", dst="include", src="src", keep_path=True)
        cmake = self.get_cmake()
        cmake.install()

    def build(self):
        cmake = self.get_cmake()
        cmake.build()

    def get_cmake(self):
        cmake = CMake(self)
        cmake.configure()
        return cmake
