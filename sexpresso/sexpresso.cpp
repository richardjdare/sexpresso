// Author: Isak Andersson 2016 bitpuffin dot com

#ifndef SEXPRESSO_OPT_OUT_PIKESTYLE
#include <vector>
#include <string>
#include <cstdint>
#endif
#include "sexpresso.hpp"

#include <cctype>
#include <stack>
#include <algorithm>
#include <sstream>
#include <array>
#include <iostream>

namespace sexpresso {
    SexpAttributes::SexpAttributes()
        : kind(SexpValueKind::SEXP),
        sexpkind(SexpSexpKind::NONE),
        atomkind(SexpAtomKind::NONE),
        quotekind(SexpQuoteKind::NONE),
        macrokind(SexpMacroSyntaxKind::NONE){}

	Sexp::Sexp() {
        this->attributes.kind = SexpValueKind::SEXP;
        this->attributes.atomkind = SexpAtomKind::NONE;
        this->attributes.quotekind = SexpQuoteKind::NONE;
        this->attributes.sexpkind = SexpSexpKind::NONE;
	}
    Sexp::Sexp(int64_t startpos, int64_t endpos){
        this->attributes.kind = SexpValueKind::SEXP;
        this->attributes.atomkind = SexpAtomKind::NONE;
        this->attributes.quotekind = SexpQuoteKind::NONE;
        this->attributes.sexpkind = SexpSexpKind::NONE;
        this->startpos = startpos;
        this->value.startpos = 0;
        this->endpos = endpos;
        this->value.endpos = 0;
    }
	Sexp::Sexp(std::string const& strval) {
        this->attributes.kind = SexpValueKind::ATOM;
        this->attributes.atomkind = SexpAtomKind::SYMBOL;
        this->attributes.quotekind = SexpQuoteKind::NONE;
        this->attributes.sexpkind = SexpSexpKind::NONE;
		this->value.str = escape(strval);
	}
    Sexp::Sexp(std::string const& strval, SexpAtomKind atomkind) {
        this->attributes.kind = SexpValueKind::ATOM;
        this->attributes.quotekind = SexpQuoteKind::NONE;
        this->attributes.sexpkind = SexpSexpKind::NONE;
        this->attributes.atomkind = atomkind;
        this->value.str = escape(strval);
    }
    Sexp::Sexp(std::string const& strval, int64_t startpos, int64_t endpos) {
        this->attributes.kind = SexpValueKind::ATOM;
        this->attributes.atomkind = SexpAtomKind::SYMBOL;
        this->attributes.quotekind = SexpQuoteKind::NONE;
        this->attributes.sexpkind = SexpSexpKind::NONE;
        this->value.str = escape(strval);
        this->startpos = startpos;
        this->value.startpos = 0;
        this->endpos = endpos;
        this->value.endpos = 0;
    }
    Sexp::Sexp(std::string const& strval, int64_t startpos, int64_t endpos, SexpAtomKind atomkind) {
        this->attributes.kind = SexpValueKind::ATOM;
        this->attributes.quotekind = SexpQuoteKind::NONE;
        this->attributes.sexpkind = SexpSexpKind::NONE;
        this->attributes.atomkind = atomkind;
        this->value.str = escape(strval);
        this->startpos = startpos;
        this->value.startpos = 0;
        this->endpos = endpos;
        this->value.endpos = 0;
    }
	Sexp::Sexp(std::vector<Sexp> const& sexpval) {
        this->attributes.kind = SexpValueKind::SEXP;
        this->attributes.atomkind = SexpAtomKind::NONE;
        this->attributes.quotekind = SexpQuoteKind::NONE;
        this->attributes.sexpkind = SexpSexpKind::NONE;
		this->value.sexp = sexpval;
	}
    Sexp::Sexp(std::vector<Sexp> const& sexpval, int64_t startpos, int64_t endpos) {
        this->attributes.kind = SexpValueKind::SEXP;
        this->attributes.atomkind = SexpAtomKind::NONE;
        this->attributes.quotekind = SexpQuoteKind::NONE;
        this->attributes.sexpkind = SexpSexpKind::NONE;
        this->value.sexp = sexpval;
        this->startpos = startpos;
        this->value.startpos = 0;
        this->endpos = endpos;
        this->value.endpos = 0;
    }
	auto Sexp::addChild(Sexp sexp) -> void {
        if(this->attributes.kind == SexpValueKind::ATOM) {
            this->attributes.kind = SexpValueKind::SEXP;
            this->value.sexp.push_back(Sexp{std::move(this->value.str), this->startpos, this->endpos});
		}
		this->value.sexp.push_back(std::move(sexp));
	}

