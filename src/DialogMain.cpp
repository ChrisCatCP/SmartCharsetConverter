#include "DialogMain.h"

#include <tstring.h>
#include <FileFunction.h>
#include "Control/TMenu.h"

#ifdef _DEBUG
#include <iostream>
#endif

#include <stdexcept>
#include <sstream>
#include <set>
#include <regex>

const std::tstring appTitle = TEXT("���ܱ��뼯ת���� v0.1 by Tom Willow");

using namespace std;

DialogMain::DialogMain() :core(TEXT("SmartCharsetConverter.ini"))
{
}


void DialogMain::OnClose()
{
	EndDialog(0);
}

BOOL DialogMain::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	// ���ô��ڵĴ�Сͼ��
	// ��ͼ�꣺����alt+tab���л�����ʱ��Ӧ��ͼ��
	// Сͼ�꣺���Ǵ������ϽǶ�Ӧ���Ǹ�ͼ��
	HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	::SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	::SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

	SetWindowText(appTitle.c_str());

	// ����/�ų�ָ����׺
	SetFilterMode(core.GetConfig().filterMode);
	//GetDlgItem(IDC_EDIT_INCLUDE_TEXT).SetWindowTextW(core.GetConfig().includeRule);

	// target
	SetOutputTarget(core.GetConfig().outputTarget);
	GetDlgItem(IDC_EDIT_OUTPUT_DIR).SetWindowTextW(core.GetConfig().outputDir.c_str());
	static_cast<CEdit>(GetDlgItem(IDC_EDIT_OUTPUT_DIR)).SetReadOnly(true);

	SetOutputCharset(core.GetConfig().outputCharset);

	// listview
	listview.Attach(GetDlgItem(IDC_LISTVIEW));
	listview.ModifyStyle(0, LVS_REPORT);
	listview.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	listview.AddColumn(TEXT("���"), static_cast<int>(ListViewColumn::INDEX));
	listview.SetColumnWidth(0, 40);

	listview.AddColumn(TEXT("�ļ���"), static_cast<int>(ListViewColumn::FILENAME));
	listview.SetColumnWidth(1, 300);

	listview.AddColumn(TEXT("��С"), static_cast<int>(ListViewColumn::FILESIZE));
	listview.SetColumnWidth(2, 60);

	listview.AddColumn(TEXT("����"), static_cast<int>(ListViewColumn::ENCODING));
	listview.SetColumnWidth(3, 60);

	listview.AddColumn(TEXT("�ı�Ƭ��"), static_cast<int>(ListViewColumn::TEXT_PIECE));
	listview.SetColumnWidth(4, 200);

	setlocale(LC_CTYPE, "");

	CenterWindow();

	return 0;
}

void DialogMain::SetFilterMode(Configuration::FilterMode mode)
{
	core.SetFilterMode(mode);
	bool isSmart = (mode == Configuration::FilterMode::SMART);
	CButton(GetDlgItem(IDC_RADIO_STRETEGY_SMART)).SetCheck(isSmart);
	CButton(GetDlgItem(IDC_RADIO_STRETEGY_MANUAL)).SetCheck(!isSmart);

	GetDlgItem(IDC_EDIT_INCLUDE_TEXT).EnableWindow(!isSmart);
}

void DialogMain::SetOutputTarget(Configuration::OutputTarget outputTarget)
{
	core.SetOutputTarget(outputTarget);
	bool isToOrigin = (outputTarget == Configuration::OutputTarget::ORIGIN);

	CButton(GetDlgItem(IDC_RADIO_TO_ORIGIN)).SetCheck(isToOrigin);
	CButton(GetDlgItem(IDC_RADIO_TO_DIR)).SetCheck(!isToOrigin);

	GetDlgItem(IDC_EDIT_OUTPUT_DIR).EnableWindow(!isToOrigin);
	GetDlgItem(IDC_BUTTON_SET_OUTPUT_DIR).EnableWindow(!isToOrigin);
}

void DialogMain::SetOutputCharset(CharsetCode charset)
{
	core.SetOutputCharset(charset);
	bool isNormalCharset = Configuration::IsNormalCharset(charset);

	CButton(GetDlgItem(IDC_RADIO_UTF8)).SetCheck(charset == CharsetCode::UTF8);
	CButton(GetDlgItem(IDC_RADIO_UTF8BOM)).SetCheck(charset == CharsetCode::UTF8BOM);
	CButton(GetDlgItem(IDC_RADIO_GB18030)).SetCheck(charset == CharsetCode::GB18030);
	CButton(GetDlgItem(IDC_RADIO_OTHER)).SetCheck(Configuration::IsNormalCharset(charset) == false);

	GetDlgItem(IDC_COMBO_OTHER_CHARSET).EnableWindow(!isNormalCharset);

}

