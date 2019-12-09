#!/usr/bin/env bash

set -euo pipefail

compiler=$(${CXX} -v 2>&1)

case $compiler in
    *clang*|*gcc*)
        version=`${CXX} --version | grep -o '\([0-9]\)\+.\([0-9]\)\+.\([0-9]\)\+'`
        echo ${version::1}
        ;;
    *)
        echo
        unknown
        ;;
esac
