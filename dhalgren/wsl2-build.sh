#!/usr/bin/env bash

set -x
set -o pipefail

PROJECT_NAME=dhalgren
MAKE_OBJECTS=webbuild.zip
MAKE_TARGET=webzip

if test -z ${PROJECT_SRC_DIR}; then
    echo "Please 'export PROJECT_SRC_DIR=<path to ${PROJECT_NAME} source>'"
    exit 1
fi

if test -z ${PROJECT_BUILD_DIR}; then
    echo "Please 'export PROJECT_BUILD_DIR=<path to parent directory where you want things to get built (e.g. ${HOME})>'"
    exit 1
fi

read -p "Build target '${MAKE_TARGET}' for ${PROJECT_NAME} in build directory '${PROJECT_BUILD_DIR}'? [y/N] " CONTINUE  
if [[ ${CONTINUE} != "y" && ${CONTINUE} != "Y" ]]; then
    exit 1
fi

cd ${PROJECT_BUILD_DIR}

rm -rf ${PROJECT_NAME}

cp -r ${PROJECT_SRC_DIR} .

cd ${PROJECT_NAME}

make ${MAKE_TARGET}

cp ${MAKE_OBJECTS} ${PROJECT_SRC_DIR}

echo "Built '${MAKE_TARGET}' for ${PROJECT_NAME} in '$(pwd)' and copied results '${MAKE_OBJECTS}' back into source dir ${PROJECT_SRC_DIR}"
