# Locate VorbisFile
# This module defines
# VorbisFile_LIBRARIES
# VorbisFile_FOUND, if false, do not try to link to VorbisFile
# VorbisFile_INCLUDE_DIRS, where to find the headers
#
# $VORBISDIR is an environment variable that would
# correspond to the ./configure --prefix=$VORBISDIR
# used in building Vorbis.
#
# Created by Sukender (Benoit Neil). Based on FindOpenAL.cmake module.
# TODO Add hints for linux and Mac

FIND_PATH(VorbisFile_INCLUDE_DIRS
	vorbis/vorbisfile.h
	HINTS
	$ENV{VORBISDIR}
    $ENV{CSP_DEVPACK}
	$ENV{VORBIS_PATH}
	PATH_SUFFIXES include
	PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)

FIND_LIBRARY(VorbisFile_LIBRARIES 
	NAMES vorbisfile
	HINTS
	$ENV{VORBISDIR}
	$ENV{VORBIS_PATH}
    $ENV{CSP_DEVPACK}
	PATH_SUFFIXES win32/VorbisFile_Dynamic_Release
	PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw
	/opt/local
	/opt/csw
	/opt
)

FIND_LIBRARY(VorbisFile_LIBRARIES_DEBUG 
	NAMES VorbisFile_d
	HINTS
	$ENV{VORBISDIR}
    $ENV{CSP_DEVPACK}
	$ENV{VORBIS_PATH}
	PATH_SUFFIXES win32/VorbisFile_Dynamic_Debug
	PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw
	/opt/local
	/opt/csw
	/opt
)


SET(VORBIS_FOUND "NO")
IF(VORBIS_LIBRARIES AND VORBIS_INCLUDE_DIRS)
	SET(VORBIS_FOUND "YES")
ENDIF(VORBIS_LIBRARIES AND VORBIS_INCLUDE_DIRS)

