from conans import ConanFile, CMake


class VariadictableConan(ConanFile):
    name = "variadic_table"
    version = "0.9.1"
    license = "MIT"
    author = "kaiyin keenzhong@qq.com"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of Variadictable here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")
    exports_sources = "src/*"
    no_copy_source = True

    def package(self):
        self.copy("*.h", dst="include", src="src")
