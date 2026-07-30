#!/bin/bash

if [[ ! -d ./build ]]; then
	cmake -B build -G Ninja
	ninja -C build
else 
	ninja -C build
fi


if [[ ! -e compile_commands.json && -d build ]]; then
	echo "Creating linked file for compile_commands.json"
	ln -s $(pwd)/build/compile_commands.json compile_commands.json
fi
