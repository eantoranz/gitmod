#!/bin/bash

set -ex

# Script that will take care of doing the build process
git config --global --add safe.directory /mnt/work

echo Current status:
git status
echo Ready to start working

/bin/bash
