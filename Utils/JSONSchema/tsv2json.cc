/*******************************************************************************
The MIT License (MIT)

Copyright (c) 2016 Dmitry "Dima" Korolev <dmitry.korolev@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#include <iostream>
#include <string>
#include <sstream>

#include "../../TypeSystem/Serialization/json.h"

#include "../../Bricks/dflags/dflags.h"

#include "../../Bricks/strings/printf.h"
#include "../../Bricks/strings/split.h"

DEFINE_bool(header, false, "Set to treat the first row of the data as the header, and extract field names from it.");
DEFINE_string(separator, "\t", "The characters to use as separators in the input TSV/CSV file.");

std::string ExcelColName(size_t i) {
  if (i < 26) {
    return std::string(1, 'A' + i);
  } else {
    return ExcelColName((i / 26) - 1) + static_cast<char>('A' + (i % 26));
  }
}

std::string MakeValidIdentifier(const std::string& s) {
  if (s.empty()) {
    return "_";
  } else {
    std::string result;

    for (char c : s) {
      if (std::isalnum(c)) {
        result += c;
      } else {
        result += current::strings::Printf("x%02X", static_cast<int>(static_cast<unsigned char>(c)));
      }
    }

    return std::isalpha(result.front()) ? result : "_" + result;
  }
}

int main(int argc, char** argv) {
  ParseDFlags(&argc, &argv);
  std::vector<std::map<std::string, std::string>> output;
  std::string row_as_string;
  bool header_to_parse = FLAGS_header;
  std::vector<std::string> field_names;
  while (std::getline(std::cin, row_as_string)) {
    if (!row_as_string.empty()) {
      const std::vector<std::string> fields =
          current::strings::Split(row_as_string, FLAGS_separator, current::strings::EmptyFields::Keep);
      if (header_to_parse) {
        std::unordered_map<std::string, size_t> key_counters;  // To convert "X,X,X" into "X,X2,X3".
        for (const std::string& field : fields) {
          std::string key = MakeValidIdentifier(field);
          const size_t index = ++key_counters[key];
          if (index > 1) {
            // NOTE(dkorolev): Yes, this may create duplicates. For now it's by design.
            key += current::ToString(index);
          }
          field_names.emplace_back(std::move(key));
        }
        header_to_parse = false;
      } else {
        std::map<std::string, std::string> row;
        for (size_t i = 0; i < fields.size(); ++i) {
          row[i < field_names.size() ? field_names[i] : ExcelColName(i)] = fields[i];
        }
        output.emplace_back(std::move(row));
      }
    }
  }
  std::cout << JSON(output) << std::endl;
}
