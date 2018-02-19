#!/bin/bash

usage="$(basename "$0") [-h] [-c] [-d] -- builds the source of carolocup

where:
    -h  show this help text
    -c  clean build directory
    -d  turn on debug information"

DEBUG=false

while getopts '::chd' option; do
   case "$option" in
      h) echo "$usage"
         exit
         ;;
      c) rm -rf build bin
         ;;
      d) DEBUG=true
         ;;
      :) printf "missing argument for -%s\n" "$OPTARG" >&2
         echo "$usage" >&2
         exit 1
         ;;
     \?) printf "illegal option: -%s\n" "$OPTARG" >&2
         echo "$usage" >&2
         exit 1
         ;;
   esac
done
shift $((OPTIND - 1))

if [ $DEBUG = true ]; then 
   echo "Options processed" >&2;
fi

mkdir -p build bin

if [ $DEBUG = true ]; then 
   echo "Folders build and bin created" >&2;
fi

cd build

if [ $DEBUG = true ]; then 
   echo "Call cmake" >&2;
fi

cmake -DDEBUG:BOOL=$DEBUG ../src

if [ $DEBUG = true ]; then 
   echo "Call make" >&2;
fi

make

if [ $DEBUG = true ]; then 
   echo "Copy executable to bin" >&2;
fi

cp carolocup ../bin/carolocup
