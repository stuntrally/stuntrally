import os
from conan import ConanFile
from conan.tools.files import copy


class StuntRally(ConanFile):
    name = "StuntRally"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    default_options = {
        "ogre3d*:nodeless_positioning": "True",
        "ogre3d*:resourcemanager_strict": "off",
        "bullet3*:extras": "True",
        "bullet3*:network_support": "True",
        "sdl*:sdl2main": "False",
        "ogre3d*:nodeless_positioning": "True",
    }

    def requirements(self):
        self.requires("boost/1.81.0")
        self.requires("ogre3d/13.6.4@anotherfoxguy/stable")
        self.requires("bullet3/3.25@anotherfoxguy/patched")# Needs a patched to build on windows 
        self.requires("sdl/2.26.1")
        self.requires("mygui/3.4.1@anotherfoxguy/stable")
        self.requires("ogg/1.3.5")
        self.requires("vorbis/1.3.7")
        self.requires("openal/1.22.2")
        self.requires("enet/1.3.17")
        #self.requires("tinyxml/2.6.2")
        self.requires("tinyxml2/9.0.0")

        self.requires("libpng/1.6.39", override=True)
        self.requires("libwebp/1.3.0", override=True)
        self.requires("zlib/1.2.13", override=True)
        self.requires("xz_utils/5.4.2", override=True)

    def generate(self):
        for dep in self.dependencies.values():
            for f in dep.cpp_info.bindirs:
                self.cp_data(f)
            for f in dep.cpp_info.libdirs:
                self.cp_data(f)

    def cp_data(self, src):
        bindir = self.build_folder
        copy(self, "*.dll", src, bindir, False)
        copy(self, "*.so*", src, bindir, False)
        redistdir = os.path.join(self.build_folder, "redist")
        copy(self, "*.dll", src, redistdir, False)
        copy(self, "*.so*", src, redistdir, False)
