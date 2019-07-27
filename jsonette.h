#ifndef JSONETTE_H
#define JSONETTE_H

#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>

namespace jsonette
{
	enum class JType { object, array, int64, dbl, string, tru, fal, null };

	// A class representing a JSON value
	class JSON
	{
	public:
		///////////////////////////////////////////////////////////////////////
		// Constructors/Destructors
		///////////////////////////////////////////////////////////////////////
		JSON() : m_type(JType::null), m_data(nullptr) {} // null value
		JSON(std::string const & str)
		{
			init(str, 0, str.size());
		}

		JSON(JSON&& other) : m_type(other.m_type), m_data(other.m_data)
		{
			other.m_type = JType::null;
			other.m_data = nullptr;
		}

		JSON& operator=(JSON&& other)
		{
			if (this != &other)
			{
				clear();
				m_type = other.m_type;
				m_data = other.m_data;
				other.m_type = JType::null;
				other.m_data = nullptr;
			}
			return *this;
		}

		~JSON() { clear(); }

		inline void clear()
		{
			switch (m_type)
			{
			case JType::object:
				delete reinterpret_cast<Obj*>(m_data);
				break;
			case JType::array:
				delete reinterpret_cast<std::vector<JSON>*>(m_data);
				break;
			case JType::int64:
				delete reinterpret_cast<int64_t*>(m_data);
				break;
			case JType::dbl:
				delete reinterpret_cast<double*>(m_data);
				break;
			case JType::string:
				delete reinterpret_cast<std::string*>(m_data);
				break;
			default:
				break;
			}
			m_type = JType::null;
			m_data = nullptr;
		}

		///////////////////////////////////////////////////////////////////////
		// Get
		///////////////////////////////////////////////////////////////////////

		inline bool is_null() const { return m_type == JType::null; }
		inline JType const type() const { return m_type; }
		inline std::string to_string(bool pretty = true) const { return to_string(pretty, 0); }

		// Use explicit templates definied below
		template<typename T> inline T get() const { throw_type_error(); }

		// Object -- store a pair of vectors for keys/values
		inline std::vector<std::string> const & get_keys() const
		{
			if (m_type != JType::object) throw_type_error();
			Obj *data = reinterpret_cast<Obj*>(m_data);
			return data->keys;
		}
		inline std::vector<JSON> const & get_vals() const
		{
			if (m_type != JType::object) throw_type_error();
			Obj *data = reinterpret_cast<Obj*>(m_data);
			return data->vals;
		}
		inline JSON const & operator[] (std::string const & str) const // throws on error
		{
			if (m_type != JType::object) throw_type_error();
			Obj *data = reinterpret_cast<Obj*>(m_data);
			auto it = std::find(data->keys.begin(), data->keys.end(), str);
			if (it == data->keys.end()) throw_indx_error(str);
			return data->vals[std::distance(data->keys.begin(), it)];
		}

		// Array -- vector of JSON values
		inline std::vector<JSON> const & get_arr() const
		{
			if (m_type != JType::array) throw_type_error();
			return *reinterpret_cast<std::vector<JSON>*>(m_data);
		}
		inline size_t get_arr_size() const
		{
			if (m_type != JType::array) throw_type_error();
			return reinterpret_cast<std::vector<JSON>*>(m_data)->size();
		}
		inline JSON const & operator[] (size_t i) const // segfaults if out of bounds
		{
			if (m_type != JType::array) throw_type_error();
			std::vector<JSON>* data = reinterpret_cast<std::vector<JSON>*>(m_data);
			return (*data)[i];
		}
		template<> inline std::vector<JSON> const & get<std::vector<JSON> const &>() const { return get_arr(); }

		// Integer
		inline int64_t get_int() const
		{
			if (m_type != JType::int64) throw_type_error();
			return *reinterpret_cast<int64_t*>(m_data);
		}
		template<> inline char		get<char>() const { return static_cast<char>(get_int()); }
		template<> inline short		get<short>() const { return static_cast<short>(get_int()); }
		template<> inline int		get<int>() const { return static_cast<int>(get_int()); }
		template<> inline long		get<long>() const { return static_cast<long>(get_int()); }
		template<> inline long long get<long long>() const { return static_cast<long long>(get_int()); }

