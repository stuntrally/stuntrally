#!/bin/bash -e

SUITES="natty maverick lucid"

usage()
{
	echo "Usage:   $0 [ PARAMETERS ] WORKDIR PPA VERSION"
	echo "Example: $0 --release /tmp/stuntrally-ppa stable 1.3-0-ppa1"
	echo
	echo "This script assumes you have new, clean sources in WORKDIR/stuntrally* and"
	echo "orig archive in WORKDIR"
	echo
	echo "Mandatory arguments:"
	echo "  WORKDIR               path to the sources"
	echo "  PPA                   stable | testing"
	echo "  VERSION               full package version without distro suffix"
	echo
	echo "Optional arguments:"
	echo "  -h, --help            print this help and exit"
	echo "  -r, --release         this is a release, not a dev snapshot"
	echo "  -d, --diff            upload only diff, no orig archive"
}

# Parse args
until [ -z "$1" ]; do
	case "$1" in
		-h|--help)
			usage
			exit 0
			;;
		-r|--release)
			RELEASE=1
			;;
		-d|--diff)
			ONLYDIFF=1
			;;
		*)
			if [ x"$WORKDIR" = x"" ]; then
				WORKDIR="$1"
			elif [ x"$PPA" = x"" ]; then
				PPA="$1"
			else
				VERSION="$1"
			fi
	esac
	shift
done

# Validate command line
if [ x"$VERSION" = x"" ]; then
	echo "Error: Invalid command line parameters."
	echo
	usage
	exit 1
fi
if [ ! -d "$WORKDIR" ]; then
	echo "Error: Working dir \"$WORKDIR\" does not exist."
	exit 1
fi

# Check that some required env vars exists
if [ x"$DEBFULLNAME" = x"" ]; then
	echo "Error: Environment variable DEBFULLNAME needs to be set to your full name."
	exit 1
elif [ x"$DEBEMAIL" = x"" ]; then
	echo "Error: Environment variable DEBEMAIL needs to be set to your email address."
	exit 1
fi

# Go to root of sources
cd "$WORKDIR"/stuntrally*

# Setup some variables depending on ppa type
if [ "$RELEASE" ]; then
	message="New upstream release."
else
	gitversion=`cat gitcommit | head -1`
	message="Development snapshot from Git $gitversion."
fi
packagingdate=`date -R`

# Loop the different distros
for distro in $SUITES; do

	# Copy fresh debian dir
	rm -rf debian .pc
	cp -r dist/debian debian

	# Create changelog
	rm -f debian/changelog
	cat >> debian/changelog << EOF
stuntrally (${VERSION}~${distro}) $distro; urgency=low

  [ $DEBFULLNAME ]
  * $message

 -- $DEBFULLNAME <$DEBEMAIL>  $packagingdate

EOF

	# Create package
	if [ x"$ONLYDIFF" = x"" ]; then
		dpkg-buildpackage -sa -S   # Full orig
		ONLYDIFF=1
	else
		dpkg-buildpackage -sd -S   # Only diff
	fi

	# Upload
	(
		cd ..
		dput ppa:stuntrally-team/$PPA stuntrally_*_source.changes

		# Clean up
		rm -f *.dsc *.changes *.upload *.debian.tar.*
	)

done
