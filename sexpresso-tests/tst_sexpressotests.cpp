#include <QtTest>

// add necessary includes here
#define SEXPRESSO_OPT_OUT_PIKESTYLE
#include "sexpresso/sexpresso.hpp"

class SexpressoTests : public QObject
{
    Q_OBJECT

public:
    SexpressoTests();
    ~SexpressoTests();

private slots:

    // tests from the original sexpresso
    void empty_string();
    void empty_sexp();
    void multiple_empty_sexp();
    void equality();
    void inequality();
    void string_literal();
    void hierarchy_query();
    void unacceptable_syntax();
    void argument_iterator();
    void string_escape_sequences();
    void escape_strings();
    void create_path();
    void add_expression();
    void toString_with_escape_value_in_string();
    // this test will fail with the changes made for
    // Common Lisp syntax
    //void toString_with_comma();

    // tests for common lisp functionality added to sexpresso::parse()
    void simple_string_positions();
    void nested_string_positions_1();
    void nested_string_positions_2();
    void nested_string_positions_multibyte();
    void vector_string_positions();
    void complex_number_string_positions();
    void scalar_type_string_positions();
    void quoted_sexp_string_positions();
    void macro_char_string_positions();
    void single_line_comments();
    void block_comments();
    void nested_block_comments();
    void sexp_sexp_kind();
    void sexp_atom_kind();
    void sexp_attribute_kind();
    void broken_sexp_missing_parens();
    void broken_sexp_too_many_parens();
    void broken_sexp_unterminated_string();
    void broken_sexp_unclosed_block_comment();
    void broken_sexp_unclosed_nested_block_comment();
};

SexpressoTests::SexpressoTests()
{

}

SexpressoTests::~SexpressoTests()
{

}

//----------------------------------------------------------------------------
// tests from sexpresso/test/sexpresso_test.cpp
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// empty_string()
//----------------------------------------------------------------------------
void SexpressoTests::empty_string()
{
    auto str = std::string{};
    QVERIFY(str.empty());

    auto err = std::string{};
    auto s = sexpresso::parse(str, err);

    QVERIFY(err.empty());
    QVERIFY(s.kind == sexpresso::SexpValueKind::SEXP);
    QVERIFY(s.value.str.empty());
    QVERIFY(s.value.sexp.empty());
    QVERIFY(s.childCount() == 0);
    QVERIFY(s.isNil());
}

//----------------------------------------------------------------------------
// empty_sexp()
//----------------------------------------------------------------------------
void SexpressoTests::empty_sexp()
{
    auto str = std::string{"()"};

    auto err = std::string{};
    auto s = sexpresso::parse(str, err);

    QVERIFY(err.empty());
    QVERIFY(s.kind == sexpresso::SexpValueKind::SEXP);
    QVERIFY(s.value.str.empty());
    QVERIFY(s.childCount() == 1);
    QVERIFY(s.childCount() == 1);
    QVERIFY(s.value.sexp[0].kind == sexpresso::SexpValueKind::SEXP);
    QVERIFY(s.value.sexp[0].value.sexp.empty());
    QVERIFY(s.toString() == "()");
}

//----------------------------------------------------------------------------
// multiple_empty_sexp()
//----------------------------------------------------------------------------
void SexpressoTests:: multiple_empty_sexp()
{
    auto str = std::string{"()\n() ()"};

    auto err = std::string{};
    auto s = sexpresso::parse(str, err);

    QVERIFY(err.empty());
    QVERIFY(s.childCount() == 3);

    for(auto&& sc : s.value.sexp) {
        QVERIFY(sc.isNil());
    }

    QVERIFY(s.toString() == "() () ()");
}

//----------------------------------------------------------------------------
// equality()
//----------------------------------------------------------------------------
void SexpressoTests::equality()
{
    auto str = std::string{"hi there (what a cool (little list) parser) (library)"};

    auto err = std::string{};
    auto s = sexpresso::parse(str, err);

    QVERIFY(err.empty());

    auto outer = sexpresso::Sexp{};
    outer.addChild("hi");
    outer.addChild("there");


    auto what = sexpresso::Sexp{};
    what.addChild("what");
    what.addChild("a");
    what.addChild("cool");

    auto little = sexpresso::Sexp{};
    little.addChild("little");
    little.addChild("list");
    what.addChild(std::move(little));

    what.addChild("parser");

    outer.addChild(std::move(what));

    auto libholder = sexpresso::Sexp{};
    libholder.addChild("library");

    outer.addChild(std::move(libholder));

    QVERIFY(s.equal(outer));
    QVERIFY(str == s.toString());
}

