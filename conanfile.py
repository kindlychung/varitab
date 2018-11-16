from conans import ConanFile, CMake


class VariadictableConan(ConanFile):
    name = "varitab"
    version = "0.9.2"
    license = "MIT"
    author = "kaiyin keenzhong@qq.com"
    url = "https://github.com/kindlychung/varitab"
    description = "C++ library for pretty printing tables"
    topics = ("cpp", "tables", "data")
    exports_sources = "src/*"
    no_copy_source = True

    def package(self):
        self.copy("*.h", dst="include", src="src")
