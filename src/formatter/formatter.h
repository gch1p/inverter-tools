// SPDX-License-Identifier: BSD-3-Clause

#ifndef INVERTER_TOOLS_PRINT_H
#define INVERTER_TOOLS_PRINT_H

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <ios>
#include <iomanip>
#include <nlohmann/json.hpp>

#include "src/util.h"

namespace formatter {

using nlohmann::json;
using nlohmann::ordered_json;


/**
 * Enumerations
 */

enum class Unit {
    None = 0,
    V,
    A,
    Wh,
    VA,
    Hz,
    Percentage,
    Celsius,
};

enum class Format {
    Table,
    SimpleTable,
    JSON,
    SimpleJSON,
};
std::ostream& operator<<(std::ostream& os, Unit val);


/**
 * Helper functions
 */

template <typename T>
std::string to_str(T& v) {
    std::ostringstream buf;
    buf << v;
    return buf.str();
}


/**
 * Items
 */

template <typename T>
struct TableItem {
    explicit TableItem(std::string key, std::string title, T value, Unit unit = Unit::None, unsigned precision = 0) :
        key(std::move(key)),
        title(std::move(title)),
        value(value),
        unit(unit) {}

    std::string key;
    std::string title;
    T value;
    Unit unit;
};

template <typename T>
struct ListItem {
    explicit ListItem(T value) : value(value) {}
    T value;
};


/**
 * Items holders
 */

class Formattable {
protected:
    Format format_;

public:
    explicit Formattable(Format format) : format_(format) {}
    virtual ~Formattable() = default;

    virtual std::ostream& writeJSON(std::ostream& os) const = 0;
    virtual std::ostream& writeSimpleJSON(std::ostream& os) const = 0;
    virtual std::ostream& writeTable(std::ostream& os) const = 0;
    virtual std::ostream& writeSimpleTable(std::ostream& os) const = 0;

    friend std::ostream& operator<<(std::ostream& os, Formattable const& ref) {
        switch (ref.format_) {
            case Format::Table:
                return ref.writeTable(os);

            case Format::SimpleTable:
                return ref.writeSimpleTable(os);

            case Format::JSON:
                return ref.writeJSON(os);

            case Format::SimpleJSON:
                return ref.writeSimpleJSON(os);
        }

        return os;
    }
};


// T must have `operator<<`, `json toJSON()` and `json toSimpleJSON()` methods
template <typename T>
class Table : public Formattable {
protected:
    std::vector<TableItem<T>> v_;

public:
    explicit Table(Format format, std::vector<TableItem<T>> v)
        : Formattable(format), v_(v) {}

    void push(TableItem<T> item) {
        v_.push_back(item);
    }

    std::ostream& writeSimpleTable(std::ostream& os) const override {
        for (const auto& item: v_) {
            os << item.key << " ";

            std::string value = to_str(item.value);
            bool space = string_has(value, ' ');
            if (space)
                os << "\"";
            os << value;
            if (space)
                os << "\"";

            if (item.unit != Unit::None)
                os << " " << item.unit;

            if (&item != &v_.back())
                os << std::endl;
        }
        return os;
    }

    std::ostream& writeTable(std::ostream& os) const override {
        int maxWidth = 0;
        for (const auto& item: v_) {
            int width = item.title.size()+1 /* colon */;
            if (width > maxWidth)
                maxWidth = width;
        }

        std::ios_base::fmtflags f(os.flags());
        os << std::left;
        for (const auto &item: v_) {
            os << std::setw(maxWidth) << (item.title+":") << " " << item.value;

            if (item.unit != Unit::None)
                os << " " << item.unit;

            if (&item != &v_.back())
                os << std::endl;
        }

        os.flags(f);
        return os;
    }

    std::ostream& writeJSON(std::ostream& os) const override {
        ordered_json j = {
            {"result", "ok"},
            {"data", {}}
        };
        for (const auto &item: v_) {
            if (item.unit != Unit::None) {
                json jval = json::object();
                jval["value"] = item.value.toJSON();
                jval["unit"] = to_str(item.unit);
                j["data"][item.key] = jval;
            } else {
                j["data"][item.key] = item.value.toJSON();
            }
        }
        return os << j.dump();
    }

    std::ostream& writeSimpleJSON(std::ostream& os) const override {
        ordered_json j = {
            {"result", "ok"},
            {"data", {}}
        };
        for (const auto &item: v_) {
            j["data"][item.key] = item.value.toSimpleJSON();
        }
        return os << j.dump();
    }
};

template <typename T>
class List : public Formattable {
protected:
    std::vector<ListItem<T>> v_;

public:
    explicit List(Format format, std::vector<ListItem<T>> v)
        : Formattable(format), v_(v) {}

    std::ostream& writeSimpleTable(std::ostream& os) const override {
        return writeTable(os);
    }

    std::ostream& writeTable(std::ostream& os) const override {
        for (const auto &item: v_) {
            os << item.value;
            if (&item != &v_.back())
                os << std::endl;
        }
        return os;
    }

    std::ostream& writeJSON(std::ostream& os) const override {
        json data = {};
        ordered_json j;

        j["result"] = "ok";

        for (const auto &item: v_)
            data.push_back(item.value.toJSON());
        j["data"] = data;

        return os << j.dump();
    }

    std::ostream& writeSimpleJSON(std::ostream& os) const override {
        json data = {};
        ordered_json j;

        j["result"] = "ok";

        for (const auto &item: v_)
            data.push_back(item.value.toSimpleJSON());
        j["data"] = data;

        return os << j.dump();
    }
};

class Status : public Formattable {
protected:
    bool value_;
    std::string message_;

public:
    explicit Status(Format format, bool value, std::string message)
        : Formattable(format), value_(value), message_(std::move(message)) {}

    std::ostream& writeSimpleTable(std::ostream& os) const override {
        return writeTable(os);
    }

    std::ostream& writeTable(std::ostream& os) const override {
        os << (value_ ? "ok" : "error");
        if (!message_.empty())
            os << ": " << message_;
        return os;
    }

    std::ostream& writeJSON(std::ostream& os) const override {
        ordered_json j = {
            {"result", (value_ ? "ok" : "error")}
        };
        if (!message_.empty())
            j["message"] = message_;
        return os << j.dump();
    }

    std::ostream& writeSimpleJSON(std::ostream& os) const override {
        return writeJSON(os);
    }
};

}

#endif