//----------------------------------------------------------------------------
// inequality()
//----------------------------------------------------------------------------
void SexpressoTests::inequality()
{
    auto err = std::string{};
    auto astr = std::string{"this (one is nothing)"};
    auto bstr = std::string{"like the (other)"};

    auto a = sexpresso::parse(astr, err);
    QVERIFY(err.empty());

    auto b = sexpresso::parse(bstr, err);
    QVERIFY(err.empty());

    QVERIFY(!a.equal(b));
}

//----------------------------------------------------------------------------
// string_literal
//----------------------------------------------------------------------------
void SexpressoTests::string_literal()
{
    auto err = std::string{};
    auto s = sexpresso::parse("\"hello world\" hehe", err);
    QVERIFY(s.value.sexp[0].value.str == "hello world");
}

//----------------------------------------------------------------------------
// hierarchy()
//----------------------------------------------------------------------------
void SexpressoTests::hierarchy_query()
{
    auto err = std::string{};
    auto s = sexpresso::parse("(myshit (a (name me) (age 2)) (b (name you) (age 1)))", err);
    QVERIFY(s.getChildByPath("myshit/a/name")->equal(sexpresso::parse("name me", err)));
    QVERIFY(s.getChildByPath("myshit/a/age")->equal(sexpresso::parse("age 2", err)));
    QVERIFY(s.getChildByPath("myshit/a")->equal(sexpresso::parse("a (name me) (age 2)", err)));

    QVERIFY(s.getChildByPath("myshit/b/name")->equal(sexpresso::parse("name you", err)));
    QVERIFY(s.getChildByPath("myshit/b/age")->equal(sexpresso::parse("age 1", err)));
    QVERIFY(s.getChildByPath("myshit/b")->equal(sexpresso::parse("b (name you) (age 1)", err)));

    QVERIFY(s.getChildByPath("this/does/not/even/exist/dummy") == nullptr);
}

//----------------------------------------------------------------------------
// unacceptable_syntax()
//----------------------------------------------------------------------------
void SexpressoTests::unacceptable_syntax()
{
    auto err = std::string{};

    sexpresso::parse("(((lol))", err);
    QVERIFY(!err.empty());

    sexpresso::parse("((rofl)))", err);
    QVERIFY(!err.empty());

    sexpresso::parse("(((\"i am gonna start a string but not close it))", err);
    QVERIFY(!err.empty());
}

//----------------------------------------------------------------------------
// argument_iterator()
//----------------------------------------------------------------------------
void SexpressoTests::argument_iterator()
{
    auto s = sexpresso::parse("(hi (myshit 1 2 3 \"helloo there mate\"; comment\n sup))");
    auto yup = sexpresso::Sexp{};
    for(auto&& arg : s.getChildByPath("hi/myshit")->arguments()) {
        yup.addChild(arg);
    }
    QVERIFY(yup.toString() == "1 2 3 \"helloo there mate\" sup");
}

//----------------------------------------------------------------------------
// string_escape_sequences()
// rjd: not sure about this one...
//----------------------------------------------------------------------------
void SexpressoTests::string_escape_sequences()
{
    auto err = std::string{};
    auto s = sexpresso::parse("\"hey I said \\\"hey\\\" yo.\\n\"", err);
    QVERIFY(err.empty());
    QVERIFY(s.getChild(0).getString().length() > 0);
    QVERIFY(s.getChild(0).getString() == "hey I said \\\"hey\\\" yo.\\n");
    QCOMPARE(QString::fromStdString(s.getChild(0).getString()), "hey I said \\\"hey\\\" yo.\\n");
}

//----------------------------------------------------------------------------
// escape_strings()
//----------------------------------------------------------------------------
void SexpressoTests::escape_strings()
{
    auto escaped = sexpresso::escape("\n \t \b");
    QVERIFY(escaped == "\\n \\t \\b");
}

//----------------------------------------------------------------------------
// create_path()
//----------------------------------------------------------------------------
void SexpressoTests::create_path()
{
    auto s1 = sexpresso::Sexp{};
    auto pth = std::string{"wow/this/is/cool"};
    auto c = s1.getChildByPath(pth);
    QVERIFY(c == nullptr);
    c = &(s1.createPath(pth));
    QVERIFY(s1.toString() == "(wow (this (is (cool))))");
    QVERIFY(c == (s1.getChildByPath(pth)));
}