		// Double
		inline double get_dbl() const
		{
			if (m_type != JType::dbl) throw_type_error();
			return *reinterpret_cast<double*>(m_data);
		}
		template<> inline float  get<float> () const { return static_cast<float>(get_dbl()); }
		template<> inline double get<double>() const { get_dbl(); }

		// String
		inline std::string const & get_str() const
		{
			if (m_type != JType::string) throw_type_error();
			return *reinterpret_cast<std::string*>(m_data);
		}
		template<> inline std::string const & get<std::string const &>() const { return get_str(); }
		template<> inline std::string		  get<std::string>() const { return get_str(); }

		// Bool
		inline bool get_bool() const
		{
			switch (m_type)
			{
			case JType::tru: return true;
			case JType::fal: return false;
			default: throw_type_error();
			}
		}
		template<> inline bool get<bool>() const { return get_bool(); }


	private:

		///////////////////////////////////////////////////////////////////////
		// Data Members
		///////////////////////////////////////////////////////////////////////
		JType m_type;
		void  *m_data;

		struct Obj
		{
			std::vector<std::string> keys;
			std::vector<JSON> vals;
		};

		///////////////////////////////////////////////////////////////////////
		// Utility Functions
		///////////////////////////////////////////////////////////////////////
		inline void throw_type_error() const { throw std::runtime_error("JSON::get() bad type"); }
		inline void throw_read_error(std::string const & str, size_t at) const
		{
			std::cerr << "Bad text: " << str.c_str() + at << std::endl;
			throw std::runtime_error("JSON() parse error");
		}
		inline void throw_indx_error(std::string const & indx) const
		{
			throw std::runtime_error("JSON() bad index: " + indx);
		}

		// Whitespace
		inline bool is_ws(char c)
		{
			return (c == 0x20 || c == 0x0D || c == 0x0A || c == 0x09);
		}
		// Digit
		inline bool is_digit(char c)
		{
			return (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
					c == '5' || c == '6' || c == '7' || c == '8' || c == '9');
		}
		// Finds the next non-whitespace character
		inline size_t next_non_ws(std::string const & str, size_t begin, size_t end)
		{
			while (begin != end)
			{
				if (is_ws(str[begin])) begin++;
				else return begin;
			}
			return begin;
		}

		///////////////////////////////////////////////////////////////////////
		// Helper Constructors
		///////////////////////////////////////////////////////////////////////

		JSON(JType type, void *data) : m_type(type), m_data(data) {}

		// initailizes a JSON object from the first consecutive set of valid characters in str
		// end is a hard cap to end parsing
		// returns the next index in str after the last character used by this value
		size_t init(std::string const & str, size_t begin, size_t end)
		{
			size_t ind = next_non_ws(str, begin, end);
			if (ind >= end) throw_read_error(str, begin);

			if (str[ind] == '{') // is an object
			{
				Obj *data = new Obj;
				ind = next_non_ws(str, ind + 1, end);
				while (ind < end) // loop through object members
				{
					if (str[ind] == '}') break;

					// key is a JSON string
					JSON key;
					ind = key.init(str, ind, end);
					data->keys.emplace_back(key.get_str());

					// ':' divider
					ind = next_non_ws(str, ind, end);
					if (str[ind] != ':') throw_read_error(str, ind);

					// value
					data->vals.emplace_back();
					ind = data->vals.back().init(str, ind + 1, end);
					ind = next_non_ws(str, ind, end);
					if (ind < end && str[ind] == ',') ind++;
				}

				m_type = JType::object;
				m_data = reinterpret_cast<void*>(data);
				return ind + 1;
			}
			else if (str[ind] == '[') // is an array
			{
				std::vector<JSON> *data = new std::vector<JSON>;
				ind = next_non_ws(str, ind + 1, end);
				while (ind < end) // loop through array entries
				{
					if (str[ind] == ']') break;
					data->emplace_back();
					ind = data->back().init(str, ind, end);
					ind = next_non_ws(str, ind, end);
					if (ind < end && str[ind] == ',') ind++;
				}
				m_type = JType::array;
				m_data = reinterpret_cast<void*>(data);
				return ind + 1;
			}
			else if (str[ind] == '"') // is a string
			{
				size_t end = str.find('"', ind + 1); // TODO escape characters
				if (end == std::string::npos) throw_read_error(str, ind);
				m_type = JType::string;
				m_data = reinterpret_cast<void*>(new std::string(str, ind + 1, end - ind - 1));
				return end + 1;
			}
			else if (is_digit(str[ind]) || str[ind] == '-' || str[ind] == '+')// is a number
			{
				// parse as double if '.' or 'e','E' is present
				bool is_dbl = false;
				size_t i = ind + 1;
				while (i < end && is_digit(str[i])) i++;
				if (i < end && (str[i] == '.' || str[i] == 'e' || str[i] == 'E'))
					is_dbl = true;

				char *endptr = nullptr;
				if (is_dbl)
				{
					m_type = JType::dbl;
					double *data = new double(strtod(str.c_str() + ind, &endptr));
					m_data = reinterpret_cast<void*>(data);
				}
				else
				{
					m_type = JType::int64;
					int64_t *data = new int64_t(strtoll(str.c_str() + ind, &endptr, 10));
					m_data = reinterpret_cast<void*>(data);
				}
				if (endptr) return endptr - str.c_str();
				else { throw_read_error(str, ind); return ind; }
			}
			else if (strncmp(str.c_str() + ind, "true", 4) == 0) // is boolean true
			{
				m_type = JType::tru;
				return ind + 4;
			}
			else if (strncmp(str.c_str() + ind, "false", 5) == 0) // is boolean false
			{
				m_type = JType::fal;
				return ind + 5;
			}
			else if (strncmp(str.c_str() + ind, "null", 4) == 0) // null
			{
				m_type = JType::null;
				return ind + 4;
			}
			else
			{
				throw_read_error(str, ind);
				return ind;
			}
		}

