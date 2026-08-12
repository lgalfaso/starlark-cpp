// Copyright 2026 Lucas Mirelmann

#include <gmock/gmock.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <functional>
#include <map>
#include <string>
#include <utility>

#include "interpreter/interpreter.hpp"

using ::starlark::grammar::grammar_options;
using ::starlark::interpreter::frame;
using ::starlark::interpreter::interpreter;
using ::starlark::interpreter::kv_module_loader;
using ::starlark::interpreter::module_info;
using ::starlark::logging::logger;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_obj;
using ::testing::SizeIs;

namespace {

std::string print_logs(logger& logging) {
  std::string result;
  for (const auto& entry : logging) {
    result += std::format("Error at {}\n{}\n", entry.pos().ShortDebugString(), entry.message());
  }
  return result;
}

TEST(Interpreter, ModuleLoading) {
  std::string starlark_code = R"starlark(
load(":foo.star", "foo")

a = foo()
)starlark";
  std::string module_code = R"starlark(
def foo():
  return "Hello from another module"
)starlark";

  interpreter runner;
  logger logging;

  std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>> modules;
  std::map<std::string, starlark_obj*, std::less<>> custom_binding;
  modules.try_emplace("main", starlark_code, custom_binding);
  modules.try_emplace(":foo.star", module_code, custom_binding);
  kv_module_loader loader{modules};

  auto result = runner.run(loader, "main", grammar_options{}, runtime_options{}, logging);
  ASSERT_TRUE(result.ok()) << print_logs(logging);
  ASSERT_THAT((*result)->elements, SizeIs(1));
  ASSERT_NE(nullptr, (*result)->elements[0]);
  EXPECT_EQ((*result)->elements[0]->str(), "Hello from another module");
}

TEST(Interpreter, ModuleLoadingModuleNotFound) {
  std::string starlark_code = R"starlark(
load(":foo.star", "foo")

a = foo()
)starlark";

  interpreter runner;
  logger logging;

  std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>> modules;
  std::map<std::string, starlark_obj*, std::less<>> custom_binding;
  modules.try_emplace("main", starlark_code, custom_binding);
  kv_module_loader loader{modules};

  auto result = runner.run(loader, "main", grammar_options{}, runtime_options{}, logging);
  ASSERT_FALSE(result.ok());
  ASSERT_THAT(logging, SizeIs(1));
  EXPECT_EQ(logging.begin()->message(), "ModuleNotFoundError: No module named ':foo.star'");
}

TEST(Interpreter, ModuleLoadingSymbolNotFound) {
  std::string starlark_code = R"starlark(
load(":foo.star", "bar")

a = bar()
)starlark";
  std::string module_code = R"starlark(
def foo():
  return "Hello from another module"
)starlark";

  interpreter runner;
  logger logging;

  std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>> modules;
  std::map<std::string, starlark_obj*, std::less<>> custom_binding;
  modules.try_emplace("main", starlark_code, custom_binding);
  modules.try_emplace(":foo.star", module_code, custom_binding);
  kv_module_loader loader{modules};

  auto result = runner.run(loader, "main", grammar_options{}, runtime_options{}, logging);
  ASSERT_FALSE(result.ok());
  ASSERT_THAT(logging, SizeIs(1));
  EXPECT_EQ(logging.begin()->message(), "LoadError: Module ':foo.star' does not contain the symbol bar");
}

TEST(Interpreter, MultipleModuleLoading_1) {
  std::string starlark_code = R"starlark(
load(":foo.star", "foo")
load(":bar.star", "bar")

a = foo() + bar()
)starlark";

  std::string foo_module_code = R"starlark(
def foo():
  return "this is foo - "
)starlark";

  std::string bar_module_code = R"starlark(
load(":foo.star", "foo")

def bar():
  return "this is bar - " + foo() + "this is also bar - "
)starlark";

  interpreter runner;
  logger logging;

  std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>> modules;
  std::map<std::string, starlark_obj*, std::less<>> custom_binding;
  modules.try_emplace("main", starlark_code, custom_binding);
  modules.try_emplace(":foo.star", foo_module_code, custom_binding);
  modules.try_emplace(":bar.star", bar_module_code, custom_binding);
  kv_module_loader loader{modules};

  auto result = runner.run(loader, "main", grammar_options{}, runtime_options{}, logging);
  ASSERT_TRUE(result.ok()) << print_logs(logging);
  ASSERT_THAT((*result)->elements, SizeIs(1));
  ASSERT_NE(nullptr, (*result)->elements[0]);
  EXPECT_EQ((*result)->elements[0]->str(), "this is foo - this is bar - this is foo - this is also bar - ");
}