//----------------------------------------------------------------------------
// add_expression()
//----------------------------------------------------------------------------
void SexpressoTests::add_expression()
{
    auto s = sexpresso::Sexp{};
    auto& p = s.createPath("oh/my/god");
    p.addExpression("(r 0) (g 0) (b 23)");
    auto b = s.getChildByPath("oh/my/god/b");
    QVERIFY(b != nullptr);
    QVERIFY(b->toString() == "b 23");
    QVERIFY(s.toString() == "(oh (my (god (r 0) (g 0) (b 23))))");
}

//----------------------------------------------------------------------------
// toString_with_escape_value_in_string()
//----------------------------------------------------------------------------
void SexpressoTests::toString_with_escape_value_in_string()
{
    auto s = sexpresso::Sexp{};
    s.addChildUnescaped("\t\n");
    QVERIFY(s.toString() == "\"\\t\\n\"");
}

//----------------------------------------------------------------------------
// toString_with_comma()
// This test will fail now sexpresso uses Common Lisp syntax
//----------------------------------------------------------------------------
/*
void SexpressoTests::toString_with_comma()
{
    auto str = std::string{"(a ((b ,(c d))))"};
    auto err = std::string{};
    auto s = sexpresso::parse(str, err);
    QVERIFY(s.toString() == "(a ((b , (c d))))");
}
*/

//----------------------------------------------------------------------------
// simple_string_positions() - does the parser return the correct
// start and end positions of parsed elements?
//----------------------------------------------------------------------------
void SexpressoTests::simple_string_positions()
{
    std::string str = "hello world";
    std::string err;
    auto s = sexpresso::parse(str,err);
    QVERIFY(err.length() == 0);

    auto s1 = s.getChild(0);
    QVERIFY(str.substr(static_cast<unsigned int>(s1.startpos),
                       static_cast<unsigned int>(s1.endpos - s1.startpos)) == "hello");

    auto s2 = s.getChild(1);
    QVERIFY(str.substr(static_cast<unsigned int>(s2.startpos),
                       static_cast<unsigned int>(s2.endpos - s2.startpos)) == "world");
}

//----------------------------------------------------------------------------
// nested_string_positions_1() - string positions on nested sexps
//----------------------------------------------------------------------------
void SexpressoTests::nested_string_positions_1()
{
    std::string str = "(defun foo (x) (print \"hello\"))";
    std::string err;
    auto s = sexpresso::parse(str,err);
    QVERIFY(err.length() == 0);

    auto s1 = s.getChild(0).getChild(3);
    QVERIFY(str.substr(static_cast<unsigned int>(s1.startpos),
                       static_cast<unsigned int>(s1.endpos - s1.startpos)) == "(print \"hello\")");
}

//----------------------------------------------------------------------------
// nested_string_positions_2() - string positions on strings in nested sexps
//----------------------------------------------------------------------------
void SexpressoTests::nested_string_positions_2()
{
    std::string str = "(defun foo (x) (print \"hello\n\"))";
    std::string err;
    auto s = sexpresso::parse(str,err);
    QVERIFY(err.length() == 0);

    auto s1 = s.getChild(0).getChild(3).getChild(1);
    QVERIFY(str.substr(static_cast<unsigned int>(s1.startpos),
                       static_cast<unsigned int>(s1.endpos - s1.startpos)) == "\"hello\n\"");
}

//----------------------------------------------------------------------------
// nested_string_positions_2() - string positions on strings in nested sexps
// with multibyte unicode characters
//----------------------------------------------------------------------------
void SexpressoTests::nested_string_positions_multibyte()
{
    std::string str = "(defun foo (x) (print \"Ⴔ Ⴕ Ⴖ Ⴗ Ⴘ Ⴙ Ⴚ Ⴛ Ⴜ Ⴝ Ⴞ Ⴟ Ⴠ Ⴡ Ⴢ\n\"))";
    std::string err;
    auto s = sexpresso::parse(str,err);
    QVERIFY(err.length() == 0);

    auto s1 = s.getChild(0).getChild(3).getChild(1);
    QVERIFY(str.substr(static_cast<unsigned int>(s1.startpos),
                       static_cast<unsigned int>(s1.endpos - s1.startpos)) == "\"Ⴔ Ⴕ Ⴖ Ⴗ Ⴘ Ⴙ Ⴚ Ⴛ Ⴜ Ⴝ Ⴞ Ⴟ Ⴠ Ⴡ Ⴢ\n\"");
}


