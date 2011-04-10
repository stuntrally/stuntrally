#!/bin/bash -e
# This script is meant for easily creating clean source packages.
# It does the following:
# * Fetches fresh sources from Git
# * Optionally can ignore data and tracks
# * Removes all Git stuff (.git*)
# * Optionally creates an archive
#
# TODO: Allow copying files from a local source
# TODO: Other archive formats?


usage()
{
	echo "Possible parameters (all optional):"
	echo "  -h, --help             print this help"
	echo "  -t, --tag TAGNAME      use the given TAGNAME"
	echo "  -s, --source-only      don't download tracks"
	echo "  -b, --bz2              create tar.bz2 archive"
	echo "  -d, --dir WORKINGDIR   use given WORKINGDIR instead of random name in /tmp"
}

# Parse args
until [ -z "$1" ]; do
	case "$1" in
		-h|--help)
			usage
			exit 0
			;;
		-t|--tag)
			shift
			TAG="$1"
			;;
		-s|--source-only)
			NOTRACKS=1
			;;
		-b|--bz2)
			BZ2=1
			;;
		-d|--dir)
			shift
			TEMPDIR="$1"
			;;
		*)
			echo "Unrecognized argument: $1"
			usage
			exit 1
	esac
	shift
done

clone_cd_and_purge()
{
	echo "Cloning $2..."
	git clone "$1" "$2"
	cd "$2"
	if [ "$TAG" ]; then
		echo "Changing to tag $TAG..."
		git checkout "$TAG"
	fi
	echo "Deleting Git stuff..."
	rm -rf .git* # Purge mercurial stuff
}

# Working directory
if [ ! "$TEMPDIR" ]; then
	TEMPDIR=`mktemp -dt stuntrally-sources$TAG.XXXXXXXXXX`
else
	mkdir -p "$TEMPDIR"
fi

# Check that the destination is clean
if [ -d "$TEMPDIR/stuntrally" ]; then
	echo "Directory $TEMPDIR/stuntrally already exists, aborting!"
	exit 1
fi

echo "Working in $TEMPDIR"
cd "$TEMPDIR"

# Fetch the sources
(
	clone_cd_and_purge git://github.com/stuntrally/stuntrally.git stuntrally

	if [ ! "$NOTRACKS" ]; then
		(
			cd data
			clone_cd_and_purge git://github.com/stuntrally/tracks.git tracks
		)
	fi
)

# Create archive
if [ "$BZ2" ]; then
	if [ "$TAG" ]; then
		ARCHIVENAME="stuntrally-$TAG-sources.tar.bz2"
	else
		ARCHIVENAME="stuntrally-sources.tar.bz2"
	fi
	tar -cvjf "$ARCHIVENAME" stuntrally
	echo "Created archive $TEMPDIR/$ARCHIVENAME"
fi

echo "All done, files at $TEMPDIR"

