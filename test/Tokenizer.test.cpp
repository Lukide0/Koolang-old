#include "ast/Tokenizer.h"
#include "ast/Token.h"
#include "test.h"

using ast::Tokenizer;
using ast::TokenTag;

static File gTestFile;

TEST_CASE("Tokenizer - END_OF_FILE")
{
    gTestFile.Content = "";
    Tokenizer tk(gTestFile);

    auto t = tk.Tokenize();

    CHECK_EQ(t.NextTok(), TokenTag::END_OF_FILE);
}

TEST_CASE("Tokenizer - DOC_COMMENT")
{

    gTestFile.Content = "/**\n"
                        "\n"
                        "\n This is DOC_COMMENT\n"
                        "*/struct";

    Tokenizer tk(gTestFile);

    auto t = tk.Tokenize();

    CHECK_EQ(t.NextTok(), TokenTag::DOC_COMMENT);
    CHECK_EQ(t.NextTok(), TokenTag::K_STRUCT);
    CHECK_EQ(t.NextTok(), TokenTag::END_OF_FILE);
}

TEST_CASE("Tokenizer - PAREN_L, PAREN_R")
{
    gTestFile.Content = "(((\n"
                        ")))\n"
                        "// (( )))";

    Tokenizer tk(gTestFile);

    auto t = tk.Tokenize();

    CHECK_EQ(t.NextTok(), TokenTag::PAREN_L);
    CHECK_EQ(t.NextTok(), TokenTag::PAREN_L);
    CHECK_EQ(t.NextTok(), TokenTag::PAREN_L);

    CHECK_EQ(t.NextTok(), TokenTag::PAREN_R);
    CHECK_EQ(t.NextTok(), TokenTag::PAREN_R);
    CHECK_EQ(t.NextTok(), TokenTag::PAREN_R);

    CHECK_EQ(t.NextTok(), TokenTag::END_OF_FILE);
}

TEST_CASE("Tokenizer - LS, GT")
{

    gTestFile.Content = "<<<\n"
                        ">>>";
    Tokenizer tk(gTestFile);

    auto t = tk.Tokenize();

    CHECK_EQ(t.NextTok(), TokenTag::LS);
    CHECK_EQ(t.NextTok(), TokenTag::LS);
    CHECK_EQ(t.NextTok(), TokenTag::LS);

    CHECK_EQ(t.NextTok(), TokenTag::GT);
    CHECK_EQ(t.NextTok(), TokenTag::GT);
    CHECK_EQ(t.NextTok(), TokenTag::GT);

    CHECK_EQ(t.NextTok(), TokenTag::END_OF_FILE);
}

TEST_CASE("Tokenizer - SQUARE_L, SQUARE_R")
{

    gTestFile.Content = "[[[\n"
                        "]]]";
    Tokenizer tk(gTestFile);

    auto t = tk.Tokenize();

    CHECK_EQ(t.NextTok(), TokenTag::SQUARE_L);
    CHECK_EQ(t.NextTok(), TokenTag::SQUARE_L);
    CHECK_EQ(t.NextTok(), TokenTag::SQUARE_L);

    CHECK_EQ(t.NextTok(), TokenTag::SQUARE_R);
    CHECK_EQ(t.NextTok(), TokenTag::SQUARE_R);
    CHECK_EQ(t.NextTok(), TokenTag::SQUARE_R);

    CHECK_EQ(t.NextTok(), TokenTag::END_OF_FILE);
}

TEST_CASE("Tokenizer - CURLY_L, CURLY_R")
{

    gTestFile.Content = "{{{\n"
                        "}}}";
    Tokenizer tk(gTestFile);

    auto t = tk.Tokenize();

    CHECK_EQ(t.NextTok(), TokenTag::CURLY_L);
    CHECK_EQ(t.NextTok(), TokenTag::CURLY_L);
    CHECK_EQ(t.NextTok(), TokenTag::CURLY_L);

    CHECK_EQ(t.NextTok(), TokenTag::CURLY_R);
    CHECK_EQ(t.NextTok(), TokenTag::CURLY_R);
    CHECK_EQ(t.NextTok(), TokenTag::CURLY_R);

    CHECK_EQ(t.NextTok(), TokenTag::END_OF_FILE);
}