		///////////////////////////////////////////////////////////////////////
		// To String
		///////////////////////////////////////////////////////////////////////

		// `indent`: indent level of parent for use by object and array.
		// i.e. closing bracket is at level `indent`, while values should indented one more
		std::string to_string(bool pretty, int indent) const
		{
			const static int tabsize = 2;
			switch (m_type)
			{
			case JType::object:
			{
				std::string str((pretty) ? "{\n" : "{");
				auto const & keys = get_keys();
				auto const & vals = get_vals();
				for (size_t i = 0; i < keys.size(); i++)
				{
					if (pretty) str.append((indent + 1) * tabsize, ' ');
					str.append("\"" + keys[i] + ((pretty) ? "\": " : "\":"));
					str.append(vals[i].to_string(pretty, (pretty) ? indent + 1 : 0));
					if (i != keys.size() - 1) str.append((pretty) ? ",\n" : ",");
				}
				if (pretty) { str.append("\n"); str.append(indent * tabsize, ' '); }
				str.append("}");
				return str;
			}
			case JType::array:
			{
				std::string str((pretty) ? "[\n" : "[");
				auto const & vals = get_arr();
				for (size_t i = 0; i < vals.size(); i++)
				{
					if (pretty) str.append((indent + 1) * tabsize, ' ');
					str.append(vals[i].to_string(pretty, (pretty) ? indent + 1 : 0));
					if (i != vals.size() - 1) str.append((pretty) ? ",\n" : ",");
				}
				if (pretty) { str.append("\n"); str.append(indent * tabsize, ' '); }
				str.append("]");
				return str;
			}
			case JType::int64:
				return std::to_string(get_int());
			case JType::dbl:
				return std::to_string(get_dbl());
			case JType::string:
				return "\"" + get_str() + "\"";
			case JType::tru:
				return "true";
			case JType::fal:
				return "false";
			case JType::null:
				return "null";
			default:
				return "";
			}
		}

		JSON(const JSON &) = delete;
		JSON & operator= (const JSON &) = delete;
	};
}


enum class JType { object, array, int64, dbl, string, tru, fal, null };

inline std::ostream & operator << (std::ostream &out, jsonette::JType type)
{
	switch (type)
	{
	case jsonette::JType::object: out << "object"; break;
	case jsonette::JType::array: out << "array"; break;
	case jsonette::JType::int64: out << "int"; break;
	case jsonette::JType::dbl: out << "double"; break;
	case jsonette::JType::string: out << "string"; break;
	case jsonette::JType::tru: out << "true"; break;
	case jsonette::JType::fal: out << "false"; break;
	case jsonette::JType::null: out << "null"; break;
	}
	return out;
}

#endif // JSONETTE_H