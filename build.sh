#!/bin/bash

exit_show() 
{
    read -n 1 -s -r -p "Press any key to exit...
"
    exit
}

usage="$(basename "$0") [-h] [-c] [-d] -- builds the source of carolocup

where:
    -h  show this help text
    -c  clean build directory
    -d  turn on debug information"

DEBUG=false
OS=unknown

while getopts '::chd' option; do
    case "$option" in
		h) echo "$usage" >&2;
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

case "$OSTYPE" in
    linux*)  
    {
	OS=linux
        if [ $DEBUG = true ]; then 
            echo "Linux OS detected" >&2;
        fi
    } ;;
    msys*)
    {	
	OS=win
        if [ $DEBUG = true ]; then 
            echo "Windows OS detected" >&2;
        fi
	} ;;
    *)       
    {
        echo "Unknown OS" >&2;
    } ;;
esac

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


if [ $OS = "linux" ]; then 
    cmake -DDEBUG:BOOL=$DEBUG -DLINUX:BOOL=true ../src
elif [ $OS = "win" ]; then 
    cmake -DDEBUG:BOOL=$DEBUG -DLINUX:BOOL=false ../src
fi


if [ $DEBUG = true ]; then 
    echo "Call make/build" >&2;
fi

if [ $OS = "linux" ]; then 
    make
    cp carolocup ../bin/carolocup
elif [ $OS = "win" ]; then 
    cmake --build . --target ALL_BUILD --config Release
    cp Release/carolocup.exe ../bin/carolocup.exe
fi