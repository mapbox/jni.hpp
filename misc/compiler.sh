#!/usr/bin/env bash
set -euo pipefail
case "$(${CXX} -v 2>&1)" in
    *clang*) echo clang ;;
    *gcc*) echo gcc ;;
    *) echo unknown ;;
esac
