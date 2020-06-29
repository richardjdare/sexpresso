#ifndef SEXPRESSO_H
#define SEXPRESSO_H
// Author: Isak Andersson 2016 bitpuffin dot com

#include <vector>
#include <string>
#include <cstdint>

namespace sexpresso {
    enum class SexpValueKind : uint8_t { SEXP, ATOM };
    enum class SexpSexpKind : uint8_t { NONE, VECTOR, COMPLEX };
    enum class SexpAtomKind : uint8_t { NONE, SYMBOL, STRING, CHAR, BINARY, OCTAL, HEX };
    enum class SexpAttributeKind : uint8_t { QUOTE, BACKQUOTE, FUNCQUOTE, COMMASPLICE, ATSPLICE, DOTSPLICE };
    enum class SexpressoPrintMode : uint8_t { NO_TOPLEVEL_PARENS, TOP_LEVEL_PARENS };
	struct SexpArgumentIterator;

	struct Sexp {
		Sexp();
        Sexp(int64_t startpos, int64_t endpos = 0);
		Sexp(std::string const& strval);
        Sexp(std::string const& strval, SexpAtomKind atomkind);
        Sexp(std::string const& strval,int64_t startpos, int64_t endpos = 0);
        Sexp(std::string const& strval, int64_t startpos, int64_t endpos, SexpAtomKind atomkind);
		Sexp(std::vector<Sexp> const& sexpval);
        Sexp(std::vector<Sexp> const& sexpval, int64_t startpos, int64_t endpos = 0);
		SexpValueKind kind;
        SexpSexpKind sexpkind;
        SexpAtomKind atomkind;
        std::vector<SexpAttributeKind> attributes;
        int64_t startpos;
        int64_t endpos;
        struct { std::vector<Sexp> sexp; std::string str; int64_t startpos; int64_t endpos = 0;} value;
		auto addChild(Sexp sexp) -> void;
		auto addChild(std::string str) -> void;
		auto addChildUnescaped(std::string str) -> void;
        auto addChildUnescaped(std::string str, int64_t startpos, int64_t endpos = 0) -> void;
        auto addChildUnescaped(std::string str, SexpAtomKind atomkind, int64_t startpos, int64_t endpos = 0) -> void;
		auto addExpression(std::string const& str) -> void;
		auto childCount() const -> size_t;
		auto getChild(size_t idx) -> Sexp&; // Call only if Sexp is a Sexp
		auto getString() -> std::string&;
		auto getChildByPath(std::string const& path) -> Sexp*; // unsafe! careful to not have the result pointer outlive the scope of the Sexp object
		auto createPath(std::vector<std::string> const& path) -> Sexp&;
		auto createPath(std::string const& path) -> Sexp&;
        auto toString(SexpressoPrintMode printmode = SexpressoPrintMode::NO_TOPLEVEL_PARENS) const -> std::string;
		auto isString() const -> bool;
		auto isSexp() const -> bool;
		auto isNil() const -> bool;
		auto equal(Sexp const& other) const -> bool;
		auto arguments() -> SexpArgumentIterator;
		static auto unescaped(std::string strval) -> Sexp;
        static auto unescaped(std::string strval, int64_t startpos, int64_t endpos = 0) -> Sexp;
        static auto unescaped(std::string strval, SexpAtomKind atomkind, int64_t startpos, int64_t endpos = 0) -> Sexp;
	};

//	auto parse(std::string const& str, std::string& err) -> Sexp;
//	auto parse(std::string const& str) -> Sexp;
    auto parse(std::string const& str, std::string& err) -> Sexp;
    auto parse(std::string const& str, std::string& err, std::string& errsymbol) -> Sexp;
	auto escape(std::string const& str) -> std::string;

	struct SexpArgumentIterator {
		SexpArgumentIterator(Sexp& sexp);
		Sexp& sexp;

		using iterator = std::vector<Sexp>::iterator;
		using const_iterator = std::vector<Sexp>::const_iterator;

		auto begin() -> iterator;
		auto end() -> iterator;
		auto begin() const -> const_iterator;
		auto end() const -> const_iterator;
		auto size() const -> size_t;
		auto empty() const -> bool;
	};
};
#endif
