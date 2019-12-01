unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, PSAPI, TlHelp32, ShellApi, ExtCtrls, ComCtrls, XPMan,
  ImgList, Menus, IniFiles;

type
  TMain = class(TForm)
    ListView: TListView;
    RefreshBtn: TButton;
    ImageList: TImageList;
    XPManifest1: TXPManifest;
    PopupMenu: TPopupMenu;
    CloseBtn: TMenuItem;
    N1: TMenuItem;
    AboutBtn: TMenuItem;
    N2: TMenuItem;
    ShowHideBtn: TMenuItem;
    SelectLibCB: TComboBox;
    HideBtn: TButton;
    procedure RefreshBtnClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure CloseBtnClick(Sender: TObject);
    procedure AboutBtnClick(Sender: TObject);
    procedure ShowHideBtnClick(Sender: TObject);
    procedure ListViewMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure ListViewDblClick(Sender: TObject);
    procedure SelectLibCBChange(Sender: TObject);
    procedure HideBtnClick(Sender: TObject);
  private
    procedure WMNCHITTEST(var Msg: TMessage); message WM_NCHITTEST;
    procedure DefaultHandler(var Message); override;
    procedure AppShow;
    procedure AppHide;
    { Private declarations }
  public
  protected
    procedure CreateParams(var Params: TCreateParams); override;
    procedure IconMouse(var Msg: TMessage); message WM_USER + 1;
    procedure WMActivate(var Msg: TMessage); message WM_ACTIVATE;
    { Public declarations }
  end;

type
  PEnumInfo = ^TEnumInfo;
  TEnumInfo = record
    ProcessID: DWORD;
    HWND     : THandle;
  end;

var
  Main: TMain;
  WM_TaskBarCreated: Cardinal;
  RunOnce: boolean;
  HideProcessList: TStringList;

  IDS_FAIL_INJECTION, IDS_LAST_UPDATE: string;

const
  LibrariesFolder = 'Libraries';
  UtilitiesFolder = 'Utilities';

implementation

{$R *.dfm}

function GetPathFromPID(const PID: cardinal): string;
type
  TQueryFullProcessImageName = function(hProcess: Thandle; dwFlags: DWORD; lpExeName: PChar; nSize: PDWORD): BOOL; stdcall;
var
  hProcess: THandle;
  Path: array[0..MAX_PATH - 1] of char;
  QueryFullProcessImageName: TQueryFullProcessImageName;
  nSize: cardinal;
begin
  hProcess:=OpenProcess(PROCESS_QUERY_INFORMATION or PROCESS_VM_READ, False, PID);
  if hProcess <> 0 then
  try

  if GetModuleFileNameEx(hProcess, 0, Path, MAX_PATH) <> 0 then begin
    Result:=Path;
  end else if Win32MajorVersion >= 6 then begin
    nSize:=MAX_PATH;
    ZeroMemory(@Path, MAX_PATH);
    @QueryFullProcessImageName:=GetProcAddress(GetModuleHandle('kernel32'), 'QueryFullProcessImageNameW');
    if Assigned(QueryFullProcessImageName) then
      if QueryFullProcessImageName(hProcess, 0, Path, @nSize) then
        Result:=Path;
  end;

  finally
    CloseHandle(hProcess)
  end else
    RaiseLastOSError;
end;

function EnumWindowsProc(Wnd: HWND; Param: LPARAM): Bool; stdcall;
var
  PID: DWORD;
  PEI: PEnumInfo;
begin
  PEI:=PEnumInfo(Param);
  GetWindowThreadProcessID(Wnd, @PID);
  Result:=(PID <> PEI^.ProcessID) or (not IsWindowVisible(Wnd)) or (not IsWindowEnabled(Wnd));

  if not Result then
    PEI^.HWND:=Wnd;
end;

function FindMainWindow(PID: DWORD): DWORD;
var
  EI: TEnumInfo;
begin
  EI.ProcessID:=PID;
  EI.HWND:=0;
  EnumWindows(@EnumWindowsProc, LPARAM(@EI));
  Result:=EI.HWND;
end;

procedure UpdateLibs(SelectName: string);
var
  SR: TSearchRec; i: integer;
begin
  Main.SelectLibCB.Clear;

  if FindFirst(ExtractFilePath(ParamStr(0)) + LibrariesFolder + '\*.*', faDirectory, SR) = 0 then begin
    repeat
      if (SR.Name <> '.') and (SR.Name <> '..') then
        Main.SelectLibCB.Items.Add(SR.Name);
    until FindNext(SR) <> 0;
    FindClose(SR);
  end;

  Main.SelectLibCB.ItemIndex:=0;

  for i:=0 to Main.SelectLibCB.Items.Count - 1 do
    if Main.SelectLibCB.Items.Strings[i] = SelectName then
      Main.SelectLibCB.ItemIndex:=i;
end;

