#!/bin/bash
git submodule update --init --recursive
if [ "$(uname)" == "Darwin" ]; then
    ./tools/premake5-mac gmake
else
	./tools/premake5-linux gmake
fi
