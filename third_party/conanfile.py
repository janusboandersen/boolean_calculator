"""
Third-party dependency management with Conan.
Dependecies are installed on system in e.g. /root/.conan2/p/<package-name><hash>/...
https://docs.conan.io/2/tutorial/consuming_packages.html

Packages must be reinstalled after container is restarted.

See ../actions.py for structure of the install command.

Janus, 2023

"""

from conan import ConanFile

class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        #self.requires("nlohmann_json/3.11.2")
        #self.requires("fmt/9.1.0")
        self.requires("gtest/1.13.0")
        #self.requires("foonathan-lexy/2022.12.1")
        #self.requires("catch2/2.13.9")
        #self.requires("cxxopts/3.1.1")