procedure TMain.RefreshBtnClick(Sender: TObject);
var
  hSnapShot: THandle;
  ProcInfo: TProcessEntry32;

  Window: DWORD;
  WindowTitle: array [0..127] of Char;
  PIDPath: string;

  Icon: TIcon;
  Item: TListItem;
begin
  ListView.Clear;
  hSnapShot:=CreateToolHelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapShot <> THandle(-1)) then begin
    ProcInfo.dwSize:=SizeOf(ProcInfo);

    if (Process32First(hSnapshot, ProcInfo)) then

      while Process32Next(hSnapShot, ProcInfo) do begin

        //Пропускаем исключённые процессы
        if Pos(AnsiLowerCase(string(ProcInfo.szExeFile)), HideProcessList.Text) > 0 then Continue;

        Window:=FindMainWindow(ProcInfo.th32ProcessID);

        if IsWindowVisible(Window) and (Window <> Handle) then begin

          GetWindowText(Window, WindowTitle, SizeOf(WindowTitle));
          PIDPath:=GetPathFromPID(ProcInfo.th32ProcessID);
          Icon:=TIcon.Create;
          Icon.Handle:=ExtractIcon(HInstance, PChar(PIDPath), 0);

          Item:=ListView.Items.Add;
          Item.SubItems.Add(string(ProcInfo.szExeFile));
          Item.SubItems.Add(StrPas(WindowTitle));
          Item.SubItems.Add(PIDPath);
          Item.ImageIndex:=ImageList.AddIcon(Icon);

          Icon.Free;
        end;

      end;
    CloseHandle(hSnapShot);
  end;
  //UpdateLibs(SelectLibCB.Items.Strings[SelectLibCB.ItemIndex]);
end;

procedure Tray(ActInd: integer);  //1 - добавить, 2 - удалить, 3 - заменить
var
  NIM: TNotifyIconData;
begin
  with NIM do begin
    cbSize:=SizeOf(NIM);
    Wnd:=Main.Handle;
    uId:=1;
    uFlags:=NIF_MESSAGE or NIF_ICON or NIF_TIP;
    hIcon:=SendMessage(Application.Handle, WM_GETICON, ICON_SMALL2, 0);
    uCallBackMessage:=WM_USER + 1;
    StrCopy(szTip, PChar(Application.Title));
  end;
  case ActInd of
    1: Shell_NotifyIcon(NIM_ADD, @NIM);
    2: Shell_NotifyIcon(NIM_DELETE, @NIM);
    3: Shell_NotifyIcon(NIM_MODIFY, @NIM);
  end;
end;

procedure TMain.DefaultHandler(var Message);
begin
  if TMessage(Message).Msg = WM_TASKBARCREATED then
    Tray(1);
  inherited;
end;

function GetLocaleInformation(Flag: Integer): string;
var
  pcLCA: array [0..20] of Char;
begin
  if GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, Flag, pcLCA, 19) <= 0 then
    pcLCA[0]:=#0;
  Result:=pcLCA;
end;

procedure TMain.FormCreate(Sender: TObject);
var
  Ini: TIniFile;
begin
  Ini:=TIniFile.Create(ExtractFilePath(ParamStr(0)) + 'Setup.ini');
  UpdateLibs(Ini.ReadString('Main', 'CurrentLibrary', ''));
  Ini.Free;

  HideProcessList:=TStringList.Create;
  if FileExists(ExtractFilePath(ParamStr(0)) + 'HiddenProcesses.txt') then
    HideProcessList.LoadFromFile(ExtractFilePath(ParamStr(0)) + 'HiddenProcesses.txt');
  HideProcessList.Text:=AnsiLowerCase(HideProcessList.Text);

  Application.Title:=Caption;
  WM_TaskBarCreated:=RegisterWindowMessage('TaskbarCreated');
  Tray(1);
  SetWindowLong(Application.Handle, GWL_EXSTYLE, GetWindowLong(Application.Handle, GWL_EXSTYLE) or WS_EX_TOOLWINDOW);
  Height:=ListView.Height + RefreshBtn.Height + 30;

  IDS_FAIL_INJECTION:='Failed to make an injection.';
  IDS_LAST_UPDATE:='Last update:';
  if GetLocaleInformation(LOCALE_SENGLANGUAGE) = 'Russian' then begin
    ListView.Columns[1].Caption:='Имя';
    ListView.Columns[2].Caption:='Заголовок';
    ListView.Columns[3].Caption:='Расположение';
    RefreshBtn.Caption:='Обновить';
    HideBtn.Caption:='Скрыть';
    IDS_FAIL_INJECTION:='Не удалось внедриться в процесс.';
    ShowHideBtn.Caption:='Показать / скрыть';
    AboutBtn.Caption:='О программе...';
    IDS_LAST_UPDATE:='Последнее обновление:';
    CloseBtn.Caption:='Выход';
  end;
end;

