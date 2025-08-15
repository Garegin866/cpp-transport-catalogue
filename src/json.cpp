#include "json.h"

#include <sstream>
#include <stdexcept>
#include <cstring>
#include <vector>

using namespace std;

namespace transport_catalogue::json {

    namespace {

        // === Parsing ===

        void SkipWhitespace(istream& input) {
            while (isspace(input.peek())) input.get();
        }

        Node LoadNull(istream& input) {
            char buffer[4];
            input.read(buffer, 4);
            if (strncmp(buffer, "null", 4) != 0) {
                throw ParsingError("Invalid null");
            }
            if (isalpha(input.peek())) {
                throw ParsingError("Unexpected character after 'null'");
            }
            return Node{nullptr};
        }

        Node LoadBool(istream& input) {
            char buffer[5] = {};
            input.read(buffer, 4);
            if (strncmp(buffer, "true", 4) == 0) {
                if (isalpha(input.peek())) {
                    throw ParsingError("Unexpected character after 'true'");
                }
                return Node{true};
            }
            input.read(buffer + 4, 1);
            if (strncmp(buffer, "false", 5) == 0) {
                if (isalpha(input.peek())) {
                    throw ParsingError("Unexpected character after 'false'");
                }
                return Node{false};
            }
            throw ParsingError("Invalid boolean");
        }


        Node LoadNumber(istream& input) {
            string token;
            while (isdigit(input.peek()) || input.peek() == '-' || input.peek() == '+' || input.peek() == '.' || input.peek() == 'e' || input.peek() == 'E') {
                token += static_cast<char>(input.get());
            }

            if (token.empty()) throw ParsingError("Expected number");

            if (token.find('.') != string::npos || token.find('e') != string::npos || token.find('E') != string::npos) {
                double d;
                istringstream iss(token);
                iss >> d;
                if (iss.fail()) throw ParsingError("Invalid double");
                return Node(d);
            } else {
                int i;
                istringstream iss(token);
                iss >> i;
                if (iss.fail()) throw ParsingError("Invalid int");
                return Node(i);
            }
        }

        std::string ParseString(std::istream& input) {
            std::string result;
            char c;
            while (input.get(c)) {
                if (c == '"') break;
                if (c == '\\') {
                    char esc;
                    if (!input.get(esc)) throw ParsingError("Incomplete escape sequence");
                    switch (esc) {
                        case '"':  result += '"';  break;
                        case 'n':  result += '\n'; break;
                        case 'r':  result += '\r'; break;
                        case 't':  result += '\t'; break;
                        case '\\': result += '\\'; break;
                        default: throw ParsingError("Unknown escape sequence");
                    }
                } else {
                    result += c;
                }
            }
            if (c != '"') throw ParsingError("Unterminated string");
            return result;
        }

        Node LoadString(istream& input) {
            return Node{ParseString(input)};
        }

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array arr;
            SkipWhitespace(input);
            if (input.peek() == ']') {
                input.get();
                return Node{std::move(arr)};
            }

            while (true) {
                arr.push_back(LoadNode(input));
                SkipWhitespace(input);
                char c = input.get();
                if (c == ']') break;
                if (c != ',') throw ParsingError("Expected ',' or ']'");
                SkipWhitespace(input);
            }

            return Node{std::move(arr)};
        }

        Node LoadDict(istream& input) {
            Dict result;
            SkipWhitespace(input);
            if (input.peek() == '}') {
                input.get();
                return Node{std::move(result)};
            }

            while (true) {
                SkipWhitespace(input);
                if (input.get() != '"') throw ParsingError("Expected key string");
                string key = ParseString(input);
                SkipWhitespace(input);
                if (input.get() != ':') throw ParsingError("Expected ':' after key");
                SkipWhitespace(input);
                result[std::move(key)] = LoadNode(input);
                SkipWhitespace(input);
                char c = input.get();
                if (c == '}') break;
                if (c != ',') throw ParsingError("Expected ',' or '}'");
                SkipWhitespace(input);
            }

            return Node{std::move(result)};
        }

