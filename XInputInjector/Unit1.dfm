object Main: TMain
  Left = 192
  Top = 125
  Width = 357
  Height = 266
  AlphaBlend = True
  AlphaBlendValue = 0
  Caption = 'XInput injector'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  PixelsPerInch = 96
  TextHeight = 13
  object ListView: TListView
    Left = 6
    Top = 8
    Width = 331
    Height = 184
    Columns = <
      item
        Width = 40
      end
      item
        AutoSize = True
        Caption = 'Process'
      end
      item
        AutoSize = True
        Caption = 'Title'
      end
      item
        Caption = 'Location'
        Width = 0
      end>
    ReadOnly = True
    RowSelect = True
    SmallImages = ImageList
    TabOrder = 0
    ViewStyle = vsReport
    OnDblClick = ListViewDblClick
    OnMouseDown = ListViewMouseDown
  end
  object RefreshBtn: TButton
    Left = 5
    Top = 196
    Width = 75
    Height = 25
    Caption = 'Refresh'
    TabOrder = 1
    OnClick = RefreshBtnClick
  end
  object SelectLibCB: TComboBox
    Left = 87
    Top = 198
    Width = 169
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    TabOrder = 2
    OnChange = SelectLibCBChange
  end
  object SettingsBtn: TButton
    Left = 263
    Top = 196
    Width = 75
    Height = 25
    Caption = 'Settings'
    TabOrder = 3
    OnClick = SettingsBtnClick
  end
  object ImageList: TImageList
    Height = 32
    Width = 32
    Left = 48
    Top = 32
  end
  object XPManifest1: TXPManifest
    Left = 16
    Top = 32
  end
  object PopupMenu: TPopupMenu
    Left = 80
    Top = 32
    object ShowHideBtn: TMenuItem
      Caption = 'Show / Hide'
      OnClick = ShowHideBtnClick
    end
    object N2: TMenuItem
      Caption = '-'
    end
    object AboutBtn: TMenuItem
      Caption = 'About...'
      OnClick = AboutBtnClick
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object CloseBtn: TMenuItem
      Caption = 'Close'
      OnClick = CloseBtnClick
    end
  end
end
