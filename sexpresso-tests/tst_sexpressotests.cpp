#include <QtTest>

// add necessary includes here

class SexpressoTests : public QObject
{
    Q_OBJECT

public:
    SexpressoTests();
    ~SexpressoTests();

private slots:
    void test_case1();

};

SexpressoTests::SexpressoTests()
{

}

SexpressoTests::~SexpressoTests()
{

}

void SexpressoTests::test_case1()
{

}

QTEST_APPLESS_MAIN(SexpressoTests)

#include "tst_sexpressotests.moc"
