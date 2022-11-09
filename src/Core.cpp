#include "Core.h"

#include <FileFunction.h>

#include <unicode/ucnv.h>

#include <stdexcept>

#ifdef _DEBUG
#include <iostream>
#endif

#undef min
#undef max

using namespace std;

// �ַ���code�����Ƶ�ӳ���
const doublemap<CharsetCode, tstring> charsetCodeMap = {
	{CharsetCode::UTF8,TEXT("UTF-8")},
	{CharsetCode::UTF8BOM,TEXT("UTF-8 BOM")},
	{CharsetCode::UTF16LE,TEXT("UTF-16LE")},
	{CharsetCode::UTF16LEBOM,TEXT("UTF-16LE BOM")},
	{CharsetCode::UTF16BE,TEXT("UTF-16BE")},
	{CharsetCode::UTF16BEBOM,TEXT("UTF-16BE BOM")},
	{CharsetCode::GB18030,TEXT("GB18030")},
	{CharsetCode::WINDOWS_1252,TEXT("windows-1252")},
	{CharsetCode::UNKNOWN,TEXT("δ֪")}
};

std::unordered_set<CharsetCode> Configuration::normalCharset = {
	CharsetCode::UTF8,CharsetCode::UTF8BOM,CharsetCode::GB18030
};

std::tstring ToCharsetName(CharsetCode code)
{
	return charsetCodeMap[code];
}

CharsetCode ToCharsetCode(const std::tstring &name)
{
	return charsetCodeMap[name];
}

bool HasBom(CharsetCode code)
{
	switch (code)
	{
	case CharsetCode::UTF8BOM:
	case CharsetCode::UTF16LEBOM:
	case CharsetCode::UTF16BEBOM:
		return true;
	}
	return false;
}

const char *GetBomData(CharsetCode code)
{
	switch (code)
	{
	case CharsetCode::UTF8BOM:
		return UTF8BOM_DATA;
	case CharsetCode::UTF16LEBOM:
		return UTF16LEBOM_DATA;
	case CharsetCode::UTF16BEBOM:
		return UTF16BEBOM_DATA;
	}
	return nullptr;
}

int BomSize(CharsetCode code)
{
	switch (code)
	{
	case CharsetCode::UTF8BOM:
		return sizeof(UTF8BOM_DATA);
	case CharsetCode::UTF16LEBOM:
		return sizeof(UTF16LEBOM_DATA);
	case CharsetCode::UTF16BEBOM:
		return sizeof(UTF16BEBOM_DATA);
	}
	return 0;
}


std::string ToICUCharsetName(CharsetCode code)
{
	switch (code)
	{
	case CharsetCode::UTF8BOM:
		return "UTF-8";
	case CharsetCode::UTF16LEBOM:
		return "UTF-16LE";
	case CharsetCode::UTF16BEBOM:
		return "UTF-16BE";
	}
	return to_string(charsetCodeMap[code]);
}

void DealWithUCNVError(UErrorCode err)
{
	switch (err)
	{
	case U_ZERO_ERROR:
		break;
	case U_AMBIGUOUS_ALIAS_WARNING:	// windows-1252 ʱ����������ʱ����
		break;
	default:
		throw runtime_error("ucnv����code=" + to_string(err));
		break;
	}
}

// ����code���ַ��������ַ���
tuple<unique_ptr<UChar[]>, int> Decode(const char *str, size_t len, CharsetCode code)
{
	// ��codeת����icu���ַ�������
	auto icuCharsetName = ToICUCharsetName(code);

	UErrorCode err = U_ZERO_ERROR;

	// ��ת����
	UConverter *conv = ucnv_open(to_string(icuCharsetName).c_str(), &err);
	DealWithUCNVError(err);

	size_t cap = len + 1;
	unique_ptr<UChar[]> target(new UChar[cap]);

	// ����
	int retLen = ucnv_toUChars(conv, target.get(), cap, str, len, &err);
	DealWithUCNVError(err);

	ucnv_close(conv);

	return make_tuple<unique_ptr<UChar[]>, int32_t>(std::move(target), std::move(retLen));
}

