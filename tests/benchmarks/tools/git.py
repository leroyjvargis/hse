# SPDX-License-Identifier: Apache-2.0
#
# Copyright (C) 2021 Micron Technology, Inc. All rights reserved.

import pathlib
import subprocess


MODULE_DIR = pathlib.Path(__file__).parent.absolute()


def get_git_info(src_dir):
    branch = (
        subprocess.check_output(
            ["git", "rev-parse", "--abbrev-ref", "HEAD"], cwd=MODULE_DIR
        )
        .decode()
        .strip()
    )
    sha = (
        subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=MODULE_DIR)
        .decode()
        .strip()
    )
    describe = (
        subprocess.check_output(["git", "describe"], cwd=MODULE_DIR).decode().strip()
    )

    cp = subprocess.run(["git", "diff", "--quiet"], cwd=MODULE_DIR)
    dirty = bool(cp.returncode != 0)

    result = {
        "branch": branch,
        "describe": describe,
        "dirty": dirty,
        "sha": sha,
    }

    return result
