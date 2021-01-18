unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Registry, ComCtrls, XPMan;

type
  TMain = class(TForm)
    ApplyBtn: TButton;
    XPManifest1: TXPManifest;
    CloseBtn: TButton;
    MouseGB: TGroupBox;
    XAxisSensitivityLbl: TLabel;
    YAxisSensitivityLbl: TLabel;
    XAxisSensitivityTB: TTrackBar;
    YAxisSensitivityTB: TTrackBar;
    SteeringWheelGB: TGroupBox;
    SensitivityAngleLbl: TLabel;
    SensitivityAngleValueLbl: TLabel;
    SensitivityAngleTB: TTrackBar;
    SetupGB: TGroupBox;
    COMPortLbl: TLabel;
    COMPortNumberEdt: TEdit;
    XAxisSensitivityValueLbl: TLabel;
    YAxisSensitivityValueLbl: TLabel;
    AboutBtn: TButton;
    CBOnlyTrigger: TCheckBox;
    TriggerSensitivityLbl: TLabel;
    TriggerSensitivityTB: TTrackBar;
    TriggerSensitivityValueLbl: TLabel;
    procedure ApplyBtnClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure XAxisSensitivityTBChange(Sender: TObject);
    procedure YAxisSensitivityTBChange(Sender: TObject);
    procedure SensitivityAngleTBChange(Sender: TObject);
    procedure AboutBtnClick(Sender: TObject);
    procedure CloseBtnClick(Sender: TObject);
    procedure TriggerSensitivityTBChange(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Main: TMain;

implementation

{$R *.dfm}

function GetLocaleInformation(flag: integer): string;
var
  pcLCA: array [0..20] of Char;
begin
  if GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, flag, pcLCA, 19) <= 0 then
    pcLCA[0]:=#0;
  Result:=pcLCA;
end;

procedure TMain.FormCreate(Sender: TObject);
var
  Reg: TRegistry;
begin
  Application.Title:=Caption;
  SetWindowLong(COMPortNumberEdt.Handle, GWL_STYLE, GetWindowLong(COMPortNumberEdt.Handle, GWL_STYLE) or ES_NUMBER);
  Reg:=TRegistry.Create;
  Reg.RootKey:=HKEY_CURRENT_USER;
  if Reg.OpenKey('\Software\r57zone\X360Advance', false) then begin
    COMPortNumberEdt.Text:=IntToStr(Reg.ReadInteger('Port'));
    XAxisSensitivityTB.Position:=Reg.ReadInteger('SensX');
    YAxisSensitivityTB.Position:=Reg.ReadInteger('SensY');
    SensitivityAngleTB.Position:=Reg.ReadInteger('WheelAngle');
    CBOnlyTrigger.Checked:=Reg.ReadBool('OnlyTrigger');
    TriggerSensitivityTB.Position:=Reg.ReadInteger('TriggerSens');
  end;
  Reg.Free;
  XAxisSensitivityValueLbl.Caption:=FloatToStr(XAxisSensitivityTB.Position * 0.1);
  YAxisSensitivityValueLbl.Caption:=FloatToStr(YAxisSensitivityTB.Position * 0.1);
  SensitivityAngleValueLbl.Caption:=FloatToStr(SensitivityAngleTB.Position);
  TriggerSensitivityValueLbl.Caption:=FloatToStr(TriggerSensitivityTB.Position * 0.1);

  if GetLocaleInformation(LOCALE_SENGLANGUAGE) = 'Russian' then begin
    Main.Caption:='X360Advance - ���������';
    SetupGB.Caption:='���������';
    COMPortLbl.Caption:='����� COM-�����';
    SteeringWheelGB.Caption:='����';
    SensitivityAngleLbl.Caption:='���� ����������������';
    MouseGB.Caption:='����';
    CBOnlyTrigger.Caption:='�������. �� ������ ��������';
    XAxisSensitivityLbl.Caption:='���������������� ��� �';
    YAxisSensitivityLbl.Caption:='���������������� ��� Y';
    TriggerSensitivityLbl.Caption:='��������. � ����� ���������';
    TriggerSensitivityLbl.Hint:='��������� ����������������, ����� ����� ����� �������.';
    ApplyBtn.Caption:='���������';
    CloseBtn.Caption:='�����';
  end;
end;

procedure TMain.ApplyBtnClick(Sender: TObject);
var
  Reg: TRegistry;
begin
  Reg:=TRegistry.Create;
  Reg.RootKey:=HKEY_CURRENT_USER;
  if Reg.OpenKey('\Software\r57zone\X360Advance', true) then begin
    Reg.WriteInteger('Port', StrToInt(COMPortNumberEdt.Text));
    Reg.WriteInteger('SensX', XAxisSensitivityTB.Position);
    Reg.WriteInteger('SensY', YAxisSensitivityTB.Position);
    Reg.WriteInteger('WheelAngle', SensitivityAngleTB.Position);
    Reg.WriteBool('OnlyTrigger', CBOnlyTrigger.Checked);
    Reg.WriteInteger('TriggerSens', TriggerSensitivityTB.Position);
  end;
  Reg.Free;
  Close;
end;

procedure TMain.XAxisSensitivityTBChange(Sender: TObject);
begin
  XAxisSensitivityValueLbl.Caption:=FloatToStr(XAxisSensitivityTB.Position * 0.1);
end;

procedure TMain.YAxisSensitivityTBChange(Sender: TObject);
begin
  YAxisSensitivityValueLbl.Caption:=FloatToStr(YAxisSensitivityTB.Position * 0.1);
end;

procedure TMain.SensitivityAngleTBChange(Sender: TObject);
begin
  SensitivityAngleValueLbl.Caption:=FloatToStr(SensitivityAngleTB.Position);
end;

procedure TMain.AboutBtnClick(Sender: TObject);
begin
  Application.MessageBox(PChar('X360Advance' + #13#10 + 'https://github.com/r57zone/X360Advance' + #13#10 + 'r57zone@gmail.com'), PChar(Caption), MB_ICONINFORMATION);
end;

procedure TMain.CloseBtnClick(Sender: TObject);
begin
  Close;
end;

procedure TMain.TriggerSensitivityTBChange(Sender: TObject);
begin
  TriggerSensitivityValueLbl.Caption:=FloatToStr(TriggerSensitivityTB.Position * 0.1);
end;

end.
