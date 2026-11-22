#!/bin/sh
#
# Created by v1tr10l7 on 14.11.2026.
# Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Checks that a freestanding libprism.a needs nothing beyond the platform
# hook contract (Source/Prism/Core/PlatformHooks.hpp):
#
#   Meta/ci/check-hooks.sh build/libprism.a cryptix|carbonc
set -eu

lib=$1
target=${2:-cryptix}
if [ ! -f "$lib" ]; then
    echo "$lib: no such library" >&2
    exit 1
fi
here=$(cd "$(dirname "$0")" && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

# _GLOBAL_OFFSET_TABLE_ is provided by the linker, not by the consumer.
nm "$lib" | awk '$1 == "U" && $2 != "_GLOBAL_OFFSET_TABLE_" { print $2 }' \
    | sort -u > "$work/undefined"
nm "$lib" | awk 'NF == 3 && $2 ~ /[TWDBRVu]/ { print $3 }' | sort -u > "$work/defined"
comm -23 "$work/undefined" "$work/defined" | c++filt | LC_ALL=C sort > "$work/needed"
grep -v '^#' "$here/platform-hooks-$target.txt" | LC_ALL=C sort > "$work/allowed"

extra=$(LC_ALL=C comm -23 "$work/needed" "$work/allowed")
if [ -n "$extra" ]; then
    echo "libprism.a needs symbols outside the platform hook contract:"
    echo "$extra"
    echo "Remove the dependency, or add it to Meta/ci/platform-hooks-$target.txt and"
    echo "Source/Prism/Core/PlatformHooks.hpp."
    exit 1
fi
echo "libprism.a needs only the platform hooks:"
cat "$work/needed"
