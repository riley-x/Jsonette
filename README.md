# Jsonette
A pico-minimal json reader.

### Usage examples:

```c++
using namespace std;
using namespace jsonette;

// from https://json.org/example.html
std::string text(R"( 
  {
    "menu": {
      "id": "file",
      "value": "File",
      "popup": {
        "menuitem": [
          {"value": "New", "onclick": "CreateNewDoc()"},
          {"value": "Open", "onclick": "OpenDoc()"},
          {"value": "Close", "onclick": "CloseDoc()"}
        ]
      }
    } 
  }
)");

JSON j(text);
cout << j.type() << endl; // JType::Object
vector<string> const & keys = j.get_keys(); // ["menu", "id", "value", "popup"]
vector<JSON> const & vals = j.get_vals(); 
for (JSON const & val : vals) cout << val.type() << endl; // [String, String, Object]
string item_value0 = j["menu"]["popup"]["menuitem"][0]["value"].get_str(); // "New"

cout << j.to_string() << endl; // pretty print
cout << j.to_string(false) << endl; // compact print

JSON j2("[true, false, null, 123, -3.14e1]");
cout << j2.type() << endl; // JType::Array
vector<JSON> const & vals2 = j2.get_arr();
for (JSON const & val : vals2) cout << val.type() << endl; // [True, False, Null, Integer, Double]
bool b = j2[0].get<bool>(); // true
bool b2 = j2[1].get_bool(); // false
bool b3 = j2[2].is_null(); // true
int i2 = j2[3].get<int>(); // 123
int64_t i = j2[3].get_int(); // 123
double d = j2[4].get<double>(); // -31.4
double d2 = j2[4].get_dbl(); // -31.4
```

