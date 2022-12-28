; Script generated by the Inno Script Studio Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "TICK"
#define MyAppVersion "0.6.0"
#define MyAppPublisher "Tal Aviram"
#define MyAppURL "https://www.talaviram.com"
#define MyAppExeName "TICK.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{9A1B3649-DAA3-4A79-9B60-2759E780B582}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
DisableDirPage=yes
OutputBaseFilename=Setup
Compression=lzma
SolidCompression=yes
DisableWelcomePage=False

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\Builds\VisualStudio2019\x64\Release\Standalone Plugin\TICK.exe"; DestDir: "{autopf}\{#MyAppName}"; Flags: ignoreversion; Components: standalone
Source: "..\Builds\VisualStudio2019\x64\Release\VST3\TICK.vst3"; DestDir: "{commoncf}\VST3"; Flags: ignoreversion; Components: vst3
Source: "..\Builds\VisualStudio2019\x64\Release\VST\TICK.dll"; DestDir: "{code:GetVST2_64bit_Dir}"; Flags: ignoreversion; Components: vst2
Source: "..\Builds\VisualStudio2019\x64\Release\AAX\TICK.aaxplugin"; DestDir: "{commoncf}\Avid\Audio\Plug-Ins"; Flags: ignoreversion recursesubdirs; Components: aax
Source: "Factory\*"; DestDir: "{commondocs}\Tal Aviram\TICK\Presets\Factory"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{autopf}\{#MyAppName}\TICK.EXE";
Name: "{group}\Uninstall My Program"; Filename: "{uninstallexe}"

[Run]
Filename: "{autopf}\{#MyAppName}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Components]
Name: "standalone"; Description: "Standalone"; Types: full custom
Name: "vst3"; Description: "VST3"; Types: full custom
Name: "aax"; Description: "AAX"; Types: full custom
Name: "vst2"; Description: "VST"; Types: custom

[Code]
var
  VST2_64bit_InputDirPage: TInputDirWizardPage;

procedure InitializeWizard;
begin
  VST2_64bit_InputDirPage :=
    CreateInputDirPage(wpSelectComponents, 'VST2 Folder', 'VST2 Folder', 'Select folder used by your host for VST2. This can differ between hosts.', False, '');
  VST2_64bit_InputDirPage.Add('');
  VST2_64bit_InputDirPage.Values[0] := ExpandConstant('{commoncf}\VST2');
end;

function ShouldSkipPage(PageID: Integer): Boolean;
begin
  Result := False;
  if PageID = VST2_64bit_InputDirPage.ID then
    Result := not WizardIsComponentSelected('vst2');
end;

function GetVST2_64bit_Dir(Param: String): String;
begin
  Result := VST2_64bit_InputDirPage.Values[0];
end;