//----------------------------------------------------------------------------
// vector_string_positions() - do the string positions handle sexps
// with attributes like vector
//----------------------------------------------------------------------------
void SexpressoTests::vector_string_positions()
{
    std::string str = "(defparameter *x* #(1 2 3 4 5))";
    std::string err;
    auto s = sexpresso::parse(str,err);
    QVERIFY(err.length() == 0);

    auto s1 = s.getChild(0).getChild(2);
    QVERIFY(str.substr(static_cast<unsigned int>(s1.startpos),
                       static_cast<unsigned int>(s1.endpos)) == "#(1 2 3 4 5)");
}

//----------------------------------------------------------------------------
// complex_number_string_positions - can the string position functionality
// handle complex numbers?
//----------------------------------------------------------------------------
void SexpressoTests::complex_number_string_positions()
{
    std::string str = "(defparameter *x* #c(1 2))";
    std::string err;
    auto s = sexpresso::parse(str,err);
    QVERIFY(err.length() == 0);

    auto s1 = s.getChild(0).getChild(2);
    QVERIFY(str.substr(static_cast<unsigned int>(s1.startpos),
                       static_cast<unsigned int>(s1.endpos)) == "#c(1 2)");
}

//----------------------------------------------------------------------------
// scalar_type_string_positions() can the string position functionality
// handle scalar types?
//----------------------------------------------------------------------------
void SexpressoTests::scalar_type_string_positions()
{
    {
        std::string binstr = "(defparameter *bin* #b111)";
        std::string binerr;
        auto binsexp = sexpresso::parse(binstr,binerr);
        QVERIFY(binerr.length() == 0);

        auto bins = binsexp.getChild(0).getChild(2);
        QVERIFY(binstr.substr(static_cast<unsigned int>(bins.startpos),
                           static_cast<unsigned int>(bins.endpos)) == "#b111");
    }

    {
        std::string hexstr = "(defparameter *hex* #xfff)";
        std::string hexerr;
        auto hexsexp = sexpresso::parse(hexstr,hexerr);
        QVERIFY(hexerr.length() == 0);

        auto hexs = hexsexp.getChild(0).getChild(2);
        QVERIFY(hexstr.substr(static_cast<unsigned int>(hexs.startpos),
                           static_cast<unsigned int>(hexs.endpos)) == "#xfff");
    }

    {
        std::string octstr = "(defparameter *oct* #o111)";
        std::string octerr;
        auto octsexp = sexpresso::parse(octstr,octerr);
        QVERIFY(octerr.length() == 0);

        auto octs = octsexp.getChild(0).getChild(2);
        QVERIFY(octstr.substr(static_cast<unsigned int>(octs.startpos),
                           static_cast<unsigned int>(octs.endpos)) == "#o111");
    }

    {
        std::string charstr = "(defparameter *char* #\\A)";
        std::string charerr;
        auto charsexp = sexpresso::parse(charstr,charerr);
        QVERIFY(charerr.length() == 0);

        auto chars = charsexp.getChild(0).getChild(2);
        QVERIFY(charstr.substr(static_cast<unsigned int>(chars.startpos),
                           static_cast<unsigned int>(chars.endpos)) == "#\\A");
    }
}

//----------------------------------------------------------------------------
// quoted_sexp_string_positions() - can we get string positions for quoted
// sexps?
//----------------------------------------------------------------------------
void SexpressoTests::quoted_sexp_string_positions()
{
    std::string str = "(defparameter *x* '(1 2 3 4))";
    std::string err;
    auto sexp = sexpresso::parse(str,err);
    QVERIFY(err.length() == 0);

    auto chars = sexp.getChild(0).getChild(2);
    QVERIFY(str.substr(static_cast<unsigned int>(chars.startpos),
                       static_cast<unsigned int>(chars.endpos)) == "'(1 2 3 4)");
}

//----------------------------------------------------------------------------
// macro_char_string_positions()
//----------------------------------------------------------------------------
void SexpressoTests::macro_char_string_positions()
{
    std::string str = "(defparameter *x* `(,@test))";
    std::string err;
    auto sexp = sexpresso::parse(str,err);
    QVERIFY(err.length() == 0);

    auto chars = sexp.getChild(0).getChild(2).getChild(0);
    QVERIFY(str.substr(static_cast<unsigned int>(chars.startpos),
                       static_cast<unsigned int>(chars.endpos)) == ",@test");
}