TEST_CASE("Tokenizer - STRING_LIT")
{

    gTestFile.Content = "\"abcdef\"\n"
                        "\"abcde\\\"\"\n"
                        "`123456`\n"
                        "`\n"
                        "abcdef\n"
                        "`";
    Tokenizer tk(gTestFile);

    auto t = tk.Tokenize();

    CHECK_EQ(t.NextTok(), TokenTag::STRING_LIT);
    CHECK_EQ(t.NextTok(), TokenTag::STRING_LIT);
    CHECK_EQ(t.NextTok(), TokenTag::STRING_LIT);
    CHECK_EQ(t.NextTok(), TokenTag::STRING_LIT);

    CHECK_EQ(t.NextTok(), TokenTag::END_OF_FILE);
}

TEST_CASE("Tokenizer - CHAR_LIT")
{

    gTestFile.Content = "'a'\n"
                        "'\\\\'\n"
                        "'\\n'";
    Tokenizer tk(gTestFile);

    auto t = tk.Tokenize();

    CHECK_EQ(t.NextTok(), TokenTag::CHAR_LIT);
    CHECK_EQ(t.NextTok(), TokenTag::CHAR_LIT);
    CHECK_EQ(t.NextTok(), TokenTag::CHAR_LIT);

    CHECK_EQ(t.NextTok(), TokenTag::END_OF_FILE);
}

TEST_CASE("Tokenizer - NUMBER_LIT")
{

    gTestFile.Content = "12345\n"
                        "1_2__3__4___5\n"
                        "0b001\n"
                        "0xFF\n"
                        "0o55";
    Tokenizer tk(gTestFile);

    auto t = tk.Tokenize();

    CHECK_EQ(t.NextTok(), TokenTag::NUMBER_LIT);
    CHECK_EQ(t.NextTok(), TokenTag::NUMBER_LIT);
    CHECK_EQ(t.NextTok(), TokenTag::NUMBER_LIT);
    CHECK_EQ(t.NextTok(), TokenTag::NUMBER_LIT);
    CHECK_EQ(t.NextTok(), TokenTag::NUMBER_LIT);

    CHECK_EQ(t.NextTok(), TokenTag::END_OF_FILE);
}

TEST_CASE("Tokenizer - FLOAT_LIT")
{

    gTestFile.Content = "12345.0\n"
                        "1.\n"
                        "5._\n"
                        "5.0_0__0\n"
                        "5.555.555\n"
                        "5.0";
    Tokenizer tk(gTestFile);

    auto t = tk.Tokenize();

    CHECK_EQ(t.NextTok(), TokenTag::FLOAT_LIT);

    CHECK_EQ(t.NextTok(), TokenTag::NUMBER_LIT);
    CHECK_EQ(t.NextTok(), TokenTag::DOT);

    CHECK_EQ(t.NextTok(), TokenTag::NUMBER_LIT);
    CHECK_EQ(t.NextTok(), TokenTag::DOT);
    CHECK_EQ(t.NextTok(), TokenTag::UNDERSCORE);

    CHECK_EQ(t.NextTok(), TokenTag::FLOAT_LIT);

    CHECK_EQ(t.NextTok(), TokenTag::FLOAT_LIT);
    CHECK_EQ(t.NextTok(), TokenTag::DOT);
    CHECK_EQ(t.NextTok(), TokenTag::NUMBER_LIT);

    CHECK_EQ(t.NextTok(), TokenTag::FLOAT_LIT);

    CHECK_EQ(t.NextTok(), TokenTag::END_OF_FILE);
}

