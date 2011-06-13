#!/bin/bash -e
# This script creates and uploads new source packages to Launchpad PPA.
# It can do stable releases as well as testing versions.

# TODO: Script doesn't work yet
echo "Script doesn't work yet!"
exit 1

SUITES="natty maverick lucid"

usage()
{
	echo "Possible parameters:"
	echo "  -h, --help            print this help"
	echo "  -u, --upload          upload to ppa (otherwise just create packages)"
	echo "  -d, --dir WORKINGDIR  use given WORKINGDIR instead of random name in /tmp"
	echo "  -t, --tag TAGNAME     use the given TAGNAME, implies stable ppa"
	echo "If no tag is given, git master is used and uploaded to testing ppa."
}

# Parse args
until [ -z "$1" ]; do
	case "$1" in
		-h|--help)
			usage
			exit 0
			;;
		-u|--upload)
			UPLOAD=1
			;;
		-d|--dir)
			shift
			TEMPDIR="$1"
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

# Working directory
if [ ! "$TEMPDIR" ]; then
	TEMPDIR=`mktemp -dt stuntrally-ppa.XXXXXXXXXX`
else
	mkdir -p "$TEMPDIR"
fi

# Create the sources only if they don't exist already
#TODO: Only do this if they aren't there already
SOURCESCMDLINE="-d $TEMPDIR --bz2"
if [ "$TAG" ]; then
	SOURCESCMDLINE="$SOURCESCMDLINE -t $TAG"
fi
./create-clean-sources.sh $SOURCESCMDLINE

cd "$TEMPDIR"/stuntrally*

# Setup some variable depending on ppa type
if [ "$TAG" ]; then
	message="New upstream release $TAG"
	version="$TAG"
	longversion="$version-0" #FIXME
else
	headcommit=`cat gitcommit | head -1 | cut -c 1-10` # 10 chars is enough
	message="Development snapshot from Git $headcommit."
	version="1.5" #FIXME
	longversion="1.5-git" #FIXME
fi


#TODO: Mangle dir and package names

# Copy debian dir
cp -r dist/debian debian

# Loop the different distros
for distro in $SUITES; do

	#FIXME: Update changelog
	if [ "$TAG" ]; then
		dch -b -v $version -D $distro "$message"
	else
		dch -b -v $version -D $distro "$message"
	fi

	# Create package
	dpkg-buildpackage -sa -S   # Full orig
	cd ..

	# Upload
	if [ "$UPLOAD" ]; then
		if [ "$TAG" ]; then
			dput ppa:stuntrally-team/testing stuntrally_*_source.changes
		else
			dput ppa:stuntrally-team/stable stuntrally_*_source.changes
		fi
	fi

	#TODO: Clean-up
done