bool DialogMain::AddItem(const std::tstring &filename)
{
	// ʶ���ַ���
	auto [charsetName, content, contentSize] = core.GetEncodingStr(filename);

	try
	{
		auto count = listview.GetItemCount();
		listview.AddItem(count, static_cast<int>(ListViewColumn::INDEX), to_tstring(count + 1).c_str());
		listview.AddItem(count, static_cast<int>(ListViewColumn::FILENAME), filename.c_str());
		listview.AddItem(count, static_cast<int>(ListViewColumn::FILESIZE), FileSizeToTString(GetFileSize(filename)).c_str());

		listview.AddItem(count, static_cast<int>(ListViewColumn::ENCODING), charsetName.c_str());

		listview.AddItem(count, static_cast<int>(ListViewColumn::TEXT_PIECE), reinterpret_cast<wchar_t *>(content.get()));

	}
	catch (runtime_error &err)
	{
		// ���AddItem֮������Ƴ���������Ŀ
		listview.DeleteItem(listview.GetItemCount() - 1);
		throw err;
	}

	return content != nullptr;
}

void DialogMain::AddItems(const std::vector<std::tstring> &filenames)
{
	vector<pair<tstring, tstring>> failed;
	for (auto &filename : filenames)
	{
		try
		{
			// ����ظ���
			if (listFileNames.find(filename) != listFileNames.end())
			{
				failed.push_back({ filename,TEXT("�ظ����") });
				continue;	// ���ظ������
			}
			AddItem(filename);

			listFileNames.insert(filename);
		}
		catch (runtime_error &e)
		{
			failed.push_back({ filename,to_tstring(e.what()) });
		}
	}

	if (failed.empty() == false)
	{
		tstring info = TEXT("�����ļ����ʧ�ܣ�\r\n");
		for (auto &pr : failed)
		{
			info += pr.first + TEXT(" ԭ��") + pr.second + TEXT("\r\n");
		}
		MessageBox(info.c_str(), TEXT("Error"), MB_OK | MB_ICONERROR);
	}
}