TEST_CASE("Tokenizer - SYMBOLS")
{

    gTestFile.Content = "_\n"
                        ";\n"
                        "#\n"
                        "->\n"
                        ".\n"
                        ":\n"
                        "::\n"
                        "+\n"
                        "+=\n"
                        "-\n"
                        "-=\n"
                        "*\n"
                        "*=\n"
                        "%\n"
                        "%=\n"
                        "/\n"
                        "/=\n"
                        "?\n"
                        "??\n"
                        "!\n"
                        "~\n"
                        "&\n"
                        "&&\n"
                        "&=\n"
                        "|\n"
                        "||\n"
                        "|=\n"
                        "^\n"
                        "^=\n"
                        "=\n"
                        "==\n"
                        "!=\n";
    Tokenizer tk(gTestFile);

    auto t = tk.Tokenize();

    CHECK_EQ(t.NextTok(), TokenTag::UNDERSCORE);
    CHECK_EQ(t.NextTok(), TokenTag::SEMI);
    CHECK_EQ(t.NextTok(), TokenTag::HASHTAG);
    CHECK_EQ(t.NextTok(), TokenTag::ARROW);
    CHECK_EQ(t.NextTok(), TokenTag::DOT);
    CHECK_EQ(t.NextTok(), TokenTag::COLON);
    CHECK_EQ(t.NextTok(), TokenTag::COLON2);
    CHECK_EQ(t.NextTok(), TokenTag::ADD);
    CHECK_EQ(t.NextTok(), TokenTag::ADD_EQ);
    CHECK_EQ(t.NextTok(), TokenTag::MINUS);
    CHECK_EQ(t.NextTok(), TokenTag::MINUS_EQ);
    CHECK_EQ(t.NextTok(), TokenTag::STAR);
    CHECK_EQ(t.NextTok(), TokenTag::STAR_EQ);
    CHECK_EQ(t.NextTok(), TokenTag::MOD);
    CHECK_EQ(t.NextTok(), TokenTag::MOD_EQ);
    CHECK_EQ(t.NextTok(), TokenTag::DIV);
    CHECK_EQ(t.NextTok(), TokenTag::DIV_EQ);
    CHECK_EQ(t.NextTok(), TokenTag::QUESTION);
    CHECK_EQ(t.NextTok(), TokenTag::QUESTION2);
    CHECK_EQ(t.NextTok(), TokenTag::BANG);
    CHECK_EQ(t.NextTok(), TokenTag::TILDE);
    CHECK_EQ(t.NextTok(), TokenTag::AND);
    CHECK_EQ(t.NextTok(), TokenTag::AND_AND);
    CHECK_EQ(t.NextTok(), TokenTag::AND_EQ);
    CHECK_EQ(t.NextTok(), TokenTag::OR);
    CHECK_EQ(t.NextTok(), TokenTag::OR_OR);
    CHECK_EQ(t.NextTok(), TokenTag::OR_EQ);
    CHECK_EQ(t.NextTok(), TokenTag::CARET);
    CHECK_EQ(t.NextTok(), TokenTag::CARET_EQ);
    CHECK_EQ(t.NextTok(), TokenTag::EQ);
    CHECK_EQ(t.NextTok(), TokenTag::EQ_EQ);
    CHECK_EQ(t.NextTok(), TokenTag::NOT_EQ);

    CHECK_EQ(t.NextTok(), TokenTag::END_OF_FILE);
}

TEST_CASE("Tokenizer - EatTokAny")
{
    gTestFile.Content = "a 0 b 1";
    Tokenizer tk(gTestFile);
    auto t = tk.Tokenize();

    CHECK_EQ(t.EatTokAny<2>({ TokenTag::ADD, TokenTag::K_VAR }), NULL_INDEX);
    CHECK_NE(t.EatTokAny<2>({ TokenTag::IDENT, TokenTag::NUMBER_LIT }), NULL_INDEX);
    CHECK_NE(t.EatTokAny<2>({ TokenTag::IDENT, TokenTag::NUMBER_LIT }), NULL_INDEX);
    CHECK_NE(t.EatTokAny<2>({ TokenTag::IDENT, TokenTag::NUMBER_LIT }), NULL_INDEX);
    CHECK_NE(t.EatTokAny<2>({ TokenTag::IDENT, TokenTag::NUMBER_LIT }), NULL_INDEX);
    CHECK_EQ(t.NextTok(), TokenTag::END_OF_FILE);
}