//----------------------------------------------------------------------------
// single_line_comments() - can the parser handle single line comments?
//----------------------------------------------------------------------------
void SexpressoTests::single_line_comments()
{
    std::string str = ";;testing\n(defparameter *x* 123)\n;;;comments";
    std::string err;
    auto sexp = sexpresso::parse(str,err);
    QVERIFY(err.length() == 0);

    auto test = sexp.getChild(0).toString(sexpresso::SexpressoPrintMode::TOP_LEVEL_PARENS);
    QVERIFY(test == "(defparameter *x* 123)");
}

//----------------------------------------------------------------------------
// block_comments()
//----------------------------------------------------------------------------
void SexpressoTests::block_comments()
{
    std::string str = "(defparameter *x* 123)\n#| block comment\n on multiple lines|#\n(foo bar baz)";
    std::string err;
    auto sexp = sexpresso::parse(str,err);
    QVERIFY(err.length() == 0);

    auto test = sexp.getChild(1).toString(sexpresso::SexpressoPrintMode::TOP_LEVEL_PARENS);
    QVERIFY(test == "(foo bar baz)");
}

//----------------------------------------------------------------------------
// nested_block_comments()
//----------------------------------------------------------------------------
void SexpressoTests::nested_block_comments()
{
    std::string str = "(defparameter *x* 123)\n#| block comment\n #| nested |#\n on multiple lines|#\n(foo bar baz)";
    std::string err;
    auto sexp = sexpresso::parse(str,err);
    QVERIFY(err.length() == 0);

    auto test = sexp.getChild(1).toString(sexpresso::SexpressoPrintMode::TOP_LEVEL_PARENS);
    QVERIFY(test == "(foo bar baz)");
}

//----------------------------------------------------------------------------
// sexp_sexp_kind() can the parser distinguish between ordinary sexps
// vectors and complex numbers?
//----------------------------------------------------------------------------
void SexpressoTests::sexp_sexp_kind()
{
    {
        std::string ordinarystr = "(foo bar baz)";
        std::string err;
        auto sexp = sexpresso::parse(ordinarystr,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).sexpkind == sexpresso::SexpSexpKind::NONE);
    }

    {
        std::string vectorstr = "#(foo bar baz)";
        std::string err;
        auto sexp = sexpresso::parse(vectorstr,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).sexpkind == sexpresso::SexpSexpKind::VECTOR);
    }
    {
        std::string complexstr = "#c(1 2)";
        std::string err;
        auto sexp = sexpresso::parse(complexstr,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).sexpkind == sexpresso::SexpSexpKind::COMPLEX);
    }
}

//----------------------------------------------------------------------------
// sexp_atom_kind() - can the parser correctly distinguish between different
// types of atoms
//----------------------------------------------------------------------------
void SexpressoTests::sexp_atom_kind()
{
    {
        std::string str = "()";
        std::string err;
        auto sexp = sexpresso::parse(str,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).atomkind == sexpresso::SexpAtomKind::NONE);
    }
    {
        std::string str = "foo";
        std::string err;
        auto sexp = sexpresso::parse(str,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).atomkind == sexpresso::SexpAtomKind::SYMBOL);
    }
    {
        std::string str = "\"foo\"";
        std::string err;
        auto sexp = sexpresso::parse(str,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).atomkind == sexpresso::SexpAtomKind::STRING);
    }
    {
        std::string str = "#\\A";
        std::string err;
        auto sexp = sexpresso::parse(str,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).atomkind == sexpresso::SexpAtomKind::CHAR);
    }
    {
        std::string str = "#b11";
        std::string err;
        auto sexp = sexpresso::parse(str,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).atomkind == sexpresso::SexpAtomKind::BINARY);
    }
    {
        std::string str = "#o11";
        std::string err;
        auto sexp = sexpresso::parse(str,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).atomkind == sexpresso::SexpAtomKind::OCTAL);
    }
    {
        std::string str = "#xfff";
        std::string err;
        auto sexp = sexpresso::parse(str,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).atomkind == sexpresso::SexpAtomKind::HEX);
    }
    {
        std::string str = "#p\"path/to/nowhere\"";
        std::string err;
        auto sexp = sexpresso::parse(str,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).atomkind == sexpresso::SexpAtomKind::PATHNAME);
    }
}

