#pragma once

#include <tstring.h>

#include <atlbase.h>        // ������ATL��
#include <atlwin.h>         // ATL������
#include <atlapp.h>     // WTL ����ܴ�����
#include <atlctrls.h>
#include <atlcrack.h>   // WTL ��ǿ����Ϣ��

#include <vector>

class TListView : public CWindowImpl<TListView, CListViewCtrl>
{
public:
	BEGIN_MSG_MAP_EX(TListView)
		MESSAGE_HANDLER(WM_DROPFILES, OnDropFiles)
	END_MSG_MAP()

	std::vector<int> GetSelectedItems() const;

	std::tstring GetItemText(int nItem, int nSubItem) const;

	LRESULT OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
	{
		SendMessage(GetParent(), uMsg, wParam, lParam);
		return 0;
	}
};