	auto Sexp::addChild(std::string str) -> void {
        this->addChild(Sexp{std::move(str), this->startpos});
	}

	auto Sexp::addChildUnescaped(std::string str) -> void {
		this->addChild(Sexp::unescaped(std::move(str)));
	}

    auto Sexp::addChildUnescaped(std::string str, int64_t startpos, int64_t endpos) -> void {
        this->addChild(Sexp::unescaped(std::move(str), startpos, endpos));
    }
    auto Sexp::addChildUnescaped(std::string str, SexpAtomKind atomkind, int64_t startpos, int64_t endpos) -> void {
        this->addChild(Sexp::unescaped(std::move(str), atomkind, startpos, endpos));
    }
	auto Sexp::addExpression(std::string const& str) -> void {
		auto err = std::string{};
		auto sexp = parse(str, err);
		if(!err.empty()) return;
		for(auto&& c : sexp.value.sexp) this->addChild(std::move(c));
	}
	auto Sexp::childCount() const -> size_t {
        switch(this->attributes.kind) {
		case SexpValueKind::SEXP:
			return this->value.sexp.size();
        case SexpValueKind::ATOM:
			return 1;
		}
	}

	static auto splitPathString(std::string const& path) -> std::vector<std::string> {
		auto paths = std::vector<std::string>{};
		if(path.size() == 0) return paths;
		auto start = path.begin();
		for(auto i = path.begin()+1; i != path.end(); ++i) {
			if(*i == '/') {
				paths.push_back(std::string{start, i});
				start = i + 1;
			}
		}
		paths.push_back(std::string{start, path.end()});
		return std::move(paths);
	}

	auto Sexp::getChildByPath(std::string const& path) -> Sexp* {
        if(this->attributes.kind == SexpValueKind::ATOM) return nullptr;

		auto paths = splitPathString(path);

		auto* cur = this;
		for(auto i = paths.begin(); i != paths.end();) {
			auto start = i;
			for(auto& child : cur->value.sexp) {
				auto brk = false;
                switch(child.attributes.kind) {
                case SexpValueKind::ATOM:
					if(i == paths.end() - 1 && child.value.str == *i) return &child;
					else continue;
				case SexpValueKind::SEXP:
					if(child.value.sexp.size() == 0) continue;
					auto& fst = child.value.sexp[0];
                    switch(fst.attributes.kind) {
                    case SexpValueKind::ATOM:
						if(fst.value.str == *i) {
							cur = &child;
							++i;
							brk = true;
						}
						break;
					case SexpValueKind::SEXP: continue;
					}
				}
				if(brk) break;
			}
			if(i == start) return nullptr;
			if(i == paths.end()) return cur;
		}
		return nullptr;
	}

	static auto findChild(Sexp& sexp, std::string name) -> Sexp* {
		auto findPred = [&name](Sexp& s) {
            switch(s.attributes.kind) {
			case SexpValueKind::SEXP: {
				if(s.childCount() == 0) return false;
				auto& hd = s.getChild(0);
                switch(hd.attributes.kind) {
				case SexpValueKind::SEXP:
					return false;
                case SexpValueKind::ATOM:
					return hd.getString() == name;
				}
			}
            case SexpValueKind::ATOM:
				return s.getString() == name;
			}
		};
		auto loc = std::find_if(sexp.value.sexp.begin(), sexp.value.sexp.end(), findPred);
		if(loc == sexp.value.sexp.end()) return nullptr;
		else return &(*loc);
	}
	
