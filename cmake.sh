#!/bin/bash

BUILD="../build"
rm -Rf $BUILD
mkdir $BUILD && cd $BUILD && \
cmake.exe -G"Visual Studio 10 2010 Win64" ../ \
&& echo 'done'
|| rm -Rf $BUILD
