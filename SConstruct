# The target name.
target = "huffman"

# The list of the compilation flags.
flags = [
    "-O2",
    "-std=c99",
    "-Wall",
    "-Wpedantic",
    "-Werror",
    "-Wunused"]

# The location of the all required headers.
include = ["include"]

# The list of source files.
source = Glob("src/*.c")

# The list of headers files.
headers = Glob("include/*.h", strings=True)
headers += Glob("include/huffman/*.h", strings=True)

# The library installation path.
library_path = "/usr/local/lib"
include_path = "/usr/local/include"

# Compile the shared library.
env = Environment(CPPPATH=include, CFLAGS=flags)
library = env.SharedLibrary(target=target, source=source)

# Install the library shared object.
env.Install(dir=library_path, source=library)
# Install the library headers.
env.Install(dir=include_path, source=headers)

# Define the install target.
env.Alias("install", [library_path, include_path])

# Define the uninstall target.
env.Command("uninstall", None, Delete(FindInstalledFiles()))
