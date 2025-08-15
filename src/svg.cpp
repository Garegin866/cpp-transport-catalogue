#include "svg.h"

namespace transport_catalogue::svg {

    using namespace std::literals;

    std::ostream& operator<< (std::ostream& out, StrokeLineCap cap) {
        switch (cap) {
            case StrokeLineCap::BUTT:   out << "butt"; break;
            case StrokeLineCap::ROUND:  out << "round"; break;
            case StrokeLineCap::SQUARE: out << "square"; break;
        }

        return out;
    }

    std::ostream& operator<< (std::ostream& out, StrokeLineJoin join) {
        switch (join) {
            case StrokeLineJoin::ARCS:        out << "arcs"; break;
            case StrokeLineJoin::BEVEL:       out << "bevel"; break;
            case StrokeLineJoin::MITER:       out << "miter"; break;
            case StrokeLineJoin::MITER_CLIP:  out << "miter-clip"; break;
            case StrokeLineJoin::ROUND:       out << "round"; break;
        }

        return out;
    }

    std::ostream& operator<< (std::ostream& out, const Color& color) {
        if (std::holds_alternative<std::monostate>(color)) {
            out << "none";
        } else if (std::holds_alternative<std::string>(color)) {
            out << std::get<std::string>(color);
        } else if (std::holds_alternative<Rgb>(color)) {
            const auto& rgb = std::get<Rgb>(color);
            out << "rgb(" << static_cast<int>(rgb.red) << ","
                << static_cast<int>(rgb.green) << ","
                << static_cast<int>(rgb.blue) << ")";
        } else if (std::holds_alternative<Rgba>(color)) {
            const auto& rgba = std::get<Rgba>(color);
            out << "rgba(" << static_cast<int>(rgba.red) << ","
                << static_cast<int>(rgba.green) << ","
                << static_cast<int>(rgba.blue) << ","
                << rgba.opacity << ")";
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

// ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center)  {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)  {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

// ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.emplace_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\"";
        bool first = true;
        for (const auto& point : points_) {
            if (!first) out << " ";
            out << point.x << "," << point.y;
            first = false;
        }
        out << "\"";

        RenderAttrs(context.out);
        out << "/>"sv;
    }

// ---------- Text ------------------

    Text& Text::SetPosition(Point position) {
        position_ = position;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\"";

        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\"";
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\"";
        }

        RenderAttrs(context.out);
        out << ">"sv;
        out << EscapeText(data_);
        out << "</text>"sv;
    }


    std::string Text::EscapeText(const std::string& data) {
        std::string result;
        for (char c : data) {
            switch (c) {
                case '\"': result += "&quot;"; break;
                case '\'': result += "&apos;"; break;
                case '<':  result += "&lt;"; break;
                case '>':  result += "&gt;"; break;
                case '&':  result += "&amp;"; break;
                default:   result += c;
            }
        }
        return result;
    }

// ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& object_ptr) {
        if (object_ptr) {
            objects_.push_back(std::move(object_ptr));
        }
    }

    void Document::Render(std::ostream& out) const {
        RenderContext ctx(out, 2);
        out << R"(<?xml version="1.0" encoding="UTF-8" ?>)"sv << std::endl;
        out << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)"sv << std::endl;
        for (const auto& obj : objects_) {
            obj->Render(ctx);
        }
        out << "</svg>"sv;
    }

}  // namespace transport_catalogue::svg