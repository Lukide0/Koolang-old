#include "ast/Parser.h"
#include "test.h"

using ast::Parser;

TEST_CASE("Parser - import statement")
{
    File file;
    file.Content = "import a::a;\n"
                   "import a::b = c;\n"
                   "import a::{\n"
                   "    b,\n"
                   "    c = C,\n"
                   "    d::e,\n"
                   "    f::g = G\n"
                   "};";
    Parser p(file, "IMPORT");

    p.Parse();

    CHECK_EQ(file.ErrMsgs.size(), 0);
    CHECK_EQ(file.WarnMsgs.size(), 0);
}

TEST_CASE("Parser - Constant statement")
{
    File file;
    file.Content = "const A : u32 = 4;\n"
                   "const B : (u32, u32) = (5, 5);\n"
                   "const C : [u32;A] = [10;A];";
    Parser p(file, "CONSTANT");

    p.Parse();

    CHECK_EQ(file.ErrMsgs.size(), 0);
    CHECK_EQ(file.WarnMsgs.size(), 0);
}

TEST_CASE("Parser - Variable statement")
{

    File file;
    file.Content = "fn x() { var x : i32 = 5;\n"
                   "var y = 8;\n"
                   "var (x : i32, y : u32) = (5, 6);\n"
                   "var (x, y) = (5, 6);\n"
                   "var A{ field_x -> x, field_y -> y } = something; }";
    Parser p(file, "VARIABLE");

    p.Parse();

    CHECK_EQ(file.ErrMsgs.size(), 0);
    CHECK_EQ(file.WarnMsgs.size(), 0);

    SUBCASE("Parser - Expressions")
    {
        file.Content = "fn x() { var x = 1 + 2 * 3 / 4 % 5 - 6 << 7;\n"
                       "var z = a.b.c->d? + e;\n"
                       "var y = (a?.b.c->d?)? + e; }";

        Parser g(file, "EXPR");
        g.Parse();

        CHECK_EQ(file.ErrMsgs.size(), 0);
        CHECK_EQ(file.WarnMsgs.size(), 0);
    }
}

TEST_CASE("Parser - Function statement")
{

    File file;
    file.Content = "fn name() {}\n"
                   "fn name(a : i32) {}\n"
                   "fn name(a : i32, b : i32) : i32 {}\n"
                   "pub fn name(a : i32) const : i32 {}\n"
                   "pub fn name(a : i32) const : i32 {}\n"
                   "fn name(a : T) {}";
    Parser p(file, "FUNCTION");

    p.Parse();

    CHECK_EQ(file.ErrMsgs.size(), 0);
    CHECK_EQ(file.WarnMsgs.size(), 0);
}

TEST_CASE("Parser - Variant statement")
{

    File file;
    file.Content = "variant A { i32 }\n"
                   "variant B { i32, i64, u32 }\n"
                   "variant C { T }\n"
                   "variant D { T,E }\n"
                   "variant E { E }\n";
    Parser p(file, "VARIANT");

    p.Parse();

    CHECK_EQ(file.ErrMsgs.size(), 0);
    CHECK_EQ(file.WarnMsgs.size(), 0);
}

TEST_CASE("Parser - Enum statement")
{

    File file;
    file.Content = "enum A { A, B, C }\n"
                   "enum B { A = 5, B, C = 8 }\n"
                   "enum C<u32> { A, B, C }\n"
                   "enum D<(u32,u32)> { T = (0,0),E = (1,1) }\n";
    Parser p(file, "ENUM");

    p.Parse();

    CHECK_EQ(file.ErrMsgs.size(), 0);
    CHECK_EQ(file.WarnMsgs.size(), 0);
}

TEST_CASE("Parser - Expression statement")
{

    File file;
    file.Content = "fn main() {\n"
                   "    test();\n"
                   "    x = 54;\n"
                   "    x += 4 + 8;\n"
                   "    x = function();\n"
                   "    x.y.z %= a.b->c()();\n"
                   "    x.y->z |= 50;\n"
                   "    y = new S;\n"
                   "    z = new S{ x = 5, y = 8 };\n"
                   "}";
    Parser p(file, "EXPR_STMT");

    p.Parse();

    CHECK_EQ(file.ErrMsgs.size(), 0);
    CHECK_EQ(file.WarnMsgs.size(), 0);
}

TEST_CASE("Parser - Struct statement")
{

    File file;
    file.Content = "struct A { value : i32 = 5; }"
                   "\n"
                   "struct B\n"
                   "{\n"
                   "    pub const MIN : isize = 5;\n"
                   "    pub const MAX : isize = 5;\n"
                   "\n"
                   "    pub value : isize;\n"
                   "    pub default_value : isize = 5;\n"
                   "}";
    Parser p(file, "STRUCT");

    p.Parse();

    CHECK_EQ(file.ErrMsgs.size(), 0);
    CHECK_EQ(file.WarnMsgs.size(), 0);
}

TEST_CASE("Parser - Trait statement")
{

    File file;
    file.Content = "trait A\n"
                   "{\n"
                   "    fn static_method();\n"
                   "    fn object_method();\n"
                   "}\n"
                   "\n";
    Parser p(file, "TRAIT");

    p.Parse();

    CHECK_EQ(file.ErrMsgs.size(), 0);
    CHECK_EQ(file.WarnMsgs.size(), 0);
}

TEST_CASE("Parser - Implementation statement")
{

    File file;
    file.Content = "impl A\n"
                   "{\n"
                   "    fn print() {}\n"
                   "}\n"
                   "\n"
                   "impl B : C\n"
                   "{\n"
                   "    fn print() : i32 {}\n"
                   "}\n";
    Parser p(file, "IMPL");

    p.Parse();

    CHECK_EQ(file.ErrMsgs.size(), 0);
    CHECK_EQ(file.WarnMsgs.size(), 0);
}

TEST_CASE("Parser - Attributes")
{
    File file;
    file.Content = "#[a];\n"
                   "#[a,b,c];\n"
                   "#[a(b,c)];\n"
                   "static x : i32 = 5;";
    Parser p(file, "ATTR");

    p.Parse();

    CHECK_EQ(file.ErrMsgs.size(), 0);
    CHECK_EQ(file.WarnMsgs.size(), 0);
}
