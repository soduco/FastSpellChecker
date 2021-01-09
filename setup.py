from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension


__version__ = "0.0.1"

ext_modules = [
    Pybind11Extension(
        "libdictionary",
        sources = [ "pylibdictionary/dictionary.cpp", "libdictionary/src/dictionary.cpp"],
        cxx_std=17,
        include_dirs=["libdictionary/include"],
        extra_link_args = ['-static-libstdc++']
    ),
]

setup(
    name="libdictionary",
    version=__version__,
    author="Edwin Carlinet",
    author_email="edwin.carlinet@lrde.epita.fr",
    url="FIXME",
    description="A test project using pybind11",
    long_description="",
    ext_modules=ext_modules,
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    # cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
