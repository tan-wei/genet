#include "filter.hpp"
#include "frame.hpp"
#include "frame_view.hpp"
#include "layer.hpp"
#include "private/variant.hpp"
#include "property.hpp"
#include "wrapper/frame.hpp"
#include "wrapper/layer.hpp"
#include "wrapper/property.hpp"
#include <json11.hpp>
#include <nan.h>

namespace plugkit {

using namespace v8;

typedef std::function<Filter::Result(const FrameView *)> FilterFunc;

class Filter::Private {
public:
  Local<Value> fetchValue(Local<Value> value) const;
  Local<Value> fetchValue(const Filter::Result &result) const;
  FilterFunc makeFilter(const json11::Json &json) const;
  FilterFunc makeFilter(const std::string &str) const;

public:
  FilterFunc func;
};

Local<Value> Filter::Private::fetchValue(Local<Value> value) const {
  if (value.IsEmpty())
    return Nan::Null();
  auto result = value;
  if (result->IsObject()) {
    auto resultObj = result.As<Object>();
    auto resultKey = Nan::New("_filter").ToLocalChecked();
    if (const auto &prop = PropertyWrapper::unwrap(resultObj)) {
      result = Variant::Private::getValue(prop->value());
    } else if (resultObj->Has(resultKey)) {
      result = resultObj->Get(resultKey);
    }
  }
  return result;
}

Local<Value> Filter::Private::fetchValue(const Filter::Result &result) const {
  return fetchValue(result.value);
}

FilterFunc Filter::Private::makeFilter(const json11::Json &json) const {
  Isolate *isolate = Isolate::GetCurrent();
  const std::string &type = json["type"].string_value();

  if (type == "MemberExpression") {
    const json11::Json &property = json["property"];
    const std::string &propertyType = property["type"].string_value();
    FilterFunc propertyFunc;

    if (propertyType == "Identifier") {
      const std::string &name = property["name"].string_value();
      propertyFunc = [this, isolate, name](const FrameView *view) {
        return Filter::Result(Nan::New(name).ToLocalChecked());
      };
    } else {
      propertyFunc = makeFilter(property);
    }

    const FilterFunc &objectFunc = makeFilter(json["object"]);

    return FilterFunc([this, isolate, objectFunc,
                       propertyFunc](const FrameView *view) -> Filter::Result {
      Local<Value> value = objectFunc(view).value;
      Local<Value> property = propertyFunc(view).value;
      Local<Value> result;

      const std::string &name = *Nan::Utf8String(property);
      if (name.empty())
        return result;

      if (value->IsObject()) {
        const Property *child = nullptr;
        if (const auto &layer = LayerWrapper::unwrapConst(value.As<Object>())) {
          child = layer->propertyFromId(strid(name.c_str()));
        } else if (const auto &prop =
                       PropertyWrapper::unwrap(value.As<Object>())) {
          child = prop->propertyFromId(strid(name.c_str()));
        }
        if (child) {
          result = PropertyWrapper::wrap(child);
        }
      }

      value = fetchValue(value);

      if (result.IsEmpty()) {
        if (value->IsString()) {
          value = StringObject::New(value.As<String>());
        }
        if (value->IsObject()) {
          Local<Object> object = value.As<Object>();
          Local<Value> key = Nan::New(name).ToLocalChecked();
          if (object->Has(key)) {
            result = object->Get(key);
          }
        }
      }

      if (result.IsEmpty()) {
        return Filter::Result(Nan::Null());
      }

      return Filter::Result(result, value);
    });
  } else if (type == "BinaryExpression") {
    const FilterFunc &lf = makeFilter(json["left"]);
    const FilterFunc &rf = makeFilter(json["right"]);
    const std::string &op = json["operator"].string_value();

    if (op == ">") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(Nan::New(fetchValue(lf(view))->NumberValue() >
                                       fetchValue(rf(view))->NumberValue()));
      });
    } else if (op == "<") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(Nan::New(fetchValue(lf(view))->NumberValue() <
                                       fetchValue(rf(view))->NumberValue()));
      });
    } else if (op == ">=") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(Nan::New(fetchValue(lf(view))->NumberValue() >=
                                       fetchValue(rf(view))->NumberValue()));
      });
    } else if (op == "<=") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(Nan::New(fetchValue(lf(view))->NumberValue() <=
                                       fetchValue(rf(view))->NumberValue()));
      });
    } else if (op == "==") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(
            Nan::New(fetchValue(lf(view))->Equals(fetchValue(rf(view)))));
      });
    } else if (op == "!=") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(
            Nan::New(!fetchValue(lf(view))->Equals(fetchValue(rf(view)))));
      });
    } else if (op == "+") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(Nan::New(fetchValue(lf(view))->NumberValue() +
                                       fetchValue(rf(view))->NumberValue()));
      });
    } else if (op == "-") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(Nan::New(fetchValue(lf(view))->NumberValue() +
                                       fetchValue(rf(view))->NumberValue()));
      });
    } else if (op == "*") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(Nan::New(fetchValue(lf(view))->NumberValue() +
                                       fetchValue(rf(view))->NumberValue()));
      });
    } else if (op == "/") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(Nan::New(fetchValue(lf(view))->NumberValue() +
                                       fetchValue(rf(view))->NumberValue()));
      });
    } else if (op == "%") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(Nan::New(fetchValue(lf(view))->Int32Value() %
                                       fetchValue(rf(view))->Int32Value()));
      });
    } else if (op == "&") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(Nan::New(fetchValue(lf(view))->Int32Value() &
                                       fetchValue(rf(view))->Int32Value()));
      });
    } else if (op == "|") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(Nan::New(fetchValue(lf(view))->Int32Value() |
                                       fetchValue(rf(view))->Int32Value()));
      });
    } else if (op == "^") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(Nan::New(fetchValue(lf(view))->Int32Value() ^
                                       fetchValue(rf(view))->Int32Value()));
      });
    } else if (op == ">>") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(Nan::New(fetchValue(lf(view))->Int32Value() >>
                                       fetchValue(rf(view))->Int32Value()));
      });
    } else if (op == "<<") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        return Filter::Result(Nan::New(fetchValue(lf(view))->Int32Value()
                                       << fetchValue(rf(view))->Int32Value()));
      });
    }
  } else if (type == "LogicalExpression") {
    const std::string &op = json["operator"].string_value();
    const FilterFunc &lf = makeFilter(json["left"]);
    const FilterFunc &rf = makeFilter(json["right"]);
    if (op == "||") {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        auto value = lf(view).value;
        return Filter::Result(fetchValue(value)->BooleanValue() ? value
                                                                : rf(view));
      });
    } else {
      return FilterFunc([this, isolate, lf, rf](const FrameView *view) {
        auto value = lf(view).value;
        return Filter::Result(!fetchValue(value)->BooleanValue() ? value
                                                                 : rf(view));
      });
    }
  } else if (type == "UnaryExpression") {
    const FilterFunc &func = makeFilter(json["argument"]);
    const std::string &op = json["operator"].string_value();
    if (op == "+") {
      return FilterFunc(
          [this, isolate, func](const FrameView *view) -> Local<Value> {
            return Nan::New(fetchValue(func(view))->NumberValue());
          });
    } else if (op == "-") {
      return FilterFunc(
          [this, isolate, func](const FrameView *view) -> Local<Value> {
            return Nan::New(-fetchValue(func(view))->NumberValue());
          });
    } else if (op == "!") {
      return FilterFunc(
          [this, isolate, func](const FrameView *view) -> Local<Value> {
            return Nan::New(!fetchValue(func(view))->BooleanValue());
          });
    } else if (op == "~") {
      return FilterFunc(
          [this, isolate, func](const FrameView *view) -> Local<Value> {
            return Nan::New(~fetchValue(func(view))->Int32Value());
          });
    }

  } else if (type == "CallExpression") {
    const FilterFunc &cf = makeFilter(json["callee"]);
    std::vector<FilterFunc> argFuncs;
    for (const json11::Json &item : json["arguments"].array_items()) {
      argFuncs.push_back(makeFilter(item));
    }
    return FilterFunc([this, isolate, cf,
                       argFuncs](const FrameView *view) -> Local<Value> {
      const Filter::Result &result = cf(view);
      Local<Value> func = result.value;
      if (func->IsFunction()) {
        std::vector<Local<Value>> args;
        for (const FilterFunc &arg : argFuncs) {
          args.push_back(arg(view).value);
        }
        Local<Value> receiver = isolate->GetCurrentContext()->Global();
        if (!result.parent.IsEmpty()) {
          receiver = result.parent;
        }
        return func.As<Function>()->Call(receiver, args.size(), args.data());
      }
      return Nan::Null();
    });

  } else if (type == "Literal") {
    const json11::Json &regex = json["regex"];
    if (regex.is_object()) {
      const std::string &value = json["raw"].string_value();
      return FilterFunc(
          [this, isolate, value](const FrameView *view) -> Local<Value> {
            auto script = Nan::CompileScript(Nan::New(value).ToLocalChecked());
            if (!script.IsEmpty()) {
              auto result = Nan::RunScript(script.ToLocalChecked());
              if (!result.IsEmpty()) {
                return result.ToLocalChecked();
              }
            }
            return Nan::Null();
          });
    } else {
      const std::string &value = json["value"].dump();
      return FilterFunc(
          [this, isolate, value](const FrameView *view) -> Local<Value> {
            auto context = isolate->GetCurrentContext();
            return JSON::Parse(context, Nan::New(value).ToLocalChecked())
                .ToLocalChecked();
          });
    }

  } else if (type == "ConditionalExpression") {
    const FilterFunc &tf = makeFilter(json["test"]);
    const FilterFunc &cf = makeFilter(json["consequent"]);
    const FilterFunc &af = makeFilter(json["alternate"]);
    return FilterFunc([this, isolate, tf, cf, af](const FrameView *view) {
      return Filter::Result(fetchValue(tf(view))->BooleanValue() ? cf(view)
                                                                 : af(view));
    });
  } else if (type == "Identifier") {
    const std::string &name = json["name"].string_value();
    return FilterFunc([this, isolate, name](const FrameView *view) {

      Local<Value> key = Nan::New(name).ToLocalChecked();
      Local<Object> frameObject = FrameWrapper::wrap(view);

      if (const auto &prop = view->propertyFromId(strid(name.c_str()))) {
        return Filter::Result(PropertyWrapper::wrap(prop));
      }

      if (frameObject->Has(key)) {
        return Filter::Result(frameObject->Get(key));
      }

      if (const auto &layer = view->layerFromId(strid(name.c_str()))) {
        Local<Object> layerObject = LayerWrapper::wrap(layer);
        return Filter::Result(layerObject);
      }

      if (name == "$") {
        return Filter::Result(frameObject);
      }
      auto global = isolate->GetCurrentContext()->Global();
      if (global->Has(key)) {
        return Filter::Result(global->Get(key));
      }
      return Filter::Result(Nan::Null());
    });
  }

  return FilterFunc(
      [](const FrameView *fram) { return Filter::Result(Nan::Null()); });
}

FilterFunc Filter::Private::makeFilter(const std::string &str) const {
  std::string err;
  const json11::Json &json = json11::Json::parse(str, err);
  auto filter = makeFilter(json);
  return FilterFunc([this, filter](const FrameView *view) -> Local<Value> {
    return fetchValue(filter(view));
  });
}

Filter::Filter(const std::string &body) : d(new Private()) {
  d->func = d->makeFilter(body);
}

Filter::~Filter() {}

bool Filter::test(const FrameView *view) const {
  return d->func(view).value->BooleanValue();
}
}
