from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension


__version__ = "0.0.1"

ext_modules = [
    Pybind11Extension(
        "fastspellchecker",
        sources = [ "FastSpellChecker/FastSpellChecker.cpp", "libfsc/src/fsc.cpp"],
        cxx_std=17,
        include_dirs=["libfsc/include"],
        extra_link_args = ['-static-libstdc++']
    ),
]

setup(
    name="fastspellchecker",
    version=__version__,
    author="Edwin Carlinet",
    author_email="edwin.carlinet@lrde.epita.fr",
    url="FIXME",
    description="",
    long_description="",
    ext_modules=ext_modules,
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    # cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
