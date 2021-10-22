# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

import pkg1 as A
import pkg1.cls1


class Cls2:
    def __init__(self) -> None:
        self.cls1 = pkg1.cls1.Cls1()