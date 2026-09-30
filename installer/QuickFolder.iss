; QuickFolder Inno Setup Script
; High-performance, lightweight Windows Explorer context-menu utility

#define MyAppName "QuickFolder"
#define MyAppVersion "0.2.0"
#define MyAppPublisher "QuickFolder Open Source Project"
#define MyAppURL "https://github.com/QuickFolder/QuickFolder"
#define MyAppExeName "QuickFolder.exe"

[Setup]
AppId={{4B2F7E41-8A19-4C1B-94E5-46D9B6E1C87D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={localappdata}\Programs\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir=..\dist
OutputBaseFilename=QuickFolder-Setup-0.2.0
SetupIconFile=..\res\QuickFolder.ico
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\{#MyAppExeName}
ChangesAssociations=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Files]
Source: "..\dist\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\QuickFolder"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"

[Run]
; Register shell context menu and COM Local Server silently upon installation
Filename: "{app}\{#MyAppExeName}"; Parameters: "--register --silent"; Flags: runhidden; Description: "Register context menu extension"

[UninstallRun]
; Unregister shell context menu and COM server cleanly before file removal
Filename: "{app}\{#MyAppExeName}"; Parameters: "--unregister --silent"; Flags: runhidden
