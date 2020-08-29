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
  //  void toString_with_comma();
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
// not sure I understand the original test as the string is added unescaped,
// and therefore should look just like the input string, not the one in the
// QVERIFY?
//----------------------------------------------------------------------------
void SexpressoTests::toString_with_escape_value_in_string()
{
    auto s = sexpresso::Sexp{};
    s.addChildUnescaped("\t\n");
    QVERIFY(s.toString() == "\"\\t\\n\"");
}

//----------------------------------------------------------------------------
// toString_with_comma()
// rjd: This test will fail with common lisp syntax
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

QTEST_APPLESS_MAIN(SexpressoTests)

#include "tst_sexpressotests.moc"
