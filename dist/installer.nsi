
; the path where to whole game binaries with data
; CHANGE to yours when starting script
!define BINARY_DIR "d:\sr\"
; also change Release Version
!define PRODUCT_VERSION "2.6"

; redist path should contain both vcredist_x86.exe and
; DirectX files: DSETUP.dll, dsetup32.dll, DXSETUP.exe, dxupdate.cab
; and (depending on which DX SDK was used when compiling OGRE)
; Jun2010_D3DCompiler_43_x86.cab, Jun2010_d3dx*_43_x86.cab
!define REDIST_DIR "d:\redist"


; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "Stunt Rally"
!define PRODUCT_PUBLISHER "Crystal Hammer"
!define PRODUCT_WEB_SITE "http://stuntrally.tuxfamily.org/"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\StuntRally.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define INST_SR_DIR "${PRODUCT_NAME} ${PRODUCT_VERSION}"

SetCompressor lzma

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install-blue.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall-blue.ico"

; Language Selection Dialog Settings
!define MUI_LANGDLL_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "NSIS:Language"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "${BINARY_DIR}\License.txt"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\StuntRally.exe"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\Readme.txt"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Finnish"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "PortugueseBR"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Romanian"

; Reserve files
!insertmacro MUI_RESERVEFILE_LANGDLL
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "StuntRally-${PRODUCT_VERSION}-installer.exe"
InstallDir "$PROGRAMFILES\${INST_SR_DIR}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

Section "Redist" SEC01
  SetOutPath "$INSTDIR\redist"
  SetOverwrite try
  File "${REDIST_DIR}\*.*"
  DetailPrint "Updating DirectX this may take a few moments..."
  ExecWait "$INSTDIR\redist\dxsetup.exe /silent"
  DetailPrint "Installing VC redistributable..."
  ExecWait "$INSTDIR\redist\vcredist_x86.exe"
SectionEnd

Section "StuntRally" SEC02
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File /r "${BINARY_DIR}"
  CreateDirectory "$SMPROGRAMS\${INST_SR_DIR}"
  CreateShortCut "$SMPROGRAMS\${INST_SR_DIR}\Stunt Rally.lnk" "$INSTDIR\StuntRally.exe"
  CreateShortCut "$SMPROGRAMS\${INST_SR_DIR}\SR Track Editor.lnk" "$INSTDIR\SR-Editor.exe"
  ;CreateShortCut "$DESKTOP\Stunt Rally.lnk" "$INSTDIR\StuntRally.exe"
SectionEnd

Section -AdditionalIcons
  SetOutPath $INSTDIR
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\${INST_SR_DIR}\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\${INST_SR_DIR}\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\StuntRally.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\StuntRally.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) has been succesfully removed."
FunctionEnd

Function un.onInit
!insertmacro MUI_UNGETLANGUAGE
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Do you want to remove $(^Name) ?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  Delete "$SMPROGRAMS\${INST_SR_DIR}\*.lnk"
  RMDir "$SMPROGRAMS\${INST_SR_DIR}"
  ;Delete "$DESKTOP\Stunt Rally.lnk"

  RMDir /r "$INSTDIR\*.*"
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