LRESULT DialogMain::OnBnClickedRadioStretegySmart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetFilterMode(Configuration::FilterMode::SMART);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioStretegyManual(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetFilterMode(Configuration::FilterMode::ONLY_SOME_EXTANT);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioToOrigin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetOutputTarget(Configuration::OutputTarget::ORIGIN);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioToDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetOutputTarget(Configuration::OutputTarget::TO_DIR);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioUtf8(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetOutputCharset(CharsetCode::UTF8);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioUtf8bom(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetOutputCharset(CharsetCode::UTF8BOM);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioGb18030(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	SetOutputCharset(CharsetCode::GB18030);
	return 0;
}


LRESULT DialogMain::OnBnClickedRadioOther(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	//SetOutputCharset(Configuration::OutputCharset::OTHER_UNSPECIFIED);
	return 0;
}

void DialogMain::CheckAndTraversalIncludeRule(std::function<void(const std::tstring &dotExt)> fn)
{
	// ��׺�ַ���
	auto &extsStr = core.GetConfig().includeRule;

	// �з�
	auto exts = Split(extsStr, TEXT(" "));

	// ���Ϊ��
	if (exts.empty())
	{
		throw runtime_error("ָ���ĺ�׺��Ч��\r\n\r\n���ӣ�*.h *.hpp *.c *.cpp *.txt");
	}

	// ������
	for (auto ext : exts)
	{
		tstring extStr(ext);
		wstring pattern = TEXT(R"(\*(\.\w+))");	// ƥ�� *.xxx ������
		wregex r(pattern);
		wsmatch results;
		if (regex_match(extStr, results, r) == false)
		{
			throw runtime_error("ָ���ĺ�׺��Ч��" + to_string(extStr) + "��\r\n\r\n���ӣ� * .h * .hpp * .c * .cpp * .txt");
		}

		fn(results.str(1));
	}

}

LRESULT DialogMain::OnBnClickedButtonAddFiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)try
{
	vector<pair<tstring, tstring>> dialogFilter;
	if (core.GetConfig().filterMode == Configuration::FilterMode::SMART)
	{
		// ����ʶ�������ı�

		dialogFilter = { { L"�����ļ�*.*", L"*.*" } };
	}
	else
	{
		// ֻ����ָ����׺

		tstring filterExtsStr;	// dialog�Ĺ�����Ҫ��;�ָ�
		CheckAndTraversalIncludeRule([&](const tstring &dotExt)
			{
				filterExtsStr += TEXT("*") + dotExt + TEXT(";");
			});

		// dialog������
		dialogFilter.push_back(make_pair(filterExtsStr, filterExtsStr));
	}

	// ���ļ��Ի���
	TFileDialog dialog(*this, dialogFilter, true);
	if (dialog.Open())
	{
		auto ans = dialog.GetResult();

		AddItems(ans);
	}
	return 0;
}
catch (runtime_error &err)
{
	MessageBox(to_tstring(err.what()).c_str(), TEXT("����"), MB_OK | MB_ICONERROR);
	return 0;
}


LRESULT DialogMain::OnBnClickedButtonAddDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)try
{
	// �洢�����ļ�ʱҪ�����ĺ�׺
	vector<tstring> filterDotExts;

	if (core.GetConfig().filterMode == Configuration::FilterMode::SMART)
	{
		// ����ʶ�������ı�
	}
	else
	{
		// ֻ����ָ����׺

		CheckAndTraversalIncludeRule([&](const tstring &dotExt)
			{
				filterDotExts.push_back(dotExt);
			});
	}

	tstring dir;

	TFolderBrowser folderBrowser(*this);
	if (folderBrowser.Open(dir))
	{
		// ����ָ��Ŀ¼
		auto filenames = TraversalAllFileNames(dir, filterDotExts);

		if (filenames.empty())
		{
			MessageBox((TEXT("ָ����Ŀ¼û�з��ϵ��ļ���") + dir).c_str(), TEXT("��ʾ"), MB_OK | MB_ICONERROR);
			return 0;
		}

		AddItems(filenames);
	}

	return 0;
}
catch (runtime_error &err)
{
	MessageBox(to_tstring(err.what()).c_str(), TEXT("����"), MB_OK | MB_ICONERROR);
	return 0;
}


LRESULT DialogMain::OnBnClickedButtonStart(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL &bHandle /*bHandled*/)try
{
	// ���û������
	if (listview.GetItemCount() == 0)
	{
		throw runtime_error("û�д�ת�����ļ���");
	}

	// ������Ŀ¼
	if (core.GetConfig().outputTarget != Configuration::OutputTarget::ORIGIN)
	{
		if (core.GetConfig().outputDir.empty())
		{
			throw runtime_error("���Ŀ¼��Ч��");
		}
	}

	vector<tstring> allOutputFileNames;	// ȫ���ļ����ɹ�ʧ�ܾ��У�
	vector<pair<tstring, tstring>> failed;	// ʧ���ļ�/ʧ��ԭ��
	vector<tstring> succeed;	// �ɹ����ļ�

	// Ŀ�����
	auto targetCode = core.GetConfig().outputCharset;

	// ���ת��
	for (int i = 0; i < listview.GetItemCount(); ++i)
	{
		auto filename = listview.GetItemText(i, static_cast<int>(ListViewColumn::FILENAME));
		try
		{
			// ����Ŀ���ļ���
			auto outputFileName = filename;
			if (core.GetConfig().outputTarget != Configuration::OutputTarget::ORIGIN)
			{
				// ���ļ���
				auto pureFileName = GetNameAndExt(outputFileName);

				outputFileName = core.GetConfig().outputDir + TEXT("\\") + pureFileName;
			}

			// ���뵽�����б�
			allOutputFileNames.push_back(outputFileName);

			// ȡ��ԭ���뼯
			auto originCode = ToCharsetCode(listview.GetItemText(i, static_cast<int>(ListViewColumn::ENCODING)));
			if (originCode == CharsetCode::UNKNOWN)
			{
				throw runtime_error("δ̽������뼯");
			}

			// ���벻һ����ת���������ƹ�ȥ
			if (originCode != targetCode)
			{
				auto filesize = GetFileSize(filename);

				// ��ʱ�����ֿ�ת�� TODO

				{
					auto [raw, rawSize] = ReadFileToBuffer(filename);

					// ����BOMƫ��
					const char *rawStart = raw.get();

					// �����ҪĨ��BOM�������ʼλ�����õ�BOM֮��ȷ��UChar[]����BOM
					if (HasBom(originCode) && !HasBom(targetCode))
					{
						auto bomSize = BomSize(originCode);
						rawStart += bomSize;
						rawSize -= bomSize;
					}

					// ����ԭ����õ�Unicode�ַ���
					auto [buf, bufSize] = Decode(rawStart, rawSize, originCode);

					// ת��Ŀ�����
					auto [ret, retLen] = Encode(buf, bufSize, targetCode);

					// д���ļ�

					FILE *fp = _tfopen(outputFileName.c_str(), TEXT("wb"));
					unique_ptr<FILE, function<void(FILE *)>> upFile(fp, [](FILE *fp) { fclose(fp); });

					// �����Ҫ�������BOM����д��BOM
					if (!HasBom(originCode) && HasBom(targetCode))
					{
						auto bomData = GetBomData(targetCode);

						// д��BOM
						size_t wrote = fwrite(bomData, BomSize(targetCode), 1, fp);
						if (wrote != 1)
						{
							throw runtime_error("д��ʧ�ܣ�" + to_string(outputFileName));
						}
					}

					// д������
					size_t wrote = fwrite(ret.get(), retLen, 1, fp);
					if (wrote != 1)
					{
						throw runtime_error("д��ʧ�ܣ�" + to_string(outputFileName));
					}
				}
			}
			else
			{
				// ����һ��

				// �������ԭλ��ת�������ƹ�ȥ
				if (core.GetConfig().outputTarget == Configuration::OutputTarget::TO_DIR)
				{
					bool ok = CopyFile(filename.c_str(), outputFileName.c_str(), false);
					if (!ok)
					{
						throw runtime_error("д��ʧ�ܣ�" + to_string(outputFileName));
					}
				}
			}

			// ����ļ��ɹ���
			succeed.push_back(filename);
		}
		catch (runtime_error &e)
		{
			// ����ļ�ʧ����
			failed.push_back({ filename,to_tstring(e.what()) });
		}
	}

	// �Ѿ���ɴ���

	// �����ʧ�ܵ�
	if (failed.empty() == false)
	{
		tstringstream ss;
		ss << TEXT("ת���ɹ� ") << succeed.size() << TEXT(" ���ļ���\r\n\r\n");
		ss << TEXT("�����ļ�ת��ʧ�ܣ�\r\n");
		for (auto &pr : failed)
		{
			ss << pr.first << TEXT(" ԭ��") << pr.second << TEXT("\r\n");
		}
		MessageBox(ss.str().c_str(), TEXT("ת�����"), MB_OK | MB_ICONERROR);
	}
	else
	{
		// ȫ���ɹ�֮��
		tstringstream ss;
		ss << TEXT("ת����ɣ�");

		if (targetCode == CharsetCode::GB18030)
		{
			ss << TEXT("\r\n\r\nע�⣺GB18030�ڴ�Ӣ�ĵ�����º�UTF-8����λ�غϣ����Կ��ܻ����ת������ʾΪUTF-8����������");
		}
		MessageBox(ss.str().c_str(), TEXT("��ʾ"), MB_OK | MB_ICONINFORMATION);
	}

	// ����б�
	OnBnClickedButtonClear(0, 0, 0, bHandle);

	// ��ת���Ľ���ٴμ��ص��б���
	AddItems(allOutputFileNames);

	return 0;
}
catch (runtime_error &err)
{
	MessageBox(to_tstring(err.what()).c_str(), TEXT("����"), MB_OK | MB_ICONERROR);
	return 0;
}


LRESULT DialogMain::OnBnClickedButtonClear(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	listview.DeleteAllItems();
	listFileNames.clear();
	return 0;
}


LRESULT DialogMain::OnBnClickedButtonSetOutputDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	tstring dir = core.GetConfig().outputDir;

	TFolderBrowser folderBrowser(*this);
	if (folderBrowser.Open(dir))
	{
		core.SetOutputDir(dir);
		GetDlgItem(IDC_EDIT_OUTPUT_DIR).SetWindowTextW(dir.c_str());
	}

	return 0;
}