	auto Sexp::createPath(std::vector<std::string> const& path) -> Sexp& {
		auto el = this;
		auto nxt = el;
		auto pc = path.begin();
		for(; pc != path.end(); ++pc) {
			nxt = findChild(*el, *pc);
			if(nxt == nullptr) break;
			else el = nxt;
		}
		for(; pc != path.end(); ++pc) {
			el->addChild(Sexp{std::vector<Sexp>{Sexp{*pc}}});
			el = &(el->getChild(el->childCount()-1));
		}
		return *el;
	}

	auto Sexp::createPath(std::string const& path) -> Sexp& {
		return this->createPath(splitPathString(path));
	}

	auto Sexp::getChild(size_t idx) -> Sexp& {
		return this->value.sexp[idx];
	}

	auto Sexp::getString() -> std::string& {
		return this->value.str;
	}

	static const std::array<char, 11> escape_chars = { '\'', '"',  '?', '\\',  'a',  'b',  'f',  'n',  'r',  't',  'v' };
	static const std::array<char, 11> escape_vals  = { '\'', '"', '\?', '\\', '\a', '\b', '\f', '\n', '\r', '\t', '\v' };
	
	static auto isEscapeValue(char c) -> bool {
		return std::find(escape_vals.begin(), escape_vals.end(), c) != escape_vals.end();
	}

	static auto countEscapeValues(std::string const& str) -> size_t {
		return std::count_if(str.begin(), str.end(), isEscapeValue);
	}

	static auto stringValToString(std::string const& s) -> std::string {
		if(s.size() == 0) return std::string{"\"\""};
		if((std::find(s.begin(), s.end(), ' ') == s.end()) && countEscapeValues(s) == 0) return s;
		return ('"' + escape(s) + '"');
	}

    static auto stringSymbolToString(std::string const& s) -> std::string {
        return s;
    }

    static auto stringScalarToString(std::string const& s, SexpAtomKind atomkind) -> std::string{
        switch(atomkind){
            case SexpAtomKind::CHAR:
                return ("#\\"+s);
            case SexpAtomKind::HEX:
                return ("#x"+s);
            case SexpAtomKind::OCTAL:
                return ("#o"+s);
            case SexpAtomKind::BINARY:
                return ("#b"+s);
            default:
                return ("");
        }
    }

    static auto stringStringToString(std::string const& s) -> std::string {
        if(s.size() == 0) return std::string{"\"\""};
        return ('"' + escape(s) + '"');
    }

    static auto toStringImpl(Sexp const& sexp, std::ostringstream& ostream) -> void {
        switch(sexp.attributes.quotekind){
            case SexpQuoteKind::BACKQUOTE:
                ostream << "`";
                break;
            case SexpQuoteKind::SINGLEQUOTE:
                ostream << "'";
                break;
            case SexpQuoteKind::FUNCQUOTE:
                ostream << "#'";
                break;
            default: break;
        }
        switch(sexp.attributes.kind) {
        case SexpValueKind::ATOM:
            switch(sexp.attributes.atomkind){
                case SexpAtomKind::STRING:
                    ostream << stringStringToString(sexp.value.str);
                break;
                case SexpAtomKind::CHAR:
                case SexpAtomKind::HEX:
                case SexpAtomKind::OCTAL:
                case SexpAtomKind::BINARY:
                    ostream << stringScalarToString(sexp.value.str,sexp.attributes.atomkind);
                break;
                default:
                ostream << stringSymbolToString(sexp.value.str);
            }
			break;
		case SexpValueKind::SEXP:
            switch(sexp.attributes.sexpkind){
                case SexpSexpKind::VECTOR:
                    ostream << "#";
                break;
                case SexpSexpKind::COMPLEX:
                    ostream << "#c";
                break;

                default:break;
            }
			switch(sexp.value.sexp.size()) {
			case 0:
				ostream << "()";
				break;
			case 1:
				ostream << '(';
				toStringImpl(sexp.value.sexp[0], ostream);
				ostream <<  ')';
				break;
			default:
				ostream << '(';
				for(auto i = sexp.value.sexp.begin(); i != sexp.value.sexp.end(); ++i) {
					toStringImpl(*i, ostream);
					if(i != sexp.value.sexp.end()-1) ostream << ' ';
				}
				ostream << ')';
			}
		}
	}

