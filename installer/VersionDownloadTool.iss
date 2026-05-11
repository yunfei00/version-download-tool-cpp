[Setup]
AppName=VersionDownloadTool
AppVersion=0.2.0
DefaultDirName={autopf}\VersionDownloadTool
DefaultGroupName=VersionDownloadTool
OutputDir=.
OutputBaseFilename=VersionDownloadTool-Setup
Compression=lzma
SolidCompression=yes

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\package\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\版本下载工具"; Filename: "{app}\VersionDownloadTool.exe"
Name: "{commondesktop}\版本下载工具"; Filename: "{app}\VersionDownloadTool.exe"

[Run]
Filename: "{app}\VersionDownloadTool.exe"; Description: "启动版本下载工具"; Flags: nowait postinstall skipifsilent
