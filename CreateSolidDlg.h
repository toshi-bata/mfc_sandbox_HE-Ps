#pragma once
#include "Resource.h"

class CreateSolidDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CreateSolidDlg)

public:
	CreateSolidDlg(CWnd* pParent = nullptr);
	virtual ~CreateSolidDlg();
	virtual BOOL OnInitDialog();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CREATE_SOLID_DIALOG };
#endif

private:
	double m_dBlock_X = 100;
	double m_dBlock_Y = 100;
	double m_dBlock_Z = 100;

	double m_dCyl_R = 50;
	double m_dCyl_H = 100;

	double m_dSprD = 10;
	double m_dSprH = 50;
	double m_dSprWireD = 0.9;

	void updateUI();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

public:
	double m_dEditS1;
	double m_dEditS2;
	double m_dEditS3;
	double m_dEditOX;
	double m_dHookAng;
	double m_dEditOY;
	double m_dEditOZ;
	double m_dEditDX;
	double m_dEditDY;
	double m_dEditDZ;
	int m_iShape;
	afx_msg void OnBnClickedRadioBlock();
	afx_msg void OnBnClickedRadioCylinder();
	afx_msg void OnBnClickedRadioSpring();
	afx_msg void OnCbnSelchangeComboSolidType();
	CComboBox m_comboType;
};