	auto Sexp::toString() const -> std::string {
		auto ostream = std::ostringstream{};

		// outer sexp does not get surrounded by ()

        switch(this->attributes.quotekind){
            case SexpQuoteKind::BACKQUOTE:
                ostream << "`";
                break;
            case SexpQuoteKind::SINGLEQUOTE:
                ostream << "'";
                break;
            case SexpQuoteKind::FUNCQUOTE:
                ostream << "#'";
                break;
            default: break;
        }

        switch(this->attributes.kind) {
        case SexpValueKind::ATOM:
            switch(this->attributes.atomkind){
                case SexpAtomKind::STRING:
                    ostream << stringValToString(this->value.str);
                break;
            case SexpAtomKind::CHAR:
            case SexpAtomKind::HEX:
            case SexpAtomKind::OCTAL:
            case SexpAtomKind::BINARY:
                ostream << stringScalarToString(this->value.str,this->attributes.atomkind);
            break;
            default:
                ostream << stringValToString(this->value.str);
                break;
            }
            break;
		case SexpValueKind::SEXP:
			for(auto i = this->value.sexp.begin(); i != this->value.sexp.end(); ++i) {
				toStringImpl(*i, ostream);
				if(i != this->value.sexp.end()-1) ostream << ' ';
			}
		}
		return ostream.str();
	}

	auto Sexp::isString() const -> bool {
        return this->attributes.kind == SexpValueKind::ATOM;
	}

	auto Sexp::isSexp() const -> bool {
        return this->attributes.kind == SexpValueKind::SEXP;
	}

	auto Sexp::isNil() const -> bool {
        return this->attributes.kind == SexpValueKind::SEXP && this->childCount() == 0;
	}

	static auto childrenEqual(std::vector<Sexp> const& a, std::vector<Sexp> const& b) -> bool {
		if(a.size() != b.size()) return false;

		for(auto i = 0; i < a.size(); ++i) {
			if(!a[i].equal(b[i])) return false;
		}
		return true;
	}
	
	auto Sexp::equal(Sexp const& other) const -> bool {
        if(this->attributes.kind != other.attributes.kind) return false;
        switch(this->attributes.kind) {
		case SexpValueKind::SEXP:
			return childrenEqual(this->value.sexp, other.value.sexp);
        case SexpValueKind::ATOM:
			return this->value.str == other.value.str;
		}
	}

	auto Sexp::arguments() -> SexpArgumentIterator {
		return SexpArgumentIterator{*this};
	}

	auto Sexp::unescaped(std::string strval) -> Sexp {
		auto s = Sexp{};
        s.attributes.kind = SexpValueKind::ATOM;
		s.value.str = std::move(strval);
		return std::move(s);
	}

    auto Sexp::unescaped(std::string strval,int64_t startpos, int64_t endpos) -> Sexp {
        auto s = Sexp{};
        s.attributes.kind = SexpValueKind::ATOM;
        s.startpos = startpos;
        s.endpos = endpos;
        s.value.str = std::move(strval);
        return std::move(s);
    }

    auto Sexp::unescaped(std::string strval, SexpAtomKind atomkind, int64_t startpos, int64_t endpos) -> Sexp {
        auto s = Sexp{};
        s.attributes.kind = SexpValueKind::ATOM;
        s.startpos = startpos;
        s.endpos = endpos;
        s.attributes.atomkind = atomkind;
        s.value.str = std::move(strval);
        return std::move(s);
    }

