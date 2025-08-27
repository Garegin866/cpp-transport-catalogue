#pragma once

#include "json.h"

#include <optional>
#include <vector>
#include <string>
#include <stdexcept>
#include <utility>

namespace json {

    class Builder {
    private:
        class BaseContext;
        class DictContext;
        class KeyContext;
        class ArrayContext;

    public:
        Builder::DictContext StartDict();
        Builder::ArrayContext StartArray();
        Builder::BaseContext Value(Node::Value value);
        Builder::KeyContext Key(std::string key);
        Node Build();

    private:
        enum class Type { Dict, Array };
        struct Context { Type type; Node* node; };

        void EnsureNotReady(const char* where);
        void EnsureRootNotSetTwice(const char* where);
        void EnsureInDictForKey(const char* where);
        void EnsureCanPlaceValue(const char* where);
        void PushContainer(Node container, Type type);
        void PlaceValue(Node&& value);

        [[nodiscard]] bool Ready() const noexcept {
            return has_root_ && context_stack_.empty() && !pending_key_.has_value();
        }

        Node root_{};
        bool has_root_ = false;
        std::vector<Context>       context_stack_;
        std::optional<std::string> pending_key_;

    private:
        class BaseContext {
        public:
            explicit BaseContext(Builder& builder) : b_(builder) {}

            Node Build() { return b_.Build(); }

            KeyContext Key(std::string key);
            BaseContext& Value(Node::Value value);
            DictContext  StartDict();
            ArrayContext StartArray();
            BaseContext EndDict();
            BaseContext  EndArray();

        protected:
            Builder &b_;
        };

        class DictContext : private BaseContext {
        public:
            explicit DictContext(Builder& b) : BaseContext(b) {}

            using BaseContext::EndDict;
            using BaseContext::Key;
        };

        class KeyContext : private BaseContext {
        public:
            explicit KeyContext(Builder& b) : BaseContext(b) {}

            DictContext Value(Node::Value v);

            DictContext StartDict()  { return BaseContext::StartDict(); }
            ArrayContext StartArray() { return BaseContext::StartArray(); }
        };

        class ArrayContext : private BaseContext {
        public:
            explicit ArrayContext(Builder& b) : BaseContext(b) {}

            ArrayContext Value(Node::Value v);

            using BaseContext::StartDict;
            using BaseContext::StartArray;
            using BaseContext::EndArray;
        };
    };

} // namespace json