LRESULT DialogMain::OnCbnSelchangeComboOtherCharset(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	return 0;
}


LRESULT DialogMain::OnNMRclickListview(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL & /*bHandled*/)
{
	auto selectedItems = listview.GetSelectedItems();
	if (selectedItems.empty() == false)
	{
		PopupMenu(this->m_hWnd, IDR_MENU_RIGHT);
	}

	return 0;
}


LRESULT DialogMain::OnOpenWithNotepad(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	auto selectedItems = listview.GetSelectedItems();
	for (auto i : selectedItems)
	{
		auto filename = listview.GetItemText(i, static_cast<int>(ListViewColumn::FILENAME));

		wstring cmd = L"notepad " + filename;

		WinExec(to_string(cmd).c_str(), SW_SHOWNORMAL);
	}

	return 0;
}


LRESULT DialogMain::OnRemoveItem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	auto selectedItems = listview.GetSelectedItems();
	for (auto itor = selectedItems.rbegin(); itor != selectedItems.rend(); ++itor)
	{
		int i = *itor;
		auto filename = listview.GetItemText(i, static_cast<int>(ListViewColumn::FILENAME));
		listview.DeleteItem(i);
		listFileNames.erase(filename);
	}
	return 0;
}


LRESULT DialogMain::OnEnChangeEditIncludeText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL & /*bHandled*/)
{
	// ȡ���ַ���
	tstring filterStr;

	BSTR bstr = nullptr;
	CEdit edit(hWndCtl);
	edit.GetWindowTextW(bstr);
	filterStr = bstr;
	SysReleaseString(bstr);

	// ֱ��д��
	core.SetFilterRule(filterStr);

	return 0;
}