	auto parse(std::string const& str, std::string& err) -> Sexp {
		auto sexprstack = std::stack<Sexp>{};
        sexprstack.push(Sexp(0)); // root
		auto nextiter = str.begin();
        SexpQuoteKind quoted = SexpQuoteKind::NONE;
		for(auto iter = nextiter; iter != str.end(); iter = nextiter) {
			nextiter = iter + 1;
			if(std::isspace(*iter)) continue;
			auto& cursexp = sexprstack.top();
			switch(*iter) {
			case '(':
                sexprstack.push(Sexp(iter - str.begin(), nextiter - str.begin()));
                if(quoted != SexpQuoteKind::NONE){
                    sexprstack.top().attributes.quotekind = quoted;
                    quoted = SexpQuoteKind::NONE;
                }
				break;
			case ')': {
				auto topsexp = std::move(sexprstack.top());
				sexprstack.pop();
				if(sexprstack.size() == 0) {
					err = std::string{"too many ')' characters detected, closing sexprs that don't exist, no good."};
					return Sexp{};
				}
                topsexp.endpos = nextiter - str.begin();
				auto& top = sexprstack.top();
				top.addChild(std::move(topsexp));
				break;
			}
            case '\'': {
                quoted = SexpQuoteKind::SINGLEQUOTE;
                break;
            }
            case '`': {
                quoted = SexpQuoteKind::BACKQUOTE;
                break;
            }
			case '"': {
				auto i = iter+1;
				auto start = i;
				for(; i != str.end(); ++i) {
					if(*i == '\\') { ++i; continue; }
					if(*i == '"') break;
				}
				if(i == str.end()) {
					err = std::string{"Unterminated string literal"};
					return Sexp{};
				}
				auto resultstr = std::string{};
				resultstr.reserve(i - start);
				for(auto it = start; it != i; ++it) {
					switch(*it) {
					case '\\': {
						++it;
						if(it == i) {
							err = std::string{"Unfinished escape sequence at the end of the string"};
							return Sexp{};
						}
						auto pos = std::find(escape_chars.begin(), escape_chars.end(), *it);
						if(pos == escape_chars.end()) {
							err = std::string{"invalid escape char '"} + *it + '\'';
							return Sexp{};
						}
						resultstr.push_back(escape_vals[pos - escape_chars.begin()]);
						break;
					}
					default:
						resultstr.push_back(*it);
					}
				}

                int64_t x = i - str.begin();
                sexprstack.top().addChildUnescaped(std::move(resultstr), SexpAtomKind::STRING, iter - str.begin(), x);
                if(quoted != SexpQuoteKind::NONE){
                    sexprstack.top().value.sexp.back().attributes.quotekind = quoted;
                    quoted = SexpQuoteKind::NONE;
                }

                std::cout << resultstr << " startpos = " << iter - str.begin() << " : endpos = " << x  << "\n";

				nextiter = i + 1;
				break;
			}
			case ';':
				for(; nextiter != str.end() && *nextiter != '\n' && *nextiter != '\r'; ++nextiter) {}
				for(; nextiter != str.end() && (*nextiter == '\n' || *nextiter == '\r'); ++nextiter) {}
				break;
            case '#':{
                //rjd: uses fall-through
                bool willbreak = false;
                switch(*nextiter){
                    case '\'': {
                        quoted = SexpQuoteKind::FUNCQUOTE;
                        nextiter += 1;
                        willbreak = true;
                        break;
                    }
                    case '|': {
                        auto nexti = iter + 1;
                        int commentcount = 1;
                        auto i = nexti;
                        for(; i != str.end(); i = nexti) {
                            nexti = i + 1;
                            if(*i == '#' && *nexti == '|'){
                                ++commentcount;
                            }
                            else if(*i == '|' && *nexti == '#'){
                                --commentcount;
                            }
                            if(commentcount == 0) break;
                        }
                        if(i == str.end()) {
                            err = std::string{"Unclosed block comment"};
                            return Sexp{};
                        }
                        nextiter = i + 2;
                        willbreak = true;
                    }
                }
                if(willbreak) break;
                [[clang::fallthrough]];
            }
             default:
				auto symend = std::find_if(iter, str.end(), [](char const& c) { return std::isspace(c) || c == ')' || c == '('; });
				auto& top = sexprstack.top();
                auto x = symend - str.begin();
    //            std::cout << std::string{iter,symend} << " startpos = " << iter - str.begin() << " : endpos = " << x << "\n";
     //           std::cout << "dist is " << std::distance(iter,symend) << " symend = " << std::string{iter,symend}.length() << "\n";
                std::cout << std::string{iter,symend};
                top.addChild(Sexp{std::string{iter, symend}, iter - str.begin(),x});
                if(quoted != SexpQuoteKind::NONE){
                    top.value.sexp.back().attributes.quotekind = quoted;
                    quoted = SexpQuoteKind::NONE;
                }
				nextiter = symend;
			}
		}
		if(sexprstack.size() != 1) {
			err = std::string{"not enough s-expressions were closed by the end of parsing"};
			return Sexp{};
		}
		return std::move(sexprstack.top());
	}

