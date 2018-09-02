from conans import ConanFile


class DomainsConan(ConanFile):
    name = "domains"
    version = "0.0.1"
    license = "MIT"
    url = "https://github.com/skizzay/domains"
    description = "Modern C++ library to facilitate CQRS, event sourcing, and CQRS+ES architectures"
    requires = "fsm/0.0.1@skizzay/stable", "utilz/0.0.3@skizzay/stable"
    no_copy_source = True
    exports_sources = "src/*"
    exports = 'LICENSE', 'README.md'
    # No settings/options are necessary, this is header only

    def package(self):
        self.copy("*.h", dst="include", src="src", keep_path=True)
