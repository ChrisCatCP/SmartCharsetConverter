#pragma once

#include "Core.h"

#include "resource.h"

#include <tstring.h>
#include <TListView.h>

#include <atlbase.h>        // ������ATL��
#include <atlwin.h>         // ATL������
#include <atlapp.h>     // WTL ����ܴ�����
#include <atlctrls.h>  
#include <atlcrack.h>   // WTL ��ǿ����Ϣ��

#include <vector>
#include <chrono>
#include <string>
#include <unordered_set>

class DialogMain :public CDialogImpl<DialogMain>
{
private:
	const std::string caption;

	Core core;

	TListView listview;

	enum class ListViewColumn
	{
		INDEX = 0,
		FILENAME,
		FILESIZE,
		ENCODING,
		TEXT_PIECE
	};

	std::unordered_set<std::tstring> listFileNames;
public:

	enum { IDD = IDD_DIALOG_MAIN };

	DialogMain();

	BOOL OnInitDialog(CWindow wndFocus, LPARAM lInitParam);

	void SetFilterMode(Configuration::FilterMode mode);

	void SetOutputTarget(Configuration::OutputTarget outputTarget);

	void SetOutputCharset(Configuration::OutputCharset charset);

	void AddItem(const std::tstring &filename);

	void AddItems(const std::vector<std::tstring> &filenames);

	void OnClose();

	BEGIN_MSG_MAP(DialogMain)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)

		COMMAND_HANDLER(IDC_RADIO_STRETEGY_SMART, BN_CLICKED, OnBnClickedRadioStretegySmart)
		COMMAND_HANDLER(IDC_RADIO_STRETEGY_MANUAL, BN_CLICKED, OnBnClickedRadioStretegyManual)
		COMMAND_HANDLER(IDC_CHECK_INCLUDE_TEXT, BN_CLICKED, OnBnClickedCheckIncludeText)
		COMMAND_HANDLER(IDC_CHECK_EXCLUDE_TEXT, BN_CLICKED, OnBnClickedCheckExcludeText)
		COMMAND_HANDLER(IDC_RADIO_TO_ORIGIN, BN_CLICKED, OnBnClickedRadioToOrigin)
		COMMAND_HANDLER(IDC_RADIO_TO_DIR, BN_CLICKED, OnBnClickedRadioToDir)
		COMMAND_HANDLER(IDC_RADIO_UTF8, BN_CLICKED, OnBnClickedRadioUtf8)
		COMMAND_HANDLER(IDC_RADIO_UTF8BOM, BN_CLICKED, OnBnClickedRadioUtf8bom)
		COMMAND_HANDLER(IDC_RADIO_GB18030, BN_CLICKED, OnBnClickedRadioGb18030)
		COMMAND_HANDLER(IDC_RADIO_OTHER, BN_CLICKED, OnBnClickedRadioOther)
		COMMAND_HANDLER(IDC_BUTTON_ADD_FILES, BN_CLICKED, OnBnClickedButtonAddFiles)
		COMMAND_HANDLER(IDC_BUTTON_ADD_DIR, BN_CLICKED, OnBnClickedButtonAddDir)
		COMMAND_HANDLER(IDC_BUTTON_START, BN_CLICKED, OnBnClickedButtonStart)
		COMMAND_HANDLER(IDC_BUTTON_CLEAR, BN_CLICKED, OnBnClickedButtonClear)
		COMMAND_HANDLER(IDC_BUTTON_SET_OUTPUT_DIR, BN_CLICKED, OnBnClickedButtonSetOutputDir)
		COMMAND_HANDLER(IDC_COMBO_OTHER_CHARSET, CBN_SELCHANGE, OnCbnSelchangeComboOtherCharset)
		NOTIFY_HANDLER(IDC_LISTVIEW, NM_RCLICK, OnNMRclickListview)
		COMMAND_ID_HANDLER(ID_OPEN_WITH_NOTEPAD, OnOpenWithNotepad)
		COMMAND_ID_HANDLER(ID_REMOVE_ITEM, OnRemoveItem)
	END_MSG_MAP()
	LRESULT OnBnClickedRadioStretegySmart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedRadioStretegyManual(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedCheckIncludeText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedCheckExcludeText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedRadioToOrigin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedRadioToDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedRadioUtf8(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedRadioUtf8bom(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedRadioGb18030(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedRadioOther(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedButtonAddFiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedButtonAddDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedButtonStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedButtonClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnBnClickedButtonSetOutputDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnCbnSelchangeComboOtherCharset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnNMRclickListview(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL & /*bHandled*/);
	LRESULT OnOpenWithNotepad(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
	LRESULT OnRemoveItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
};