	auto parse(std::string const& str) -> Sexp {
		auto ignored_error = std::string{};
		return parse(str, ignored_error);
	}

    void closeStack(std::stack<Sexp>& stack, const int64_t endpos){
        while(stack.size() != 1){
            auto topsexp = std::move(stack.top());
            stack.pop();
            topsexp.endpos = topsexp.getChild(topsexp.childCount() -1).endpos;
            auto& top = stack.top();
            top.addChild(std::move(topsexp));
        }
    }

    void addString(std::stack<Sexp>& stack,std::string str, int64_t start, int64_t end, SexpQuoteKind quoted){
        stack.top().addChildUnescaped(std::move(str), SexpAtomKind::STRING, start, end);
        if(quoted != SexpQuoteKind::NONE){
            stack.top().value.sexp.back().attributes.quotekind = quoted;
            quoted = SexpQuoteKind::NONE;
        }
    }

    auto parseBroken(std::string const& str, std::string& err) -> Sexp {
        auto sexprstack = std::stack<Sexp>{};
        sexprstack.push(Sexp(0)); // root
        auto nextiter = str.begin();
        SexpQuoteKind quoted = SexpQuoteKind::NONE;
        SexpAtomKind atomkind = SexpAtomKind::NONE;
        SexpSexpKind sexpkind = SexpSexpKind::NONE;
        SexpMacroSyntaxKind macrokind = SexpMacroSyntaxKind::NONE;
        for(auto iter = nextiter; iter != str.end(); iter = nextiter) {
            nextiter = iter + 1;
            if(std::isspace(*iter)) continue;
            auto& cursexp = sexprstack.top();
            switch(*iter) {
            case '(':
                sexprstack.push(Sexp(iter - str.begin(), nextiter - str.begin()));
                if(quoted != SexpQuoteKind::NONE){
                    sexprstack.top().attributes.quotekind = quoted;
                    quoted = SexpQuoteKind::NONE;
                }
                if(sexpkind != SexpSexpKind::NONE){
                    sexprstack.top().attributes.sexpkind = sexpkind;
                    sexpkind = SexpSexpKind::NONE;
                }
                break;
            case ')': {
                auto topsexp = std::move(sexprstack.top());
                sexprstack.pop();
                if(sexprstack.size() == 0) {
                    err = std::string{"too many ')' characters detected, closing sexprs that don't exist, no good."};
                    return std::move(topsexp);
                }
                topsexp.endpos = nextiter - str.begin();
                auto& top = sexprstack.top();
                top.addChild(std::move(topsexp));
                break;
            }
            case '\'': {
                quoted = SexpQuoteKind::SINGLEQUOTE;
                break;
            }
            case '`': {
                quoted = SexpQuoteKind::BACKQUOTE;
                break;
            }
            case ',':{
                switch(*nextiter){
                    case '@': {
                        macrokind = SexpMacroSyntaxKind::ATSPLICE;
                        nextiter += 1;
                        break;
                    }
                    case '.': {
                        macrokind = SexpMacroSyntaxKind::COMMADOTSPLICE;
                        nextiter += 1;
                        break;
                    default:
                        macrokind = SexpMacroSyntaxKind::COMMASPLICE;
                        break;
                    }
                }
                break;
            }
            case '"': {
                auto i = iter+1;
                auto start = i;
                for(; i != str.end(); ++i) {
                    if(*i == '\\') { ++i; continue; }
                    if(*i == '"') break;
                }
                if(i == str.end()) {
                    err = std::string{"Unterminated string literal"};
                    auto resultstr = std::string{iter + 1,str.end()};
                    addString(sexprstack,resultstr,iter - str.begin(), iter - str.begin() + resultstr.length() + 2,quoted);
                    int64_t len = static_cast<int64_t>(str.length() + 2 + 16);
                    sexprstack.top().addChild(Sexp{":sexpresso-error",static_cast<int64_t>(str.length() + 2),len});

                    closeStack(sexprstack, nextiter - str.begin());
                    return std::move(sexprstack.top());
                    //return Sexp{};
                }
                auto resultstr = std::string{};
                resultstr.reserve(i - start);
                for(auto it = start; it != i; ++it) {
                    switch(*it) {
                    case '\\': {
                        ++it;
                        if(it == i) {
                            err = std::string{"Unfinished escape sequence at the end of the string"};
                            auto resultstr = std::string{iter + 1,i};
                            addString(sexprstack,resultstr,iter - str.begin(),i - str.begin(), quoted);

                            int64_t len = static_cast<int64_t>(str.length() + 16);
                            sexprstack.top().addChild(Sexp{":sexpresso-error",static_cast<int64_t>(str.length()),len});

                            closeStack(sexprstack, nextiter - str.begin());
                            return std::move(sexprstack.top());
                           // return Sexp{};
                        }
                        auto pos = std::find(escape_chars.begin(), escape_chars.end(), *it);
                        if(pos == escape_chars.end()) {
                            err = std::string{"invalid escape char '"} + *it + '\'';
                            auto resultstr = std::string{iter + 1,i};
                            addString(sexprstack,resultstr,iter - str.begin(),i - str.begin(), quoted);

                            int64_t len = static_cast<int64_t>(str.length() + 16);
                            sexprstack.top().addChild(Sexp{":sexpresso-error",static_cast<int64_t>(str.length()),len});

                            closeStack(sexprstack, nextiter - str.begin());
                            return std::move(sexprstack.top());
                           // return Sexp{};
                        }
                        resultstr.push_back(escape_vals[pos - escape_chars.begin()]);
                        break;
                    }
                    default:
                        resultstr.push_back(*it);
                    }
                }

                int64_t x = i - str.begin();
                sexprstack.top().addChildUnescaped(std::move(resultstr), SexpAtomKind::STRING, iter - str.begin(), x);
                if(quoted != SexpQuoteKind::NONE){
                    sexprstack.top().value.sexp.back().attributes.quotekind = quoted;
                    quoted = SexpQuoteKind::NONE;
                }

                std::cout << resultstr << " startpos = " << iter - str.begin() << " : endpos = " << x  << "\n";

                nextiter = i + 1;
                break;
            }
            case ';':
                for(; nextiter != str.end() && *nextiter != '\n' && *nextiter != '\r'; ++nextiter) {}
                for(; nextiter != str.end() && (*nextiter == '\n' || *nextiter == '\r'); ++nextiter) {}
                break;
            case '#':{
                //rjd: uses fall-through
                bool willbreak = false;
                switch(*nextiter){
                    case '\'': {
                        quoted = SexpQuoteKind::FUNCQUOTE;
                        nextiter += 1;
                        willbreak = true;
                        break;
                    }
                    case '\\': {
                        atomkind = SexpAtomKind::CHAR;
                        nextiter += 1;
                        willbreak = true;
                        break;
                    }
                    case 'b':
                    case 'B': {
                        atomkind = SexpAtomKind::BINARY;
                        nextiter += 1;
                        willbreak = true;
                        break;
                    }
                    case 'O':
                    case 'o': {
                        atomkind = SexpAtomKind::OCTAL;
                        nextiter += 1;
                        willbreak = true;
                        break;
                    }
                    case 'X':
                    case 'x': {
                        atomkind = SexpAtomKind::HEX;
                        nextiter += 1;
                        willbreak = true;
                        break;
                    }
                    case '(': {
                        sexpkind = SexpSexpKind::VECTOR;
                        willbreak = true;
                        break;
                    }
                    case 'C':
                    case 'c': {
                        sexpkind = SexpSexpKind::COMPLEX;
                        nextiter += 1;
                        willbreak = true;
                        break;
                    }
                    case '|': {
                        auto nexti = iter + 1;
                        int commentcount = 1;
                        auto i = nexti;
                        for(; i != str.end(); i = nexti) {
                            nexti = i + 1;
                            if(*i == '#' && *nexti == '|'){
                                ++commentcount;
                            }
                            else if(*i == '|' && *nexti == '#'){
                                --commentcount;
                            }
                            if(commentcount == 0) break;
                        }
                        if(i == str.end()) {
                            err = std::string{"Unclosed block comment"};
                            int64_t len = static_cast<int64_t>(str.length() + 16);
                            sexprstack.top().addChild(Sexp{":sexpresso-error",static_cast<int64_t>(str.length()),len});

                            closeStack(sexprstack, nextiter - str.begin());
                            return std::move(sexprstack.top());
                            //return Sexp{};
                        }
                        nextiter = i + 2;
                        willbreak = true;
                    }
                }
                if(willbreak) break;
                [[clang::fallthrough]];
            }
             default:
                auto symend = std::find_if(iter, str.end(), [](char const& c) { return std::isspace(c) || c == ')' || c == '('; });
                auto& top = sexprstack.top();
                auto x = symend - str.begin();
                std::cout << std::string{iter,symend};

                top.addChild(Sexp{std::string{iter, symend}, iter - str.begin(),x});
                if(quoted != SexpQuoteKind::NONE){
                    top.value.sexp.back().attributes.quotekind = quoted;
                    quoted = SexpQuoteKind::NONE;
                }
                if(atomkind != SexpAtomKind::NONE){
                    top.value.sexp.back().attributes.atomkind = atomkind;
                    atomkind = SexpAtomKind::NONE;
                }

                nextiter = symend;
            }
        }
        if(sexprstack.size() != 1) {
            err = std::string{"not enough s-expressions were closed by the end of parsing"};
            //return Sexp{};
            int64_t len = static_cast<int64_t>(str.length() + 16 + 1);
            sexprstack.top().addChild(Sexp{":sexpresso-error",static_cast<int64_t>(str.length() + 1),len});
            closeStack(sexprstack, nextiter - str.begin());
        }
        return std::move(sexprstack.top());
    }

