#pragma once

#include <map>
#include <string>
#include <variant>
#include <vector>
#include <stdexcept>

namespace transport_catalogue::json {

    class Node;

    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value = std::variant<std::nullptr_t, int, double, bool, std::string, Array, Dict>;

        Node() : value_(nullptr) {}
        Node(std::nullptr_t) : value_(nullptr) {}

        Node(int v) : value_(v) {}
        Node(double v) : value_(v) {}
        Node(bool v) : value_(v) {}
        Node(const char* v) : value_(std::string(v)) {}
        Node(std::string v) : value_(std::move(v)) {}
        Node(Array v) : value_(std::move(v)) {}
        Node(Dict v) : value_(std::move(v)) {}

        [[nodiscard]] bool IsNull() const { return std::holds_alternative<std::nullptr_t>(value_); }
        [[nodiscard]] bool IsInt() const { return std::holds_alternative<int>(value_); }
        [[nodiscard]] bool IsDouble() const { return IsInt() || std::holds_alternative<double>(value_); }
        [[nodiscard]] bool IsPureDouble() const { return std::holds_alternative<double>(value_); }
        [[nodiscard]] bool IsBool() const { return std::holds_alternative<bool>(value_); }
        [[nodiscard]] bool IsString() const { return std::holds_alternative<std::string>(value_); }
        [[nodiscard]] bool IsArray() const { return std::holds_alternative<Array>(value_); }
        [[nodiscard]] bool IsMap() const { return std::holds_alternative<Dict>(value_); }

        [[nodiscard]] Value GetValue() const { return value_; }
        int AsInt() const;
        double AsDouble() const;
        bool AsBool() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        bool operator==(const Node& rhs) const { return value_ == rhs.value_; }
        bool operator!=(const Node& rhs) const { return !(*this == rhs); }

    private:
        Value value_;
    };

    class Document {
    public:
        explicit Document(Node root) : root_(std::move(root)) {}
        [[nodiscard]] const Node& GetRoot() const { return root_; }

    private:
        Node root_;
    };

    Document Load(std::istream& input);
    void Print(const Document& doc, std::ostream& output);

    bool operator==(const Document& lhs, const Document& rhs);
    bool operator!=(const Document& lhs, const Document& rhs);

}  // namespace transport_catalogue::json