std::tuple<std::unique_ptr<char[]>, int> Encode(const std::unique_ptr<UChar[]> &buf, uint64_t bufSize, CharsetCode targetCode)
{
	// ��codeת����icu���ַ�������
	auto icuCharsetName = ToICUCharsetName(targetCode);

	UErrorCode err = U_ZERO_ERROR;

	// ��ת����
	UConverter *conv = ucnv_open(to_string(icuCharsetName).c_str(), &err);
	DealWithUCNVError(err);

	size_t cap = bufSize * sizeof(UChar) + 2;
	unique_ptr<char[]> target(new char[cap]);

	// ����
	int retLen = ucnv_fromUChars(conv, target.get(), cap, buf.get(), bufSize, &err);
	DealWithUCNVError(err);

	ucnv_close(conv);
	return make_tuple(std::move(target), retLen);
}

Configuration::LineBreaks GetLineBreaks(const unique_ptr<UChar[]> &buf, int len)
{
	Configuration::LineBreaks ans = Configuration::LineBreaks::EMPTY;
	for (int i = 0; i < len; )
	{
		UChar &c = buf.get()[i];
		if (c == UChar(u'\r'))
		{
			// \r\n
			if (i < len && buf.get()[i + 1] == UChar(u'\n'))
			{
				if (ans == Configuration::LineBreaks::EMPTY)
				{
					ans = Configuration::LineBreaks::CRLF;
				}
				else
				{
					if (ans != Configuration::LineBreaks::CRLF)
					{
						ans = Configuration::LineBreaks::MIX;
						return ans;
					}
				}
				i += 2;
				continue;
			}

			// \r
			if (ans == Configuration::LineBreaks::EMPTY)
			{
				ans = Configuration::LineBreaks::CR;
			}
			else
			{
				if (ans != Configuration::LineBreaks::CR)
				{
					ans = Configuration::LineBreaks::MIX;
					return ans;
				}
			}
			i++;
			continue;
		}

		// \n
		if (c == UChar(u'\n'))
		{
			if (ans == Configuration::LineBreaks::EMPTY)
			{
				ans = Configuration::LineBreaks::LF;
			}
			else
			{
				if (ans != Configuration::LineBreaks::LF)
				{
					ans = Configuration::LineBreaks::MIX;
					return ans;
				}
			}
			i++;
			continue;
		}

		i++;
	}
	return ans;
}

void ChangeLineBreaks(std::unique_ptr<UChar[]> &buf, int &len, Configuration::LineBreaks targetLineBreak)
{
	vector<UChar> out;
	out.reserve(len);

	vector<UChar> lineBreak;
	switch (targetLineBreak)
	{
	case Configuration::LineBreaks::CRLF:
		lineBreak = { u'\r',u'\n' };
		break;
	case Configuration::LineBreaks::LF:
		lineBreak = { u'\n' };
		break;
	case Configuration::LineBreaks::CR:
		lineBreak = { u'\r' };
		break;
	}

	for (int i = 0; i < len; )
	{
		UChar &c = buf.get()[i];
		if (c == UChar(u'\r'))
		{
			// \r\n
			if (i < len && buf.get()[i + 1] == UChar(u'\n'))
			{
				out.insert(out.end(), lineBreak.begin(), lineBreak.end());
				i += 2;
				continue;
			}

			// \r
			out.insert(out.end(), lineBreak.begin(), lineBreak.end());
			i ++;
			continue;
		}

		if (c == UChar(u'\n'))
		{
			out.insert(out.end(), lineBreak.begin(), lineBreak.end());
			i++;
			continue;
		}

		out.push_back(c);
		i++;
	}

	int outLen = out.size();
	buf.reset(new UChar[outLen]);
	memcpy(buf.get(), out.data(), out.size() * sizeof(UChar));
	len = outLen;

	return;
}

Core::Core(std::tstring iniFileName) :iniFileName(iniFileName)
{
	// ��ini
	ReadFromIni();

	// ��ʼ��uchardet
	det = unique_ptr<uchardet, std::function<void(uchardet *)>>(uchardet_new(), [](uchardet *det) { uchardet_delete(det); });

	//UErrorCode err;
	//auto allNames = ucnv_openAllNames(&err);
	//while (1)
	//{
	//	auto name = uenum_next(allNames, nullptr, &err);
	//	if (name == nullptr)
	//	{
	//		break;
	//	}
	//	cout << name << endl;
	//}
}

