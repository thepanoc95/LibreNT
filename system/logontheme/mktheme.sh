#!/bin/bash
# PROJECT:     LibreNT
# LICENSE:     BSD-3-Clause (https://spdx.org/licenses/BSD-3-Clause)
# PURPOSE:     Project maintenance script
# COPYRIGHT:   Copyright 2026 Panoc95

echo "Found zip at $(which zip)"
zip FreeNTLogon.zip -r ./ -x mktheme.sh
echo "Done."

