#!/bin/bash -e
# Compiles SR and puts it into release archive file structure with deps.

BUILDDIR=build
STAGEDIR="`pwd`/stage"
THISSCRIPT=`readlink -f "$0"`
THISDIR=`dirname "$THISSCRIPT"`
SOURCEDIR="$THISDIR"/../..
if [ -z "$JOBS" ]; then
	JOBS=1
	#`nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1`
fi

if [ "$(uname -m)" = "x86_64" ]; then
	echo "64bit system"
	LIBPATH="$STAGEDIR/lib/64"
	EXEPOSTFIX="_x86_64"
else
	echo "32bit system"
	LIBPATH="$STAGEDIR/lib/32"
	EXEPOSTFIX="_x86"
fi

mkdir -p "$STAGEDIR/lib/32"
mkdir -p "$STAGEDIR/lib/64"
mkdir -p "$BUILDDIR"
cd "$BUILDDIR"

cmake "$SOURCEDIR" \
	-DCMAKE_BUILD_TYPE="Release" \
	-DCMAKE_INSTALL_PREFIX="$STAGEDIR" \
	-DSHARE_INSTALL="data"

# Data and binaries
make -j $JOBS
make install

# Libs
$THISDIR/copy-libs-linux.sh "$STAGEDIR/bin/stuntrally" "$LIBPATH"

echo "Doing some file shuffling..."

# Launcher scripts
cp "$THISDIR/stuntrally" "$STAGEDIR"
cp "$THISDIR/sr-editor" "$STAGEDIR"

# Rename binaries for correct arch
mv "$STAGEDIR/bin/stuntrally" "$STAGEDIR/bin/stuntrally${EXEPOSTFIX}"
mv "$STAGEDIR/bin/sr-editor" "$STAGEDIR/bin/sr-editor${EXEPOSTFIX}"

# Copy docs
cp "$SOURCEDIR/License.txt" "$STAGEDIR"
cp "$SOURCEDIR/Readme.txt" "$STAGEDIR"

# Move config
mkdir -p "$STAGEDIR/config"
mv "$STAGEDIR/data/config"/* "$STAGEDIR/config"
rmdir "$STAGEDIR/data/config"

echo "Done @ $STAGEDIR"

