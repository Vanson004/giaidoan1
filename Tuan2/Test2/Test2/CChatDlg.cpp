// CChatDlg.cpp : implementation file
//

#include "pch.h"
#include "Test2.h"
#include "CChatDlg.h"
#include "afxdialogex.h"
#include "odbcinst.h"
#include "afxdb.h"


// CChatDlg dialog

IMPLEMENT_DYNAMIC(CChatDlg, CDialogEx)

CChatDlg::CChatDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
	, m_editMessage(_T(""))
{

}

CChatDlg::~CChatDlg()
{
}

void CChatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_MESSAGE, m_editMessage);
	DDX_Control(pDX, IDC_LIST_CHAT, m_listChat);
	DDX_Control(pDX, IDC_LIST_Friends, m_listFriends);
}

BOOL CChatDlg::OnInitDialog() {
	CDialogEx::OnInitDialog();

	m_listFriends.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_listChat.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	m_imgListFriends.Create(32, 32, ILC_COLOR32 | ILC_MASK, 0, 1);
	HICON hIconDua = AfxGetApp()->LoadIconW(IDI_ICON_Dua);
	HICON hIconBupBe = AfxGetApp()->LoadIconW(IDI_ICON_BupBe);
	HICON hIconCanhCut = AfxGetApp()->LoadIconW(IDI_ICON_CanhCut);
	
	int nIdxDua = m_imgListFriends.Add(hIconDua);
	int nIdxBupBe = m_imgListFriends.Add(hIconBupBe);
	int nIdxCanhCut = m_imgListFriends.Add(hIconCanhCut);

	m_listFriends.SetImageList(&m_imgListFriends, LVSIL_SMALL);

	m_listFriends.InsertColumn(0, _T("Danh sách bạn bè"), LVCFMT_LEFT,200);
	m_listFriends.InsertItem(0, _T("Nguyen Van A"), nIdxDua);
	m_listFriends.InsertItem(1, _T("Tran Thi B"), nIdxBupBe);
	m_listFriends.InsertItem(2, _T("Hoang Van C"), nIdxCanhCut);

	m_listChat.InsertColumn(0, _T("Thời gian"), LVCFMT_LEFT, 100);
	m_listChat.InsertColumn(1, _T("Bạn bè"), LVCFMT_LEFT, 200);
	m_listChat.InsertColumn(2, _T("Tôi"), LVCFMT_LEFT, 200);
	return TRUE;
}

BEGIN_MESSAGE_MAP(CChatDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CChatDlg::OnBnClickedButtonSend)
	ON_NOTIFY(NM_CLICK, IDC_LIST_Friends, &CChatDlg::OnNMClickListFriends)
END_MESSAGE_MAP()


// CChatDlg message handlers


void CChatDlg::OnBnClickedButtonSend()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CString name = _T("Nguyen Van Son");
	CString content = m_editMessage;
	CTime curTime = CTime::GetCurrentTime();
	int hour = curTime.GetHour();
	int minute = curTime.GetMinute();
	CString time = curTime.Format(_T("%H:%M"));

	int count = m_listChat.GetItemCount();
	int n_Index2 = m_listChat.InsertItem(count, name);
	m_listChat.SetItemText(n_Index2, 1, content);
	m_listChat.SetItemText(n_Index2, 2, time);
	UpdateData(FALSE);
}


void CChatDlg::OnNMClickListFriends(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);	
	int index = pNMItemActivate->iItem;
	if (index >= 0) {
		CString strFriendName = m_listFriends.GetItemText(index, 0);
		LoadChatHistory(strFriendName);
	}
	*pResult = 0;
}

void CChatDlg::LoadChatHistory(CString strFriendName) {
	m_listChat.DeleteAllItems();

	if (strFriendName == _T("Nguyen Van A")) {
		int iRow1 = m_listChat.InsertItem(0, _T("9:45"));
		m_listChat.SetItemText(iRow1, 1, _T("Hôm nay khỏe không"));
		m_listChat.SetItemText(iRow1, 2, _T(""));
		
		int iRow2 = m_listChat.InsertItem(1, _T("9:46"));
		m_listChat.SetItemText(iRow2, 1, _T(""));
		m_listChat.SetItemText(iRow2, 2, _T("Khỏe"));

		int iRow3 = m_listChat.InsertItem(3, _T("9:49"));
		m_listChat.SetItemText(iRow3, 1, _T("Tý đi ăn không"));
		m_listChat.SetItemText(iRow3, 2, _T(""));

		int iRow4 = m_listChat.InsertItem(4, _T("9:51"));
		m_listChat.SetItemText(iRow4, 1, _T(""));
		m_listChat.SetItemText(iRow4, 2, _T("Ok"));
	}
	else if(strFriendName == "Tran Thi B") {
		int iRow1 = m_listChat.InsertItem(0, _T("8:10"));
		m_listChat.SetItemText(iRow1, 1, _T("Xong bài tập chưa"));
		m_listChat.SetItemText(iRow1, 2, _T(""));

		int iRow2 = m_listChat.InsertItem(1, _T("8:15"));
		m_listChat.SetItemText(iRow2, 1, _T(""));
		m_listChat.SetItemText(iRow2, 2, _T("Sắp xong"));

		int iRow3 = m_listChat.InsertItem(3, _T("8:17"));
		m_listChat.SetItemText(iRow3, 1, _T("Cho tôi mượn nhé"));
		m_listChat.SetItemText(iRow3, 2, _T(""));

		int iRow4 = m_listChat.InsertItem(4, _T("8:20"));
		m_listChat.SetItemText(iRow4, 1, _T(""));
		m_listChat.SetItemText(iRow4, 2, _T("Ok"));
	}
	else if (strFriendName == "Hoang Van C") {
		int iRow1 = m_listChat.InsertItem(0, _T("9:00"));
		m_listChat.SetItemText(iRow1, 1, _T("Em lên công ty chưa"));
		m_listChat.SetItemText(iRow1, 2, _T(""));

		int iRow2 = m_listChat.InsertItem(1, _T("9:01"));
		m_listChat.SetItemText(iRow2, 1, _T(""));
		m_listChat.SetItemText(iRow2, 2, _T("Rồi ạ"));

		int iRow3 = m_listChat.InsertItem(3, _T("8:17"));
		m_listChat.SetItemText(iRow3, 1, _T("Ok"));
		m_listChat.SetItemText(iRow3, 2, _T(""));
	}
}
