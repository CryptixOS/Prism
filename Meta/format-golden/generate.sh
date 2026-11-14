#!/bin/sh
#
# Created by v1tr10l7 on 14.11.2026.
# Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Regenerates Tests/String/FormatGolden/Golden.inc: runs the golden corpus
# through the patched fmt 10.2 that CryptixOS used before Prism::Format.
# fmt is not part of Prism's build; this script downloads and patches it in
# a temporary directory. Run it from anywhere:
#
#   Meta/format-golden/generate.sh
set -eu

here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../.." && pwd)
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

url=https://github.com/fmtlib/fmt/archive/10.2.0.tar.gz
sha=3ca91733a7313a8ad41c0885929415f8ec0a2a31d4dc7e27e9331412f4ca26ac

curl -fsSL "$url" -o "$work/fmt.tar.gz"
echo "$sha  $work/fmt.tar.gz" | sha256sum -c --quiet
tar -xzf "$work/fmt.tar.gz" -C "$work"
patch -d "$work/fmt-10.2.0" -p1 -s -E --forward -i "$here/fmt-10.2.0.patch"

# The configuration CryptixOS built fmt with.
${CXX:-c++} -std=c++20 -O1 -o "$work/generate" "$here/Generate.cpp" \
    -I"$work/fmt-10.2.0/include" \
    -DFMT_STATIC_THOUSANDS_SEPARATOR=1 -DFMT_USE_LONG_DOUBLE=0 \
    -DFMT_USE_DOUBLE=0 -DFMT_USE_FLOAT=0 -DFMT_USE_FLOAT128=0 \
    -DFMT_USE_EXCEPTIONS=0 -DFMT_HEADER_ONLY=1

"$work/generate" > "$root/Tests/String/FormatGolden/Golden.inc"
echo "wrote Tests/String/FormatGolden/Golden.inc"