TEST(Interpreter, MultipleModuleLoading_2) {
  std::string starlark_code = R"starlark(
load(":bar.star", "bar")
load(":foo.star", "foo")

a = foo() + bar()
)starlark";

  std::string foo_module_code = R"starlark(
def foo():
  return "this is foo - "
)starlark";

  std::string bar_module_code = R"starlark(
load(":foo.star", "foo")

def bar():
  return "this is bar - " + foo() + "this is also bar - "
)starlark";

  interpreter runner;
  logger logging;

  std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>> modules;
  std::map<std::string, starlark_obj*, std::less<>> custom_binding;
  modules.try_emplace("main", starlark_code, custom_binding);
  modules.try_emplace(":foo.star", foo_module_code, custom_binding);
  modules.try_emplace(":bar.star", bar_module_code, custom_binding);
  kv_module_loader loader{modules};

  auto result = runner.run(loader, "main", grammar_options{}, runtime_options{}, logging);
  ASSERT_TRUE(result.ok()) << print_logs(logging);
  ASSERT_THAT((*result)->elements, SizeIs(1));
  ASSERT_NE(nullptr, (*result)->elements[0]);
  EXPECT_EQ((*result)->elements[0]->str(), "this is foo - this is bar - this is foo - this is also bar - ");
}

TEST(Interpreter, ModuleLoadingWithRecursion_1) {
  std::string starlark_code = R"starlark(
load(":foo.star", "foo")

foo()
)starlark";

  std::string foo_module_code = R"starlark(
load(":bar.star", "bar")
def foo():
  return "this is foo - " + bar()
)starlark";

  std::string bar_module_code = R"starlark(
load(":foo.star", "foo")

def bar():
  return "this is bar"

def shell():
  return "this is bar:shell - " + foo()
)starlark";

  interpreter runner;
  logger logging;

  std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>> modules;
  std::map<std::string, starlark_obj*, std::less<>> custom_binding;
  modules.try_emplace("main", starlark_code, custom_binding);
  modules.try_emplace(":foo.star", foo_module_code, custom_binding);
  modules.try_emplace(":bar.star", bar_module_code, custom_binding);
  kv_module_loader loader{modules};

  auto result = runner.run(loader, "main", grammar_options{}, runtime_options{}, logging);
  ASSERT_FALSE(result.ok());
  ASSERT_THAT(logging, SizeIs(1));
  EXPECT_EQ(logging.begin()->message(), "recursion found during module lookup\n    main\n+-> :foo.star\n|   :bar.star\n+-> :foo.star\n");
}

TEST(Interpreter, ModuleLoadingWithRecursion_2) {
  std::string starlark_code = R"starlark(
load(":foo.star", "foo")

foo()
)starlark";

  std::string foo_module_code = R"starlark(
load(":foo.star", "foo")
abc = foo
)starlark";

  interpreter runner;
  logger logging;

  std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>> modules;
  std::map<std::string, starlark_obj*, std::less<>> custom_binding;
  modules.try_emplace("main", starlark_code, custom_binding);
  modules.try_emplace(":foo.star", foo_module_code, custom_binding);
  kv_module_loader loader{modules};

  auto result = runner.run(loader, "main", grammar_options{}, runtime_options{}, logging);
  ASSERT_FALSE(result.ok());
  ASSERT_THAT(logging, SizeIs(1));
  EXPECT_EQ(logging.begin()->message(), "recursion found during module lookup\n    main\n+-> :foo.star\n+-> :foo.star\n");
}

TEST(Interpreter, ModuleLoadingWithRecursion_3) {
  std::string starlark_code = R"starlark(
load(":a.star", "a")

a()
)starlark";

  std::string a_module_code = R"starlark(
load(":b.star", "b")
load(":c.star", "c")
a = c
)starlark";
  std::string b_module_code = R"starlark(
load(":a.star", "a")
load(":c.star", "c")
b = c
)starlark";
  std::string c_module_code = R"starlark(
c = "Hi"
)starlark";

  interpreter runner;
  logger logging;

  std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>> modules;
  std::map<std::string, starlark_obj*, std::less<>> custom_binding;
  modules.try_emplace("main", starlark_code, custom_binding);
  modules.try_emplace(":a.star", a_module_code, custom_binding);
  modules.try_emplace(":b.star", b_module_code, custom_binding);
  modules.try_emplace(":c.star", c_module_code, custom_binding);
  kv_module_loader loader{modules};

  auto result = runner.run(loader, "main", grammar_options{}, runtime_options{}, logging);
  ASSERT_FALSE(result.ok());
  ASSERT_THAT(logging, SizeIs(1));
  EXPECT_EQ(logging.begin()->message(), "recursion found during module lookup\n    main\n+-> :a.star\n|   :b.star\n+-> :a.star\n");
}