        Node LoadNode(istream& input) {
            SkipWhitespace(input);
            char c = input.peek();
            if (c == '[') {
                input.get();
                return LoadArray(input);
            } else if (c == '{') {
                input.get();
                return LoadDict(input);
            } else if (c == '"') {
                input.get();
                return LoadString(input);
            } else if (c == 'n') {
                return LoadNull(input);
            } else if (c == 't' || c == 'f') {
                return LoadBool(input);
            } else if (isdigit(c) || c == '-' || c == '+') {
                return LoadNumber(input);
            }

            throw ParsingError("Unexpected character in JSON");
        }

    }  // namespace

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    const std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>& Node::GetValue() const {
        return *this;
    }

    int Node::AsInt() const {
        if (!std::holds_alternative<int>(*this)) {
            throw std::logic_error("Node is not an int");
        }
        return std::get<int>(*this);
    }

    double Node::AsDouble() const {
        if (std::holds_alternative<int>(*this)) {
            return static_cast<double>(std::get<int>(*this));
        } else if (std::holds_alternative<double>(*this)) {
            return std::get<double>(*this);
        } else {
            throw std::logic_error("Node is not a double");
        }
    }

    bool Node::AsBool() const {
        if (!std::holds_alternative<bool>(*this)) {
            throw std::logic_error("Node is not a bool");
        }
        return std::get<bool>(*this);
    }

    const std::string& Node::AsString() const {
        if (!std::holds_alternative<std::string>(*this)) {
            throw std::logic_error("Node is not a string");
        }
        return std::get<std::string>(*this);
    }

    const Array& Node::AsArray() const {
        if (!std::holds_alternative<Array>(*this)) {
            throw std::logic_error("Node is not an array");
        }
        return std::get<Array>(*this);
    }

    const Dict& Node::AsMap() const {
        if (!std::holds_alternative<Dict>(*this)) {
            throw std::logic_error("Node is not a map");
        }
        return std::get<Dict>(*this);
    }

    // === Printing ===

    void PrintString(const std::string& str, std::ostream& out) {
        out << '"';
        for (char c : str) {
            switch (c) {
                case '"':  out << "\\\""; break;
                case '\\': out << "\\\\"; break;
                case '\n': out << "\\n";  break;
                case '\r': out << "\\r";  break;
                case '\t': out << "\\t";  break;
                default:   out << c;      break;
            }
        }
        out << '"';
    }

    void PrintNode(const Node& node, ostream& out);

    void PrintArray(const Array& arr, ostream& out) {
        out << "[";
        bool first = true;
        for (const auto& item : arr) {
            if (!first) out << ", ";
            PrintNode(item, out);
            first = false;
        }
        out << "]";
    }

    void PrintDict(const Dict& dict, ostream& out) {
        out << "{";
        bool first = true;
        for (const auto& [key, val] : dict) {
            if (!first) out << ", ";
            PrintString(key, out);
            out << ": ";
            PrintNode(val, out);
            first = false;
        }
        out << "}";
    }

    void PrintNode(const Node& node, ostream& out) {
        const auto& variant = node.GetValue();
        std::visit([&out](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>) {
                out << "null";
            } else if constexpr (std::is_same_v<T, int>) {
                out << value;
            } else if constexpr (std::is_same_v<T, double>) {
                out << value;
            } else if constexpr (std::is_same_v<T, bool>) {
                out << (value ? "true" : "false");
            } else if constexpr (std::is_same_v<T, std::string>) {
                PrintString(value, out);
            } else if constexpr (std::is_same_v<T, Array>) {
                PrintArray(value, out);
            } else if constexpr (std::is_same_v<T, Dict>) {
                PrintDict(value, out);
            }
        }, variant);
    }

    void Print(const Document& doc, ostream& out) {
        PrintNode(doc.GetRoot(), out);
    }

}  // namespace transport_catalogue::vjson