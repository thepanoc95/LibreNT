#!/bin/bash

echo "Found zip at $(which zip)"
zip FreeNTLogon.zip -r ./ -x mktheme.sh
echo "Done."

