#include "string.hpp"
#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <sstream>

#include "nt.hpp"

namespace utils::string
{
	const char* va(const char* fmt, ...)
	{
		static thread_local va_provider<8, 256> provider;

		va_list ap;
		va_start(ap, fmt);

		const char* result = provider.get(fmt, ap);

		va_end(ap);
		return result;
	}

	std::vector<std::string> split(const std::string& s, const char delim)
	{
		std::stringstream ss(s);
		std::string item;
		std::vector<std::string> elems;

		while (std::getline(ss, item, delim))
		{
			elems.push_back(item); // elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
		}

		return elems;
	}

	std::string to_lower(std::string text)
	{
		std::transform(text.begin(), text.end(), text.begin(), [](const unsigned char input)
		{
			return static_cast<char>(std::tolower(input));
		});

		return text;
	}

	std::string to_upper(std::string text)
	{
		std::transform(text.begin(), text.end(), text.begin(), [](const unsigned char input)
		{
			return static_cast<char>(std::toupper(input));
		});

		return text;
	}

	bool starts_with(const std::string& text, const std::string& substring)
	{
		return text.find(substring) == 0;
	}

	bool ends_with(const std::string& text, const std::string& substring)
	{
		if (substring.size() > text.size()) return false;
		return std::equal(substring.rbegin(), substring.rend(), text.rbegin());
	}

	bool is_numeric(const std::string& text)
	{
		auto it = text.begin();
		while (it != text.end() && std::isdigit(static_cast<unsigned char>(*it)))
		{
			++it;
		}

		return !text.empty() && it == text.end();
	}

	std::string dump_hex(const std::string& data, const std::string& separator)
	{
		std::string result;

		for (unsigned int i = 0; i < data.size(); ++i)
		{
			if (i > 0)
			{
				result.append(separator);
			}

			result.append(va("%02X", data[i] & 0xFF));
		}

		return result;
	}

	std::string get_clipboard_data()
	{
		if (OpenClipboard(nullptr))
		{
			std::string data;

			auto* const clipboard_data = GetClipboardData(1u);
			if (clipboard_data)
			{
				auto* const cliptext = static_cast<char*>(GlobalLock(clipboard_data));
				if (cliptext)
				{
					data.append(cliptext);
					GlobalUnlock(clipboard_data);
				}
			}
			CloseClipboard();

			return data;
		}
		return {};
	}

	void strip(const char* in, char* out, size_t max)
	{
		assert(max);
		if (!in || !out) return;

		max--;
		size_t current = 0;
		while (*in != '\0' && current < max)
		{
			const auto color_index = (*(in + 1) - 48) >= 0xC ? 7 : (*(in + 1) - 48);

			if (*in == '^' && (color_index != 7 || *(in + 1) == '7'))
			{
				++in;
			}
			else
			{
				*out = *in;
				++out;
				++current;
			}

			++in;
		}

		*out = '\0';
	}

	void strip_material(const char* in, char* out, size_t max)
	{
		assert(max);
		if (!in || !out) return;

		size_t i = 0;
		while (*in != '\0' && i < max - 1)
		{
			if (*in != '$' && *in != '{' && *in != '}')
			{
				*out++ = *in;
				++i;
			}
			++in;
		}

		*out = '\0';
	}

	std::string convert(const std::wstring& wstr)
	{
		const int count = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);

		std::string str(count, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), str.data(), count, nullptr, nullptr);

		return str;
	}

	std::wstring convert(const std::string& str)
	{
		const auto count = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);

		std::wstring wstr(count, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), wstr.data(), count);

		return wstr;
	}

	std::string replace(std::string str, const std::string& from, const std::string& to)
	{
		if (from.empty())
		{
			return str;
		}

		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos)
		{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}

		return str;
	}

	std::string& ltrim(std::string& str)
	{
		str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](const unsigned char input)
		{
			return !std::isspace(input);
		}));

		return str;
	}

	std::string& rtrim(std::string& str)
	{
		str.erase(std::find_if(str.rbegin(), str.rend(), [](const  unsigned char input)
		{
			return !std::isspace(input);
		}).base(), str.end());

		return str;
	}

	void trim(std::string& str)
	{
		ltrim(rtrim(str));
	}

	void copy(char* dest, const size_t max_size, const char* src)
	{
		if (!max_size)
		{
			return;
		}

		for (size_t i = 0;; ++i)
		{
			if (i + 1 == max_size)
			{
				dest[i] = 0;
				break;
			}

			dest[i] = src[i];

			if (!src[i])
			{
				break;
			}
		}
	}
}
