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
	echo "  -p, --pparev NUM      PPA package revision, defaults to 1 (i.e. -ppa1)"
	echo "  -t, --tag TAGNAME     use the given TAGNAME, implies stable ppa"
	echo "If no tag is given, git master is used and uploaded to testing ppa."
}

error()
{
	echo "Error: $@"
	exit 1
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
		-p|--pparev)
			shift
			PPAREV="$1"
			;;
		-t|--tag)
			shift
			TAG=$1
			;;
		*)
			echo "Unrecognized argument: $1"
			usage
			exit 2
	esac
	shift
done

# Check that some required env vars exists
if [ x"$DEBFULLNAME" = x"" ]; then
	error "Environment variable DEBFULLNAME needs to be set to your full name."
elif [ x"$DEBEMAIL" = x"" ]; then
	error "Environment variable DEBEMAIL needs to be set to your email address."
fi

# PPA package revision
if [ x"$PPAREV" = x"" ]; then
	PPAREV=1
fi

# Working directory
if [ ! "$TEMPDIR" ]; then
	TEMPDIR=`mktemp -dt stuntrally-ppa.XXXXXXXXXX`
else
	mkdir -p "$TEMPDIR"
fi

# Create the sources only if they don't exist already
#TODO: Only do this if they aren't there already
SOURCESCMDLINE="-l -d $TEMPDIR --bz2"
if [ "$TAG" ]; then
	SOURCESCMDLINE="$SOURCESCMDLINE -t $TAG"
fi
./create-clean-sources.sh $SOURCESCMDLINE

cd "$TEMPDIR"/stuntrally*

# Setup some variables depending on ppa type
if [ "$TAG" ]; then
	version="$TAG"
	message="New upstream release $TAG"
	longversion="$version-0-ppa$PPAREV"
else
	version=`cat gitcommit | head -1 | cut -d- -f1`
	message="Development snapshot from Git $version."
	longversion="1.5-git" #FIXME
fi
packagingdate=`date -R`

mv stuntrally*.tar.bz2 stuntrally_$version

# Copy debian dir
cp -r dist/debian debian

# Loop the different distros
ONLYDIFF=""
for distro in $SUITES; do

	# Create changelog
	rm -f debian/changelog
	cat >> debian/changelog << EOF
stuntrally (${version}~${distro}) $distro; urgency=low

  [ $DEBFULLNAME ]
  * $message

 -- $DEBFULLNAME <$DEBEMAIL>  $packagingdate

EOF
exit 1
	# Create package
	if [ ! "$ONLYDIFF" ]; then
		dpkg-buildpackage -sa -S   # Full orig
		ONLYDIFF=1
	else
		dpkg-buildpackage -sd -S   # Only diff
	fi

	# Upload
	if [ "$UPLOAD" ]; then
		(
			cd ..
			if [ "$TAG" ]; then
				dput ppa:stuntrally-team/testing stuntrally_*_source.changes
			else
				dput ppa:stuntrally-team/stable stuntrally_*_source.changes
			fi
			# Clean up
			rm *.dsc *.changes *.upload *.debian.tar.*
		)
	fi

done
