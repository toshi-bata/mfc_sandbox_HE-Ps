// CreateSolidDlg.cpp
//

#include "stdafx.h"
#include "CreateSolidDlg.h"
#include "afxdialogex.h"


IMPLEMENT_DYNAMIC(CreateSolidDlg, CDialogEx)

CreateSolidDlg::CreateSolidDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CREATE_SOLID_DIALOG, pParent)
	, m_iShape(0)
	, m_dEditS1(m_dBlock_X)
	, m_dEditS2(m_dBlock_Y)
	, m_dEditS3(m_dBlock_Z)
	, m_dEditOX(0)
	, m_dEditOY(0)
	, m_dEditOZ(0)
	, m_dEditDX(0)
	, m_dEditDY(0)
	, m_dEditDZ(1)
	, m_dHookAng(180)
{
}

CreateSolidDlg::~CreateSolidDlg()
{
}

void CreateSolidDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_XL, m_dEditS1);
	DDX_Text(pDX, IDC_EDIT_YL, m_dEditS2);
	DDX_Text(pDX, IDC_EDIT_ZL, m_dEditS3);
	DDX_Text(pDX, IDC_EDIT_OX, m_dEditOX);
	DDX_Text(pDX, IDC_EDIT_OY, m_dEditOY);
	DDX_Text(pDX, IDC_EDIT_OZ, m_dEditOZ);
	DDX_Text(pDX, IDC_EDIT_DX, m_dEditDX);
	DDX_Text(pDX, IDC_EDIT_DY, m_dEditDY);
	DDX_Text(pDX, IDC_EDIT_DZ, m_dEditDZ);
	DDX_Radio(pDX, IDC_RADIO_BLOCK, m_iShape);
	DDX_Control(pDX, IDC_COMBO_SOLID_TYPE, m_comboType);
}

BOOL CreateSolidDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	UpdateData(false);

	m_comboType.AddString(_T("0"));
	m_comboType.AddString(_T("90"));
	m_comboType.AddString(_T("180"));
	m_comboType.AddString(_T("270"));
	m_comboType.SetCurSel(2);

	((CStatic*)GetDlgItem(IDC_STATIC_TYPE))->ShowWindow(false);
	((CComboBox*)GetDlgItem(IDC_COMBO_SOLID_TYPE))->ShowWindow(false);

	return 0;
}


BEGIN_MESSAGE_MAP(CreateSolidDlg, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO_BLOCK, &CreateSolidDlg::OnBnClickedRadioBlock)
	ON_BN_CLICKED(IDC_RADIO_CYLINDER, &CreateSolidDlg::OnBnClickedRadioCylinder)
	ON_BN_CLICKED(IDC_RADIO_SPRING, &CreateSolidDlg::OnBnClickedRadioSpring)
	ON_CBN_SELCHANGE(IDC_COMBO_SOLID_TYPE, &CreateSolidDlg::OnCbnSelchangeComboSolidType)
END_MESSAGE_MAP()


void CreateSolidDlg::updateUI()
{
	UpdateData(true);

	((CStatic*)GetDlgItem(IDC_STATIC_ZL))->SetWindowTextW(_T("Z"));
	((CStatic*)GetDlgItem(IDC_STATIC_ZL))->ShowWindow(true);
	((CEdit*)GetDlgItem(IDC_EDIT_ZL))->ShowWindow(true);

	((CStatic*)GetDlgItem(IDC_STATIC_TYPE))->ShowWindow(false);
	((CComboBox*)GetDlgItem(IDC_COMBO_SOLID_TYPE))->ShowWindow(false);

	switch (m_iShape)
	{
	case 0:
		SetWindowText(_T("Create Block"));
		((CStatic*)GetDlgItem(IDC_STATIC_XL))->SetWindowTextW(_T("X"));
		((CStatic*)GetDlgItem(IDC_STATIC_YL))->SetWindowTextW(_T("Y")); 

		m_dEditS1 = m_dBlock_X;
		m_dEditS2 = m_dBlock_Y;
		m_dEditS3 = m_dBlock_Z;
		break;
	case 1:
		SetWindowText(_T("Create Cylinder"));
		((CStatic*)GetDlgItem(IDC_STATIC_XL))->SetWindowTextW(_T("R"));
		((CStatic*)GetDlgItem(IDC_STATIC_YL))->SetWindowTextW(_T("H"));
		((CStatic*)GetDlgItem(IDC_STATIC_ZL))->ShowWindow(false);
		((CEdit*)GetDlgItem(IDC_EDIT_ZL))->ShowWindow(false);

		m_dEditS1 = m_dCyl_R;
		m_dEditS2 = m_dCyl_H;
		break;
	case 2:
		SetWindowText(_T("Create Spring"));
		((CStatic*)GetDlgItem(IDC_STATIC_XL))->SetWindowTextW(_T("D"));
		((CStatic*)GetDlgItem(IDC_STATIC_YL))->SetWindowTextW(_T("L"));
		((CStatic*)GetDlgItem(IDC_STATIC_ZL))->SetWindowTextW(_T("Wire D"));

		((CStatic*)GetDlgItem(IDC_STATIC_TYPE))->ShowWindow(true);
		((CComboBox*)GetDlgItem(IDC_COMBO_SOLID_TYPE))->ShowWindow(true);

		m_dEditS1 = m_dSprD;
		m_dEditS2 = m_dSprH;
		m_dEditS3 = m_dSprWireD;

		break;
	default:
		break;
	}

	UpdateData(false);
}


void CreateSolidDlg::OnBnClickedRadioBlock()
{
	updateUI();
}


void CreateSolidDlg::OnBnClickedRadioCylinder()
{
	updateUI();
}


void CreateSolidDlg::OnBnClickedRadioSpring()
{
	updateUI();
}


void CreateSolidDlg::OnCbnSelchangeComboSolidType()
{
	int iType = m_comboType.GetCurSel();
	switch (iType)
	{
	case 0: m_dHookAng = 0.0; break;
	case 1: m_dHookAng = 90.0; break;
	case 2: m_dHookAng = 180.0; break;
	case 3: m_dHookAng = 270.0; break;
	default: break;
	}
}
