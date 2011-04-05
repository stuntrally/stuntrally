#!/bin/bash -e
# This script creates and uploads new source packages to Launchpad PPA.
# It can do stable releases as well as testing versions.

# TODO: Script doesn't work yet
echo "Script doesn't work yet!"
exit 1

SUITES="maverick lucid"

usage()
{
	echo "Possible parameters:"
	echo "  -h, --help            print this help"
	echo "  -t, --tag TAGNAME     use the given TAGNAME, implies stable ppa"
	echo "If no tag is given, hg tip is used and uploaded to testing ppa."
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
			TAG=$1
			;;
		*)
			echo "Unrecognized argument: $1"
			usage
			exit 1
	esac
	shift
done

TEMPDIR=`mktemp -dt stuntrally-ppa.XXXXXXXXXX`

# Create the sources
SOURCESCMDLINE="-d \"$TEMPDIR\" --bz2"
if [ "$TAG" ]; then
	SOURCESCMDLINE="$SOURCESCMDLINE -t $TAG"
fi
./create-clean-sources.sh $SOURCESCMDLINE
#wget https://launchpad.net/~stuntrally-team/+archive/stable/+files/stuntrally_1.1-0.orig.tar.gz

cd "$TEMPDIR"

#TODO: Mangle dir and package names
#TODO: Copy debian dir

for distro in $SUITES; do

	#TODO: Update changelog

	# Create and upload package
	if [ "$TAG" ]; then
		dpkg-buildpackage -sd -S   # Only diff
		cd ..
		dput ppa:stuntrally-team/testing stuntrally_*_source.changes
	else
		dpkg-buildpackage -sd -S   # Full orig
		cd ..
		dput ppa:stuntrally-team/stable stuntrally_*_source.changes
	fi
	
	#TODO: Clean-up
done