	auto escape(std::string const& str) -> std::string {
		auto escape_count = countEscapeValues(str);
		if(escape_count == 0) return str;
		auto result_str = std::string{};
		result_str.reserve(str.size() + escape_count);
		for(auto c : str) {
			auto loc = std::find(escape_vals.begin(), escape_vals.end(), c);
			if(loc == escape_vals.end()) result_str.push_back(c);
			else {
				result_str.push_back('\\');
				result_str.push_back(escape_chars[loc - escape_vals.begin()]);
			}
		}
		return std::move(result_str);
	}

	SexpArgumentIterator::SexpArgumentIterator(Sexp& sexp) : sexp(sexp) {}

	auto SexpArgumentIterator::begin() -> iterator {
		if(this->size() == 0) return this->end(); else return ++(this->sexp.value.sexp.begin());
	}

	auto SexpArgumentIterator::end() -> iterator { return this->sexp.value.sexp.end(); }

	auto SexpArgumentIterator::begin() const -> const_iterator {
		if(this->size() == 0) return this->end(); else return ++(this->sexp.value.sexp.begin());
	}

	auto SexpArgumentIterator::end() const -> const_iterator { return this->sexp.value.sexp.end(); }

	auto SexpArgumentIterator::empty() const -> bool { return this->size() == 0;}

	auto SexpArgumentIterator::size() const -> size_t {
		auto sz = this->sexp.value.sexp.size();
		if(sz == 0) return 0; else return sz-1;
	}
}
