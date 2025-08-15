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

    class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:
        using variant::variant;

        [[nodiscard]] bool IsInt() const { return std::holds_alternative<int>(*this); }
        [[nodiscard]] bool IsDouble() const { return IsInt() || std::holds_alternative<double>(*this); }
        [[nodiscard]] bool IsPureDouble() const { return std::holds_alternative<double>(*this); }
        [[nodiscard]] bool IsBool() const { return std::holds_alternative<bool>(*this); }
        [[nodiscard]] bool IsString() const { return std::holds_alternative<std::string>(*this); }
        [[nodiscard]] bool IsArray() const { return std::holds_alternative<Array>(*this); }
        [[nodiscard]] bool IsMap() const { return std::holds_alternative<Dict>(*this); }

        [[nodiscard]] const std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>& GetValue() const;
        [[nodiscard]] int AsInt() const;
        [[nodiscard]] double AsDouble() const;
        [[nodiscard]] bool AsBool() const;
        [[nodiscard]] const std::string& AsString() const;
        [[nodiscard]] const Array& AsArray() const;
        [[nodiscard]] const Dict& AsMap() const;
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