//----------------------------------------------------------------------------
// sexp_attribute_kind() - does the parser understand "attributes"
// quotes, macro characters etc?
//----------------------------------------------------------------------------
void SexpressoTests::sexp_attribute_kind()
{
    {
        std::string str = "'(1 2 3 4)";
        std::string err;
        auto sexp = sexpresso::parse(str,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).attributes.size() == 1);
        QVERIFY(sexp.getChild(0).attributes.back() == sexpresso::SexpAttributeKind::QUOTE);
    }

    {
        std::string str = "`(1 2 3 4)";
        std::string err;
        auto sexp = sexpresso::parse(str,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).attributes.size() == 1);
        QVERIFY(sexp.getChild(0).attributes.back() == sexpresso::SexpAttributeKind::BACKQUOTE);
    }
    {
        std::string str = "#'foo";
        std::string err;
        auto sexp = sexpresso::parse(str,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).attributes.size() == 1);
        QVERIFY(sexp.getChild(0).attributes.back() == sexpresso::SexpAttributeKind::FUNCQUOTE);
    }

    {
        std::string str = ",foo";
        std::string err;
        auto sexp = sexpresso::parse(str,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).attributes.size() == 1);
        QVERIFY(sexp.getChild(0).attributes.back() == sexpresso::SexpAttributeKind::COMMASPLICE);
    }
    {
        std::string str = ",@foo";
        std::string err;
        auto sexp = sexpresso::parse(str,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).attributes.size() == 1);
        QVERIFY(sexp.getChild(0).attributes.back() == sexpresso::SexpAttributeKind::ATSPLICE);
    }
    {
        std::string str = ",.foo";
        std::string err;
        auto sexp = sexpresso::parse(str,err);
        QVERIFY(err.length() == 0);
        QVERIFY(sexp.getChild(0).attributes.size() == 1);
        QVERIFY(sexp.getChild(0).attributes.back() == sexpresso::SexpAttributeKind::DOTSPLICE);
    }
}

//----------------------------------------------------------------------------
// Can the parser handle broken input with missing parens?
//----------------------------------------------------------------------------
void SexpressoTests::broken_sexp_missing_parens()
{
    std::string str = "(defun foo (x) (print";
    std::string err;
    auto sexp = sexpresso::parse(str,err);
    QVERIFY(err == "not enough s-expressions were closed by the end of parsing");
    QVERIFY(sexp.toString() == "(defun foo (x) (print :sexpresso-error))");
}

//----------------------------------------------------------------------------
// broken_sexp_too_many_parens() - can the parser handle broken input with
// too many parens?
//----------------------------------------------------------------------------
void SexpressoTests::broken_sexp_too_many_parens()
{
    std::string str = "(defun foo (x) (print)))";
    std::string err;
    auto sexp = sexpresso::parse(str,err);
    QVERIFY(err == "too many ')' characters detected, closing sexprs that don't exist, no good.");
    QVERIFY(sexp.toString() == "(defun foo (x) (print))");
}

//----------------------------------------------------------------------------
// broken_sexp_unterminated_string() - can the parser handle broken input
// with unterminated strings?
//----------------------------------------------------------------------------
void SexpressoTests::broken_sexp_unterminated_string()
{
    std::string str = "(defun foo (x) (print \"hello";
    std::string err;
    auto sexp = sexpresso::parse(str,err);
    QVERIFY(err == "Unterminated string literal");
    QVERIFY(sexp.toString() == "(defun foo (x) (print \"hello\" :sexpresso-error))");
}

//----------------------------------------------------------------------------
// broken_sexp_unclosed_block_comment()
//----------------------------------------------------------------------------
void SexpressoTests::broken_sexp_unclosed_block_comment()
{
    std::string str = "(defun foo (x) (print \"hello\")) #| unclosed block comment";
    std::string err;
    auto sexp = sexpresso::parse(str,err);
    QVERIFY(err == "Unclosed block comment");
    QVERIFY(sexp.toString() == "(defun foo (x) (print \"hello\")) :sexpresso-error");
}

//----------------------------------------------------------------------------
// broken_sexp_unclosed_nested_block_comment()
//----------------------------------------------------------------------------
void SexpressoTests::broken_sexp_unclosed_nested_block_comment()
{
    std::string str = "(defun foo (x) (print \"hello\")) #| unclosed #| nested |# block comment";
    std::string err;
    auto sexp = sexpresso::parse(str,err);
    QVERIFY(err == "Unclosed block comment");
    QVERIFY(sexp.toString() == "(defun foo (x) (print \"hello\")) :sexpresso-error");
}


QTEST_APPLESS_MAIN(SexpressoTests)

#include "tst_sexpressotests.moc"
