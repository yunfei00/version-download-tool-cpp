#define MyAppName "VersionDownloadTool"
#define MyAppDisplayName "版本下载工具"
#define MyAppExeName "VersionDownloadTool.exe"

[Setup]
AppId={{5B53AB6D-39D6-4D23-8A47-87C43E6F7404}
AppName={#MyAppName}
AppVersion=1.0.0
AppVerName={#MyAppName}
DefaultDirName={autopf}\VersionDownloadTool
DefaultGroupName={#MyAppDisplayName}
OutputDir=.
OutputBaseFilename=VersionDownloadTool-Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
DisableProgramGroupPage=yes

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "package\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autodesktop}\{#MyAppDisplayName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autoprograms}\{#MyAppDisplayName}"; Filename: "{app}\{#MyAppExeName}"

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "启动 {#MyAppDisplayName}"; Flags: nowait postinstall skipifsilent