procedure TMain.IconMouse(var Msg: TMessage);
begin
  case Msg.LParam of
    WM_LBUTTONDOWN:
      if IsWindowVisible(Main.Handle) then AppHide else AppShow;

    WM_RBUTTONDOWN:
      PopupMenu.Popup(Mouse.CursorPos.X, Mouse.CursorPos.Y);
  end;
end;

procedure TMain.FormDestroy(Sender: TObject);
begin
  Tray(2);
  HideProcessList.Free;
end;

procedure TMain.CloseBtnClick(Sender: TObject);
begin
  Close;
end;

procedure TMain.CreateParams(var Params: TCreateParams);
begin
  inherited;
  Params.Style:=WS_POPUP or WS_THICKFRAME;
end;

procedure TMain.WMNCHITTEST(var Msg: TMessage);
begin
  Msg.Result:=HTCLIENT;
end;

procedure TMain.AboutBtnClick(Sender: TObject);
begin
  Application.MessageBox(PChar(Caption + ' 0.2.1' + #13#10 +
  IDS_LAST_UPDATE + ' 01.12.2019' + #13#10 +
  'https://r57zone.github.io' + #13#10 +
  'r57zone@gmail.com'), PChar(AboutBtn.Caption), MB_ICONINFORMATION);
end;

procedure TMain.ShowHideBtnClick(Sender: TObject);
begin
  if IsWindowVisible(Main.Handle) then AppHide else AppShow;
end;

procedure TMain.WMActivate(var Msg: TMessage);
begin
  if Msg.WParam = WA_INACTIVE then
    AppHide;
  inherited;
end;

procedure TMain.AppHide;
begin
  ShowWindow(Handle, SW_HIDE);
end;

procedure TMain.AppShow;
begin
  if RunOnce = false then begin
    Main.AlphaBlendValue:=255;
    Main.AlphaBlend:=false;
    RunOnce:=true;
  end;
  Top:=Screen.Height - Main.Height - 54;
  Left:=Screen.Width - Main.Width - 8;
  ShowWindow(Handle, SW_SHOW);
  SetForegroundWindow(Handle);
  RefreshBtn.Click;
end;

procedure TMain.ListViewMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
var
  Item: TListItem;
begin
  if ListView.Selected <> nil then begin
    Item:=ListView.Items.Item[ListView.Selected.Index];
    if Button = mbRight then
      ShellExecute(0, 'open', 'explorer', PChar('/select, "' + Item.SubItems[2] + '"'), nil, SW_SHOW);
  end;
end;

function RunProcess(ProcessPath: string): Cardinal;
var
  StartInfo: TSTARTUPINFO;
  ProcInfo: TPROCESSINFORMATION;
begin
  GetStartupInfo(StartInfo);
  if CreateProcess(nil, PChar(ProcessPath), nil, nil, false, CREATE_NO_WINDOW, nil, nil, StartInfo, ProcInfo) then begin
    CloseHandle(ProcInfo.hThread);
    WaitForSingleObject(ProcInfo.hProcess, INFINITE);
    GetExitCodeProcess(ProcInfo.hProcess, Result);
    CloseHandle(ProcInfo.hProcess);
  end else
    Result:=1;
end;

procedure TMain.ListViewDblClick(Sender: TObject);
var
  Item: TListItem;
  BinTypeProcess: Cardinal;
  ProcessStatus: integer;
begin
  if ListView.Selected <> nil then begin
    Item:=ListView.Items.Item[ListView.Selected.Index];
    ProcessStatus:=0;
    if GetBinaryType(PAnsiChar(Item.SubItems[2]), BinTypeProcess) then
      case BinTypeProcess of
        //32-bit
        0: ProcessStatus:=RunProcess(UtilitiesFolder + '\Injector.exe -n ' + Item.SubItems[0] + ' -i ' + '..\' + LibrariesFolder + '\' + SelectLibCB.Items.Strings[SelectLibCB.ItemIndex] + '\XInputInject.dll');
        //64-bit
        6: ProcessStatus:=RunProcess(UtilitiesFolder + '\Injector64.exe -n ' + Item.SubItems[0] + ' -i ' + '..\' + LibrariesFolder + '\' + SelectLibCB.Items.Strings[SelectLibCB.ItemIndex] + '\XInputInject64.dll');
      end;
    if ProcessStatus = 1 then
      MessageBox(0, PChar(IDS_FAIL_INJECTION), PChar(Caption), MB_ICONWARNING);
    AppHide;
  end;
end;

procedure TMain.SelectLibCBChange(Sender: TObject);
var
  Ini: TIniFile;
begin
  if SelectLibCB.Items.Strings[SelectLibCB.ItemIndex] <> '' then begin
    Ini:=TIniFile.Create(ExtractFilePath(ParamStr(0)) + 'Setup.ini');
    Ini.WriteString('Main', 'CurrentLibrary', SelectLibCB.Items.Strings[SelectLibCB.ItemIndex]);
    Ini.Free;
  end;
end;

procedure TMain.HideBtnClick(Sender: TObject);
begin
  AppHide;
end;

end.
