#!/usr/bin/env python
# This file was originally:
# NSIS script generator for Performous.
# Copyright (C) 2010 John Stumpo
# It was later modified and adapted by Tapio Vierros.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import os
import subprocess
import sys

# ------- !!!  this is old ver, use installer.nsi instead  !!! ---------
app = 'StuntRally'
version = '1.4'

if len(sys.argv) != 2:
    print >>sys.stderr, 'Usage: ' + sys.argv[0] + ' /path/to/stuntrally/installation'
    sys.exit(1)

workingdir = os.getcwd()
stagedir = sys.argv[1]

if not os.path.exists(stagedir):
    print >>sys.stderr, 'Given path does not exist: ' + stagedir
    sys.exit(1)

if not os.path.isdir(stagedir):
    print >>sys.stderr, 'Given path is not directory: ' + stagedir
    sys.exit(1)

os.chdir(stagedir)

try:
    makensis = subprocess.Popen([os.environ['MAKENSIS'], '-'], stdin=subprocess.PIPE)
except KeyError:
    makensis = subprocess.Popen(['makensis', '-'], stdin=subprocess.PIPE)

makensis.stdin.write(r'''!include "MUI2.nsh"

!define APP "%(app)s"
!define VERSION "%(version)s"

Name "${APP} ${VERSION}"
OutFile "%(workingdir)s/${APP}-${VERSION}.exe"

SetCompressor /SOLID lzma

ShowInstDetails show
ShowUninstDetails show

InstallDir "$PROGRAMFILES\${APP}"
InstallDirRegKey HKLM "Software\${APP}" ""

RequestExecutionLevel admin

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section
''' % {'app': app, 'version': version, 'workingdir': workingdir})

for root, dirs, files in os.walk('.'):
	makensis.stdin.write('  SetOutPath "$INSTDIR\\%s"\n' % root.replace('/', '\\'))
	for file in files:
		makensis.stdin.write('  File "%s"\n' % os.path.join(stagedir, root, file).replace('/', '\\'))

makensis.stdin.write(r'''  WriteRegStr HKLM "Software\${APP}" "" "$INSTDIR"
  WriteUninstaller "$INSTDIR\uninst.exe"
  SetOutPath "$INSTDIR"
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\${APP}"
  CreateShortcut "$SMPROGRAMS\${APP}\${APP}.lnk" "$INSTDIR\${APP}.exe"
  CreateShortcut "$SMPROGRAMS\${APP}\Stunt Rally Editor.lnk" "$INSTDIR\SR-Editor.exe"
  CreateShortcut "$SMPROGRAMS\${APP}\Uninstall.lnk" "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP}" "DisplayName" "${APP}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP}" "UninstallString" "$\"$INSTDIR\uninst.exe$\""
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP}" "DisplayIcon" "$INSTDIR\${APP}.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP}" "DisplayVersion" "${VERSION}"
SectionEnd

Section Uninstall
''')

for root, dirs, files in os.walk('.', topdown=False):
	for dir in dirs:
		makensis.stdin.write('  RmDir "$INSTDIR\\%s"\n' % os.path.join(root, dir).replace('/', '\\'))
	for file in files:
		makensis.stdin.write('  Delete "$INSTDIR\\%s"\n' % os.path.join(root, file).replace('/', '\\'))
	makensis.stdin.write('  RmDir "$INSTDIR\\%s"\n' % root.replace('/', '\\'))

makensis.stdin.write(r'''  Delete "$INSTDIR\uninst.exe"
  RmDir "$INSTDIR"
  SetShellVarContext all
  Delete "$SMPROGRAMS\${APP}\${APP}.lnk"
  Delete "$SMPROGRAMS\${APP}\Stunt Rally Editor.lnk"
  Delete "$SMPROGRAMS\${APP}\Uninstall.lnk"
  RmDir "$SMPROGRAMS\${APP}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP}"
  DeleteRegKey /ifempty HKLM "Software\${APP}"
SectionEnd
''')

makensis.stdin.close()
if makensis.wait() != 0:
	print >>sys.stderr, 'Installer compilation failed.'
	sys.exit(1)
else:
	print 'Installer ready.'

