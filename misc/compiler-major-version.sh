#!/usr/bin/env bash

set -euo pipefail

compiler=$(${CXX} -v 2>&1)

case $compiler in
    *clang*|*gcc*)
        version=`${CXX} --version | grep -o '\([0-9]\)\+.\([0-9]\)\+.\([0-9]\)\+' | sed -e 's/^\([0-9]*\).*/\1/' | head -n1`
        echo "${version}"
        ;;
    *)
        echo
        unknown
        ;;
esac
