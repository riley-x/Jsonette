# Jsonette
An exercise to create a pico-minimal json reader. 

### Basic usage:

```c++
#include "jsonette.h"
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

vector<string> const & keys = j.get_keys(); // ["menu"]
vector<JSON> const & vals = j.get_vals(); 
for (JSON const & val : vals) cout << val.type() << endl; // [Object]

string item_value0 = j["menu"]["popup"]["menuitem"][0]["value"].get_str(); // "New"

cout << j.to_string() << endl; // pretty print
cout << j.to_string(false) << endl; // compact print
```

### Various value types:

```c++
JSON j("[true, false, null, 123, -3.14e1, "hello world", {"key": "value"}]");
cout << j.type() << endl; // JType::Array

vector<JSON> const & vals = j.get_arr();
for (JSON const & val : vals) cout << val.type() << endl; // [True, False, Null, Integer, Double, String, Object]

bool b = j[0].get<bool>(); // true
bool b2 = j[1].get_bool(); // false
bool b3 = j[2].is_null(); // true

int i = j[3].get<int>(); // 123
int64_t i2 = j[3].get_int(); // 123

double d = j[4].get<double>(); // -31.4
double d2 = j[4].get_dbl(); // -31.4

string s = j[5].get<string>(); // "hello world"
string s2 = j[5].get_str(); // "hello world"

JSON const & j2 = j[6];
string s3 = j2["key"]; // "value"
```

