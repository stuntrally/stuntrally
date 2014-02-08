#!/bin/bash -e
# This script figures out library dependencies of Stunt Rally on Linux
# and copies them to one place to easen the process of creating a portable
# binary archive.
#
# Usage: ./cpy-libs-linux.sh EXEPATH OUTDIR

if [ $# -ne 2 ]; then
	echo "Usage: $0 path-to-executable libs-out-path"
	exit 1
fi

FILE1=tmp-libs1.txt
FILE2=tmp-libs2.txt
EXEFILE="$1"
OUTDIR="$2"

mkdir -p "$OUTDIR"

# If lib name contains any of these, it will be assumed to be present in the system
BLACKLIST="libSM|libICE|libX|libxcb|libz\.so|libpthread|libwrap"

libs=`ldd "$EXEFILE" > $FILE1`                        # Get list of deps
libs=`grep -v " /lib" $FILE1 > $FILE2`                # Don't copy system libs in /lib
libs=`cut -d">" -f2 $FILE2 | cut -d" " -f2 > $FILE1`  # Get the library path field
libs=`grep -v "(0x" $FILE1 > $FILE2`                  # Remove pathless fields
libs=`egrep -v "$BLACKLIST" $FILE2 > $FILE1`          # Remove common system libs

# Copy libs
for i in `cat "$FILE1"`; do
	echo "Copying $i"
	cp "$i" "$OUTDIR"
done

# Find and copy OGRE plugins
copy_plugin()
{
	plugin=`locate "$1" | grep "/usr" | grep "\.so\." | head -1`
	if [ "$plugin" ]; then
		echo "Copying OGRE plugin $plugin"
		cp "$plugin" "$OUTDIR"
	else
		echo "Error: Could not find OGRE plugin $1"
		exit 1
	fi
}
copy_plugin RenderSystem_GL.so
copy_plugin Plugin_ParticleFX.so

# Strip all libs and exes
echo "Stripping libs"
strip "$OUTDIR"/*

rm "$FILE1" "$FILE2" # Remove temp files

echo "Libs copied"

