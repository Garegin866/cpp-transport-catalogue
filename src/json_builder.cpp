#include "json_builder.h"

#include <stdexcept>
#include <utility>

namespace json {

    Builder::DictContext Builder::StartDict() {
        EnsureNotReady("StartDict()");
        EnsureCanPlaceValue("StartDict()");
        PushContainer(Node(Dict{}), Type::Dict);
        return DictContext(*this);
    }

    Builder::ArrayContext Builder::StartArray() {
        EnsureNotReady("StartArray()");
        EnsureCanPlaceValue("StartArray()");
        PushContainer(Node(Array{}), Type::Array);
        return ArrayContext(*this);
    }

    Builder::BaseContext Builder::Value(Node::Value value) {
        EnsureNotReady("Value()");
        EnsureCanPlaceValue("Value()");
        PlaceValue(Node(std::move(value)));
        return BaseContext(*this); // after top-level scalar, only Build() makes sense
    }

    Builder::KeyContext Builder::Key(std::string key) {
        EnsureNotReady("Key()");
        EnsureInDictForKey("Key()");
        pending_key_ = std::move(key);
        return KeyContext(*this);
    }

    Node Builder::Build() {
        if (!has_root_) {
            throw std::logic_error("Build(): объект не задан");
        }
        if (!context_stack_.empty()) {
            throw std::logic_error("Build(): есть незакрытые контейнеры");
        }
        if (pending_key_.has_value()) {
            throw std::logic_error("Build(): в словаре остался ключ без значения");
        }
        return root_;
    }

    void Builder::EnsureNotReady(const char* where) {
        if (Ready()) {
            throw std::logic_error(std::string(where) + ": объект уже построен");
        }
    }

    void Builder::EnsureRootNotSetTwice(const char* where) {
        if (has_root_ && context_stack_.empty()) {
            throw std::logic_error(std::string(where) + ": корень уже определён");
        }
    }

    void Builder::EnsureInDictForKey(const char* where) {
        if (context_stack_.empty() || context_stack_.back().type != Type::Dict) {
            throw std::logic_error(std::string(where) + ": Key() допустим только внутри словаря");
        }
        if (pending_key_.has_value()) {
            throw std::logic_error(std::string(where) + ": предыдущий Key() ещё не получил значение");
        }
    }

    void Builder::EnsureCanPlaceValue(const char* where) {
        if (!has_root_ && context_stack_.empty()) {
            return;
        }
        if (!context_stack_.empty()) {
            const auto& top = context_stack_.back();
            if (top.type == Type::Array) {
                return;
            }
            if (top.type == Type::Dict && pending_key_.has_value()) {
                return;
            }
        }
        throw std::logic_error(std::string(where) + ": значение допустимо только после конструктора, после Key() или внутри массива");
    }

    void Builder::PlaceValue(Node&& value) {
        if (!has_root_ && context_stack_.empty()) {
            root_ = std::move(value);
            has_root_ = true;
            return;
        }

        if (context_stack_.empty()) {
            throw std::logic_error("Value(): невозможно добавить значение — объект уже готов");
        }

        auto& top = context_stack_.back();
        if (top.type == Type::Array) {
            top.node->AsArray().emplace_back(std::move(value));
        } else {
            if (!pending_key_.has_value()) {
                throw std::logic_error("Value(): перед добавлением значения в словарь нужно вызвать Key()");
            }
            top.node->AsDict().emplace(std::move(*pending_key_), std::move(value));
            pending_key_.reset();
        }
    }

    void Builder::PushContainer(Node container, Type type) {
        if (!has_root_ && context_stack_.empty()) {
            root_ = std::move(container);
            has_root_ = true;
            context_stack_.push_back(Context{type, &root_});
            return;
        }

        if (context_stack_.empty()) {
            throw std::logic_error("PushContainer: неверный контекст");
        }

        auto& top = context_stack_.back();
        if (top.type == Type::Array) {
            top.node->AsArray().emplace_back(std::move(container));
            Node& back = top.node->AsArray().back();
            context_stack_.push_back(Context{type, &back});
        } else {
            if (!pending_key_.has_value()) {
                throw std::logic_error("PushContainer: в словаре перед началом контейнера нужно вызвать Key()");
            }
            auto [it, inserted] = top.node->AsDict().emplace(std::move(*pending_key_), std::move(container));
            pending_key_.reset();
            Node& placed = it->second;
            context_stack_.push_back(Context{type, &placed});
        }
    }

    // ================= BaseContext =================

    Builder::KeyContext Builder::BaseContext::Key(std::string key) {
        b_.EnsureNotReady("Key()");
        b_.EnsureInDictForKey("Key()");
        b_.pending_key_ = std::move(key);
        return KeyContext(b_);
    }

    Builder::BaseContext& Builder::BaseContext::Value(Node::Value value) {
        b_.EnsureNotReady("Value()");
        b_.EnsureCanPlaceValue("Value()");
        b_.PlaceValue(Node(std::move(value)));
        return *this;
    }

    Builder::DictContext Builder::BaseContext::StartDict() {
        b_.EnsureNotReady("StartDict()");
        b_.EnsureCanPlaceValue("StartDict()");
        b_.PushContainer(Node(Dict{}), Type::Dict);
        return DictContext(b_);
    }

    Builder::ArrayContext Builder::BaseContext::StartArray() {
        b_.EnsureNotReady("StartArray()");
        b_.EnsureCanPlaceValue("StartArray()");
        b_.PushContainer(Node(Array{}), Type::Array);
        return ArrayContext(b_);
    }

    Builder::BaseContext Builder::BaseContext::EndDict() {
        b_.EnsureNotReady("EndDict()");
        if (b_.context_stack_.empty() || b_.context_stack_.back().type != Type::Dict) {
            throw std::logic_error("EndDict(): текущий контекст не словарь");
        }
        if (b_.pending_key_.has_value()) {
            throw std::logic_error("EndDict(): для последнего Key() не задано значение");
        }
        b_.context_stack_.pop_back();
        return BaseContext(b_);
    }

    Builder::BaseContext Builder::BaseContext::EndArray() {
        b_.EnsureNotReady("EndArray()");
        if (b_.context_stack_.empty() || b_.context_stack_.back().type != Type::Array) {
            throw std::logic_error("EndArray(): текущий контекст не массив");
        }
        b_.context_stack_.pop_back();
        return BaseContext(b_);
    }

    // ================= KeyContext =================

    Builder::DictContext Builder::KeyContext::Value(Node::Value v) {
        b_.EnsureNotReady("Value()");
        b_.EnsureCanPlaceValue("Value()");
        b_.PlaceValue(Node(std::move(v)));
        return DictContext(b_);
    }

    // ================= ArrayContext =================

    Builder::ArrayContext Builder::ArrayContext::Value(Node::Value v) {
        b_.EnsureNotReady("Value()");
        b_.EnsureCanPlaceValue("Value()");
        b_.PlaceValue(Node(std::move(v)));
        return ArrayContext(b_);
    }
} // namespace json
