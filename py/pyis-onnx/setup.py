# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import setuptools
from setuptools.dist import Distribution

DEPENDENCIES = [
    'pathlib',
    'packaging'
]

CLASSIFIERS = [
    "Programming Language :: Python :: 3",
    "Topic :: Scientific/Engineering :: Artificial Intelligence"
]

with open("README.md", "r") as fh:
    long_description = fh.read()

class BinaryDistribution(Distribution):
    """Distribution which always forces a binary package with platform name"""
    def has_ext_modules(foo):
        return True

setuptools.setup(
    name="pyis-onnx",
    version="0.1.dev2",
    author="Lu Ye, Ze Tao, Hao Jin",
    author_email="luye@microsoft.com, zetao@microsoft.com, haoji@microsoft.com",
    description="Python Inference Script for Authoring Cross-Platform End-to-end Models",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/microsoft/python-inference-script",
    packages=setuptools.find_namespace_packages(where = '.', include = ['pyis.*']),
    python_requires="~=3.6", # 3.6 or later, but not version 4.0 or later
    # distclass=BinaryDistribution,
    classifiers=CLASSIFIERS,
    install_requires=DEPENDENCIES,
    include_package_data=True
)