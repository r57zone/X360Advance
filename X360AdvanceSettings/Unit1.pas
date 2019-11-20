unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Registry, ComCtrls, XPMan;

type
  TForm1 = class(TForm)
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
    procedure ApplyBtnClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure XAxisSensitivityTBChange(Sender: TObject);
    procedure YAxisSensitivityTBChange(Sender: TObject);
    procedure SensitivityAngleTBChange(Sender: TObject);
    procedure AboutBtnClick(Sender: TObject);
    procedure CloseBtnClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;

implementation

{$R *.dfm}

procedure TForm1.ApplyBtnClick(Sender: TObject);
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
  end;
  Reg.Free;
  Close;
end;

procedure TForm1.FormCreate(Sender: TObject);
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
  end;
  Reg.Free;
  XAxisSensitivityValueLbl.Caption:=FloatToStr(XAxisSensitivityTB.Position * 0.1);
  YAxisSensitivityValueLbl.Caption:=FloatToStr(YAxisSensitivityTB.Position * 0.1);
  SensitivityAngleValueLbl.Caption:=FloatToStr(SensitivityAngleTB.Position);
end;

procedure TForm1.XAxisSensitivityTBChange(Sender: TObject);
begin
  XAxisSensitivityValueLbl.Caption:=FloatToStr(XAxisSensitivityTB.Position * 0.1);
end;

procedure TForm1.YAxisSensitivityTBChange(Sender: TObject);
begin
  YAxisSensitivityValueLbl.Caption:=FloatToStr(YAxisSensitivityTB.Position * 0.1);
end;

procedure TForm1.SensitivityAngleTBChange(Sender: TObject);
begin
  SensitivityAngleValueLbl.Caption:=FloatToStr(SensitivityAngleTB.Position);
end;

procedure TForm1.AboutBtnClick(Sender: TObject);
begin
  Application.MessageBox(PChar('X360Advance' + #13#10 + 'https://github.com/r57zone/X360Advance' + #13#10 + 'r57zone@gmail.com'), PChar(Caption), MB_ICONINFORMATION);
end;

procedure TForm1.CloseBtnClick(Sender: TObject);
begin
  Close;
end;

end.