class bad_module_loader : public kv_module_loader {
 public:
  explicit bad_module_loader(const std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>>& values)
      : kv_module_loader(values) {}

 protected:
  std::string cannonical_name(std::string_view module_name, std::string_view caller_module_name) override {
    if (module_name == ":foo.star") {
      if (foo_retrieved) {
        return ":bar.star";
      }
      foo_retrieved = true;
    }
    return kv_module_loader::cannonical_name(module_name, caller_module_name);
  }

 private:
  bool foo_retrieved = false;
};

TEST(Interpreter, FailToLoadModuleAtExecutionTime) {
  std::string starlark_code = R"starlark(
load(":foo.star", "foo")

a = foo()
)starlark";

  std::string foo_module_code = R"starlark(
def foo():
  return "this is foo"
)starlark";

  interpreter runner;
  logger logging;

  std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>> modules;
  std::map<std::string, starlark_obj*, std::less<>> custom_binding;
  modules.try_emplace("main", starlark_code, custom_binding);
  modules.try_emplace(":foo.star", foo_module_code, custom_binding);
  bad_module_loader loader{modules};

  auto result = runner.run(loader, "main", grammar_options{}, runtime_options{}, logging);
  ASSERT_FALSE(result.ok());
  ASSERT_THAT(logging, SizeIs(1));
  EXPECT_EQ(logging.begin()->message(), "ModuleNotFoundError: Unable to load module named ':foo.star'");
}

TEST(Interpreter, FailToTranslateModuleAtExecutionTime) {
  std::string starlark_code = R"starlark(
load(":foo.star", "foo")

a = foo()
)starlark";

  std::string foo_module_code = R"starlark(
def foo():
  return "this is foo"
)starlark";

  interpreter runner;
  logger logging;

  std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>> modules;
  std::map<std::string, starlark_obj*, std::less<>> custom_binding;
  modules.try_emplace("main", starlark_code, custom_binding);
  modules.try_emplace(":foo.star", foo_module_code, custom_binding);
  modules.try_emplace(":bar.star", foo_module_code, custom_binding);
  bad_module_loader loader{modules};

  auto result = runner.run(loader, "main", grammar_options{}, runtime_options{}, logging);
  ASSERT_FALSE(result.ok());
  ASSERT_THAT(logging, SizeIs(1));
  EXPECT_EQ(logging.begin()->message(), "LoadError: Module ':foo.star' is not ready to be used");
}

TEST(Interpreter, ObjectsInModuleAreFrozen_1) {
  std::string starlark_code = R"starlark(
load(":foo.star", "foo")

foo.append(1)
)starlark";

  std::string foo_module_code = R"starlark(
foo = []
)starlark";

  interpreter runner;
  logger logging;

  std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>> modules;
  std::map<std::string, starlark_obj*, std::less<>> custom_binding;
  modules.try_emplace("main", starlark_code, custom_binding);
  modules.try_emplace(":foo.star", foo_module_code, custom_binding);
  kv_module_loader loader{modules};

  auto result = runner.run(loader, "main", grammar_options{}, runtime_options{}, logging);
  ASSERT_FALSE(result.ok());
  ASSERT_THAT(logging, SizeIs(1));
  EXPECT_EQ(logging.begin()->message(), "trying to mutate a frozen list value\n    4 | foo.append(1)\n      | ~~~~~~~~~~^^^\n");
}

TEST(Interpreter, ObjectsInModuleAreFrozen_2) {
  std::string starlark_code = R"starlark(
load(":foo.star", "foo")

foo()
)starlark";

  std::string foo_module_code = R"starlark(
def foo(x = []):
  x.append(1)
)starlark";

  interpreter runner;
  logger logging;

  std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>> modules;
  std::map<std::string, starlark_obj*, std::less<>> custom_binding;
  modules.try_emplace("main", starlark_code, custom_binding);
  modules.try_emplace(":foo.star", foo_module_code, custom_binding);
  kv_module_loader loader{modules};

  auto result = runner.run(loader, "main", grammar_options{}, runtime_options{}, logging);
  ASSERT_FALSE(result.ok());
  ASSERT_THAT(logging, SizeIs(1));
  EXPECT_EQ(logging.begin()->message(), "trying to mutate a frozen list value\n    3 |   x.append(1)\n      |   ~~~~~~~~^^^\n");
}

}  // namespace

