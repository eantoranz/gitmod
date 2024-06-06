#!/bin/bash

set -ex

# Script that will set us in a "usable" environment

git config --global --add safe.directory /mnt/work

echo Current status:
git status
echo Ready to start working

/bin/bash
