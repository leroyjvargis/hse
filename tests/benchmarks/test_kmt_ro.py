# SPDX-License-Identifier: Apache-2.0
#
# Copyright (C) 2021 Micron Technology, Inc. All rights reserved.

from pathlib import Path

from tools.kmt import KmtTest


name = Path(__file__).stem
# args = "-S0 -b -w0 -l1000 -i2000000 -t300"
args = "-S0 -b -w0 -l1000 -i200000 -t300"

t = KmtTest(name, args.split())

t.execute()
