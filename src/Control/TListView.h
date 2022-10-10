#pragma once

#include <tstring.h>

#include <atlbase.h>        // ������ATL��
#include <atlwin.h>         // ATL������
#include <atlapp.h>     // WTL ����ܴ�����
#include <atlctrls.h>

#include <vector>

class TListView : public CListViewCtrl
{
public:

	std::vector<int> GetSelectedItems() const;

	std::tstring GetItemText(int nItem, int nSubItem) const;
};