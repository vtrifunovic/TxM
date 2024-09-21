#!/bin/bash
sudo test
read -p "Build from source [y/n]? " build
if [ "$build" = "y" ]; then
	make
fi

sudo mv txm /usr/bin
if [ -d ~/.txm ]; then
	echo "TxM directory exists... skipping creation"
else
	mkdir ~/.txm && echo "Creating TxM directory in home..."
fi

cp txm_config.cfg ~/.txm/ --verbose
cp assets/unknown.png ~/.txm/ --verbose

read -p "Copy fonts to .txm folder [y/n]? " answer
if [ "$answer" = "y" ]; then
	cp fonts/* ~/.txm --verbose
fi