const Configuration &Core::GetConfig() const
{
	return config;
}

void Core::SetFilterMode(Configuration::FilterMode mode)
{
	config.filterMode = mode;
	WriteToIni();
}

void Core::SetFilterRule(std::tstring rule)
{
	config.includeRule = rule;
	WriteToIni();
}

void Core::SetOutputTarget(Configuration::OutputTarget outputTarget)
{
	config.outputTarget = outputTarget;
	WriteToIni();
}

void Core::SetOutputDir(std::tstring outputDir)
{
	config.outputDir = outputDir;
	WriteToIni();
}

void Core::SetOutputCharset(CharsetCode outputCharset)
{
	config.outputCharset = outputCharset;
	WriteToIni();
}

void Core::SetLineBreaks(Configuration::LineBreaks lineBreak)
{
	config.lineBreak = lineBreak;
	WriteToIni();
}

void Core::SetEnableConvertLineBreak(bool enableLineBreaks)
{
	config.enableConvertLineBreaks = enableLineBreaks;
}

std::tuple<CharsetCode, std::unique_ptr<UChar[]>, int32_t> Core::GetEncoding(std::tstring filename) const
{
	// ֻ��ȡ100KB
	auto [buf, bufSize] = ReadFileToBuffer(filename, 100 * KB);

	// ��uchardet�ж��ַ���
	uchardet_reset(det.get());
	int ret = uchardet_handle_data(det.get(), buf.get(), bufSize);
	switch (ret)
	{
	case HANDLE_DATA_RESULT_NEED_MORE_DATA:
	case HANDLE_DATA_RESULT_DETECTED:
		break;
	case HANDLE_DATA_RESULT_ERROR:
		throw runtime_error("uchardet fail");
	}

	uchardet_data_end(det.get());

	auto charset = string(uchardet_get_charset(det.get()));

	// filter
	CharsetCode code;
	if (charset == "ASCII" || charset == "ANSI")
	{
		code = CharsetCode::UTF8;
	}
	else if (charset == "UTF-8")
	{
		// ��������BOM
		if (bufSize > sizeof(UTF8BOM_DATA) && memcmp(buf.get(), UTF8BOM_DATA, sizeof(UTF8BOM_DATA)) == 0)
		{
			code = CharsetCode::UTF8BOM;
		}
		else
		{
			code = CharsetCode::UTF8;
		}
	}
	else if (charset == "UTF-16LE")
	{
		// ��������BOM
		if (bufSize > sizeof(UTF16LEBOM_DATA) && memcmp(buf.get(), UTF16LEBOM_DATA, sizeof(UTF16LEBOM_DATA)) == 0)
		{
			code = CharsetCode::UTF16LEBOM;
		}
		else
		{
			code = CharsetCode::UTF16LE;
		}
	}
	else if (charset == "UTF-16BE")
	{
		// ��������BOM
		if (bufSize > sizeof(UTF16BEBOM_DATA) && memcmp(buf.get(), UTF16BEBOM_DATA, sizeof(UTF16BEBOM_DATA)) == 0)
		{
			code = CharsetCode::UTF16BEBOM;
		}
		else
		{
			code = CharsetCode::UTF16BE;
		}
	}
	else if (charset == "GB18030")
	{
		code = CharsetCode::GB18030;
	}
	else if (charset == "Windows-1252")
	{
		code = CharsetCode::WINDOWS_1252;
	}
	else if (charset == "")	// ûʶ�����
	{
		code = CharsetCode::UNKNOWN;
		return make_tuple(code, nullptr, 0);
	}
	else
	{
		string info = "�ݲ�֧�֣�";
		info += charset;
		info += "������ϵ���ߡ�";
		throw runtime_error(info);
	}

	// ����uchardet�ó����ַ�������
	auto content = Decode(buf.get(), std::max(64ULL, bufSize), code);

	return make_tuple(code, std::move(get<0>(content)), get<1>(content));
}

void Core::ReadFromIni()
{
}

void Core::WriteToIni()
{
}

//UINT Configuration::ToWinCodePage(OutputCharset charset)
//{
//	switch (charset)
//	{
//	case OutputCharset::UTF8:
//		return CP_UTF8;
//	case OutputCharset::GB18030:
//		return CP_GB18030;
//	}
//}
