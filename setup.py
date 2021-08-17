# -*- coding: utf-8 -*-

"""
setup.py implementation, interesting because it parsed the first __init__.py and
    extracts the `__author__` and `__version__`
"""

from ast import Assign, Constant, Str, parse
from distutils.sysconfig import get_python_lib
from functools import partial
from operator import attrgetter
from os import path, listdir
from os.path import extsep
from platform import python_version_tuple

from setuptools import find_packages, setup

if python_version_tuple()[0] == '2':
    from itertools import imap as map, ifilter as filter

package_name = "docopt_c"

with open(
    path.join(path.dirname(__file__), "README{extsep}md".format(extsep=extsep)),
    "rt",
    encoding="utf-8",
) as fh:
    long_description = fh.read()


def to_funcs(*paths):
    """
    Produce function tuples that produce the local and install dir, respectively.

    :param paths: one or more str, referring to relative folder names
    :type paths: ```*paths```

    :returns: 2 functions
    :rtype: ```Tuple[Callable[Optional[List[str]], str], Callable[Optional[List[str]], str]]```
    """
    return (
        partial(path.join, path.dirname(__file__), package_name, *paths),
        partial(path.join, get_python_lib(prefix=""), package_name, *paths),
    )


def main():
    """Main function for setup.py; this actually does the installation"""
    with open(
        path.join(
            path.abspath(path.dirname(__file__)),
            package_name,
            "__init__{extsep}py".format(extsep=extsep),
        )
    ) as f:
        parsed_init = parse(f.read())

    __author__, __version__, __description__ = map(
        lambda node: node.value if isinstance(node, Constant) else node.s,
        filter(
            lambda node: isinstance(node, (Constant, Str)),
            map(
                attrgetter("value"),
                filter(lambda node: isinstance(node, Assign), parsed_init.body),
            ),
        ),
    )

    _data_join, _data_install_dir = to_funcs("_data")

    setup(
        name=package_name,
        author=__author__,
        author_email="807580+SamuelMarks@users.noreply.github.com",
        version=__version__,
        description=__description__,
        long_description=long_description,
        long_description_content_type="text/markdown",
        packages=find_packages(),
        package_dir={package_name: package_name},
        classifiers=[
            "Development Status :: 3 - Alpha",
            "Environment :: Console",
            "Intended Audience :: Developers",
            "License :: OSI Approved",
            "License :: OSI Approved :: MIT License",
            "Natural Language :: English",
            "Operating System :: OS Independent",
            "Programming Language :: Python :: 2.7",
            "Programming Language :: Python :: 3.7",
            "Programming Language :: Python :: 3.8",
            "Programming Language :: Python :: 3.9",
            "Programming Language :: Python :: Implementation",
            "Topic :: Scientific/Engineering :: Interface Engine/Protocol Translator",
            "Topic :: Software Development",
            "Topic :: Software Development :: Build Tools",
            "Topic :: Software Development :: Code Generators",
            "Topic :: Software Development :: Compilers",
            "Topic :: Software Development :: Pre-processors"
        ],
        url="https://github.com/offscale/docopt.c",
        data_files=[
            (_data_install_dir(), list(map(_data_join, listdir(_data_join()))))
        ]
    )


def setup_py_main():
    """Calls main if `__name__ == '__main__'`"""
    if __name__ == "__main__":
        main()


setup_py_main()
