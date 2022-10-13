object Main: TMain
  Left = 192
  Top = 125
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsSingle
  Caption = 'X360Advance - Settings'
  ClientHeight = 273
  ClientWidth = 384
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object ApplyBtn: TButton
    Left = 8
    Top = 240
    Width = 75
    Height = 25
    Caption = 'Apply'
    TabOrder = 0
    OnClick = ApplyBtnClick
  end
  object CloseBtn: TButton
    Left = 88
    Top = 240
    Width = 75
    Height = 25
    Caption = 'Close'
    TabOrder = 5
    OnClick = CloseBtnClick
  end
  object MouseGB: TGroupBox
    Left = 192
    Top = 8
    Width = 185
    Height = 225
    Caption = 'Mouse'
    TabOrder = 4
    object XAxisSensitivityLbl: TLabel
      Left = 8
      Top = 48
      Width = 76
      Height = 13
      Caption = 'X-axis sensitivity'
    end
    object YAxisSensitivityLbl: TLabel
      Left = 8
      Top = 96
      Width = 76
      Height = 13
      Caption = 'Y-axis sensitivity'
    end
    object XAxisSensitivityValueLbl: TLabel
      Left = 160
      Top = 68
      Width = 6
      Height = 13
      Caption = '0'
    end
    object YAxisSensitivityValueLbl: TLabel
      Left = 160
      Top = 116
      Width = 6
      Height = 13
      Caption = '0'
    end
    object TriggerSensitivityLbl: TLabel
      Left = 8
      Top = 144
      Width = 118
      Height = 13
      Caption = 'Sensitivity with left trigger'
    end
    object TriggerSensitivityValueLbl: TLabel
      Left = 160
      Top = 165
      Width = 6
      Height = 13
      Caption = '0'
    end
    object XAxisSensitivityTB: TTrackBar
      Left = 8
      Top = 64
      Width = 150
      Height = 33
      Max = 100
      Min = 1
      Frequency = 10
      Position = 30
      TabOrder = 1
      OnChange = XAxisSensitivityTBChange
    end
    object YAxisSensitivityTB: TTrackBar
      Left = 8
      Top = 112
      Width = 150
      Height = 33
      Max = 100
      Min = 1
      Frequency = 10
      Position = 25
      TabOrder = 2
      OnChange = YAxisSensitivityTBChange
    end
    object CBOnlyTrigger: TCheckBox
      Left = 8
      Top = 21
      Width = 169
      Height = 17
      Caption = 'Activate by left trigger'
      TabOrder = 0
    end
    object TriggerSensitivityTB: TTrackBar
      Left = 8
      Top = 160
      Width = 150
      Height = 33
      Hint = 'Decrease the sensitivity to aim more accurately.'
      Max = 20
      Min = 1
      ParentShowHint = False
      Position = 10
      ShowHint = True
      TabOrder = 3
      OnChange = TriggerSensitivityTBChange
    end
    object JoyMouseCB: TCheckBox
      Left = 8
      Top = 200
      Width = 169
      Height = 17
      Caption = 'Joystick-mouse aiming'
      TabOrder = 4
      Visible = False
    end
  end
  object SteeringWheelGB: TGroupBox
    Left = 8
    Top = 80
    Width = 177
    Height = 81
    Caption = 'Steering wheel'
    TabOrder = 2
    object SensitivityAngleLbl: TLabel
      Left = 8
      Top = 20
      Width = 76
      Height = 13
      Caption = 'Sensitivity angle'
    end
    object SensitivityAngleValueLbl: TLabel
      Left = 150
      Top = 44
      Width = 6
      Height = 13
      Caption = '0'
    end
    object SensitivityAngleTB: TTrackBar
      Left = 2
      Top = 40
      Width = 148
      Height = 33
      Max = 360
      Min = 60
      Frequency = 10
      Position = 75
      TabOrder = 0
      OnChange = SensitivityAngleTBChange
    end
  end
  object SetupGB: TGroupBox
    Left = 8
    Top = 8
    Width = 177
    Height = 65
    Caption = 'Setup'
    TabOrder = 1
    object COMPortLbl: TLabel
      Left = 8
      Top = 17
      Width = 86
      Height = 13
      Caption = 'COM port number:'
    end
    object COMPortNumberEdt: TEdit
      Left = 8
      Top = 35
      Width = 49
      Height = 21
      TabOrder = 0
      Text = '2'
    end
  end
  object AboutBtn: TButton
    Left = 349
    Top = 240
    Width = 27
    Height = 25
    Caption = '?'
    TabOrder = 6
    OnClick = AboutBtnClick
  end
  object ExternalPedalsGB: TGroupBox
    Left = 8
    Top = 168
    Width = 177
    Height = 65
    Caption = 'External pedals'
    TabOrder = 3
    object ExternalPedalsCOMPortLbl: TLabel
      Left = 8
      Top = 18
      Width = 86
      Height = 13
      Caption = 'COM port number:'
    end
    object ExternalPedalsCOMPortNumberEdt: TEdit
      Left = 8
      Top = 36
      Width = 57
      Height = 21
      TabOrder = 0
      Text = '3'
    end
  end
  object XPManifest1: TXPManifest
    Left = 152
    Top = 24
  end
end
