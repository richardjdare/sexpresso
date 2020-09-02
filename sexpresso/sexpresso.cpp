// Author: Isak Andersson 2016 bitpuffin dot com
// hacked to pieces by richardjdare 2020
#include <vector>
#include <string>
#include <cstdint>
#include "sexpresso.hpp"
#include <stack>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <array>
#include <iostream>

namespace sexpresso {

    Sexp::Sexp() {
        this->kind = SexpValueKind::SEXP;
        this->atomkind = SexpAtomKind::NONE;
        this->sexpkind = SexpSexpKind::NONE;
    }
    Sexp::Sexp(int64_t startpos, int64_t endpos){
        this->kind = SexpValueKind::SEXP;
        this->atomkind = SexpAtomKind::NONE;
        this->sexpkind = SexpSexpKind::NONE;
        this->startpos = startpos;
        this->value.startpos = 0;
        this->endpos = endpos;
        this->value.endpos = 0;
    }
    Sexp::Sexp(std::string const& strval) {
        this->kind = SexpValueKind::ATOM;
        this->atomkind = SexpAtomKind::SYMBOL;
        this->sexpkind = SexpSexpKind::NONE;
        this->value.str = escape(strval);
    }
    Sexp::Sexp(std::string const& strval, SexpAtomKind atomkind) {
        this->kind = SexpValueKind::ATOM;
        this->sexpkind = SexpSexpKind::NONE;
        this->atomkind = atomkind;
        this->value.str = escape(strval);
    }
    Sexp::Sexp(std::string const& strval, int64_t startpos, int64_t endpos) {
        this->kind = SexpValueKind::ATOM;
        this->atomkind = SexpAtomKind::SYMBOL;
        this->sexpkind = SexpSexpKind::NONE;
        this->value.str = escape(strval);
        this->startpos = startpos;
        this->value.startpos = 0;
        this->endpos = endpos;
        this->value.endpos = 0;
    }
    Sexp::Sexp(std::string const& strval, int64_t startpos, int64_t endpos, SexpAtomKind atomkind) {
        this->kind = SexpValueKind::ATOM;
        this->sexpkind = SexpSexpKind::NONE;
        this->atomkind = atomkind;
        this->value.str = escape(strval);
        this->startpos = startpos;
        this->value.startpos = 0;
        this->endpos = endpos;
        this->value.endpos = 0;
    }
    Sexp::Sexp(std::vector<Sexp> const& sexpval) {
        this->kind = SexpValueKind::SEXP;
        this->atomkind = SexpAtomKind::NONE;
        this->sexpkind = SexpSexpKind::NONE;
        this->value.sexp = sexpval;
    }
    Sexp::Sexp(std::vector<Sexp> const& sexpval, int64_t startpos, int64_t endpos) {
        this->kind = SexpValueKind::SEXP;
        this->atomkind = SexpAtomKind::NONE;
        this->sexpkind = SexpSexpKind::NONE;
        this->value.sexp = sexpval;
        this->startpos = startpos;
        this->value.startpos = 0;
        this->endpos = endpos;
        this->value.endpos = 0;
    }

    auto Sexp::addChild(Sexp sexp) -> void {
        if(this->kind == SexpValueKind::ATOM) {
            this->kind = SexpValueKind::SEXP;
            this->value.sexp.push_back(Sexp{std::move(this->value.str), this->startpos, this->endpos});
        }
        this->value.sexp.push_back(std::move(sexp));
    }

    auto Sexp::addChild(std::string str) -> void {
        this->addChild(Sexp{std::move(str), this->startpos});
        this->value.sexp.back().atomkind = SexpAtomKind::STRING;
    }

    auto Sexp::addChildUnescaped(std::string str) -> void {
        this->addChild(Sexp::unescaped(std::move(str)));
        this->value.sexp.back().atomkind = SexpAtomKind::STRING;
    }

    auto Sexp::addChildUnescaped(std::string str, int64_t startpos, int64_t endpos) -> void {
        this->addChild(Sexp::unescaped(std::move(str), startpos, endpos));
        this->value.sexp.back().atomkind = SexpAtomKind::STRING;
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
        switch(this->kind) {
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
        if(this->kind == SexpValueKind::ATOM) return nullptr;

        auto paths = splitPathString(path);

        auto* cur = this;
        for(auto i = paths.begin(); i != paths.end();) {
            auto start = i;
            for(auto& child : cur->value.sexp) {
                auto brk = false;
                switch(child.kind) {
                    case SexpValueKind::ATOM:
                        if(i == paths.end() - 1 && child.value.str == *i) return &child;
                        else continue;
                    case SexpValueKind::SEXP:
                        if(child.value.sexp.size() == 0) continue;
                        auto& fst = child.value.sexp[0];
                        switch(fst.kind) {
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
            switch(s.kind) {
                case SexpValueKind::SEXP: {
                    if(s.childCount() == 0) return false;
                    auto& hd = s.getChild(0);
                    switch(hd.kind) {
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

    static auto stringPathnameToString(std::string const& s) -> std::string {
        if(s.size() == 0) return std::string{"#p\"\""};
        return ("#p\"" + escape(s) + '"');
    }

    static auto toStringImpl(Sexp const& sexp, std::ostringstream& ostream, SexpressoPrintMode printmode) -> void {
        for(SexpAttributeKind k : sexp.attributes){
            switch(k){
            case SexpAttributeKind::QUOTE:
                ostream << "'";
                break;
            case SexpAttributeKind::BACKQUOTE:
                ostream << "`";
                break;
            case SexpAttributeKind::FUNCQUOTE:
                ostream << "#'";
                break;
            case SexpAttributeKind::COMMASPLICE:
                ostream << ",";
                break;
             case SexpAttributeKind::ATSPLICE:
                ostream << ",@";
                break;
             case SexpAttributeKind::DOTSPLICE:
                ostream << ",.";
                break;
            }
        }
        switch(sexp.kind) {
            case SexpValueKind::ATOM:
                switch(sexp.atomkind){
                    case SexpAtomKind::STRING:
                        ostream << stringStringToString(sexp.value.str);
                    break;
                    case SexpAtomKind::PATHNAME:
                        ostream << stringPathnameToString(sexp.value.str);
                    break;
                    case SexpAtomKind::CHAR:
                    case SexpAtomKind::HEX:
                    case SexpAtomKind::OCTAL:
                    case SexpAtomKind::BINARY:
                        ostream << stringScalarToString(sexp.value.str,sexp.atomkind);
                    break;
                    default:
                    ostream << stringSymbolToString(sexp.value.str);
                }
            break;
		case SexpValueKind::SEXP:
            switch(sexp.sexpkind){
                case SexpSexpKind::VECTOR:
                    ostream << "#";
                break;
                case SexpSexpKind::COMPLEX:
                    ostream << "#c";
                break;

                default:break;
            }
            if(printmode == SexpressoPrintMode::TOP_LEVEL_PARENS){
                switch(sexp.value.sexp.size()) {
                case 0:
                    ostream << "()";
                    break;
                case 1:
                    ostream << '(';
                    toStringImpl(sexp.value.sexp[0], ostream,SexpressoPrintMode::TOP_LEVEL_PARENS);
                    ostream <<  ')';
                    break;
                default:
                    ostream << '(';
                    for(auto i = sexp.value.sexp.begin(); i != sexp.value.sexp.end(); ++i) {
                        toStringImpl(*i, ostream,SexpressoPrintMode::TOP_LEVEL_PARENS);
                        if(i != sexp.value.sexp.end()-1) ostream << ' ';
                    }
                    ostream << ')';
                }
            }
            else {
                for(auto i = sexp.value.sexp.begin(); i != sexp.value.sexp.end(); ++i) {
                    toStringImpl(*i, ostream,SexpressoPrintMode::TOP_LEVEL_PARENS);
                    if(i != sexp.value.sexp.end()-1) ostream << ' ';
                }
            }
        }
    }

    auto Sexp::toString(SexpressoPrintMode printmode) const -> std::string {
        auto ostream = std::ostringstream{};
        toStringImpl(*this,ostream,printmode);
            return ostream.str();
    }

    auto Sexp::isString() const -> bool {
        return this->kind == SexpValueKind::ATOM;
    }

    auto Sexp::isSexp() const -> bool {
        return this->kind == SexpValueKind::SEXP;
    }

    auto Sexp::isNil() const -> bool {
        return this->kind == SexpValueKind::SEXP && this->childCount() == 0;
    }

    static auto childrenEqual(std::vector<Sexp> const& a, std::vector<Sexp> const& b) -> bool {
        if(a.size() != b.size()) return false;

        for(auto i = 0; i < a.size(); ++i) {
                if(!a[i].equal(b[i])) return false;
        }
        return true;
    }

    auto Sexp::equal(Sexp const& other) const -> bool {
        if(this->kind != other.kind) return false;
        switch(this->kind) {
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
        s.kind = SexpValueKind::ATOM;
        s.value.str = std::move(strval);
        return std::move(s);
    }

    auto Sexp::unescaped(std::string strval,int64_t startpos, int64_t endpos) -> Sexp {
        auto s = Sexp{};
        s.kind = SexpValueKind::ATOM;
        s.startpos = startpos;
        s.endpos = endpos;
        s.value.str = std::move(strval);
        return std::move(s);
    }

    auto Sexp::unescaped(std::string strval, SexpAtomKind atomkind, int64_t startpos, int64_t endpos) -> Sexp {
        auto s = Sexp{};
        s.kind = SexpValueKind::ATOM;
        s.startpos = startpos;
        s.endpos = endpos;
        s.atomkind = atomkind;
        s.value.str = std::move(strval);
        return std::move(s);
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

    void addStringOrPathname(std::stack<Sexp>& stack,std::string str, int64_t start, int64_t end, std::vector<SexpAttributeKind>&attribs, SexpAtomKind atomKind = SexpAtomKind::STRING){
        SexpAtomKind stringKind = SexpAtomKind::STRING;
        if(atomKind == SexpAtomKind::PATHNAME) stringKind = SexpAtomKind::PATHNAME;
        stack.top().addChildUnescaped(std::move(str),stringKind, start, end);
        stack.top().value.sexp.back().attributes = attribs;
    }

    auto parse(std::string const& str, std::string& err) -> Sexp {
        std::string errsymbol = ":sexpresso-error";
        return parse(str, err, errsymbol);
    }

    auto parse(std::string const& str, std::string& err, std::string& errsymbol) -> Sexp {
        auto sexprstack = std::stack<Sexp>{};
        sexprstack.push(Sexp(0)); // root
        auto nextiter = str.begin();

        SexpAtomKind atomkind = SexpAtomKind::NONE;
        SexpSexpKind sexpkind = SexpSexpKind::NONE;
        std::vector<SexpAttributeKind> attribs;

        for(auto iter = nextiter; iter != str.end(); iter = nextiter) {
            nextiter = iter + 1;
            if(std::isspace(static_cast<unsigned char>(*iter))) continue;
            auto& cursexp = sexprstack.top();
            switch(*iter) {
            case '(':
                sexprstack.push(Sexp(iter - str.begin(), nextiter - str.begin()));
                if(sexpkind != SexpSexpKind::NONE){
                    sexprstack.top().sexpkind = sexpkind;
                    sexpkind = SexpSexpKind::NONE;
                }
                sexprstack.top().attributes = attribs;
                attribs.clear();
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
                attribs.push_back(SexpAttributeKind::QUOTE);
                break;
            }
            case '`': {
                attribs.push_back(SexpAttributeKind::BACKQUOTE);
                break;
            }
            case ',':{
                if(nextiter == str.end()) break;
                switch(*nextiter){
                    case '@': {
                        attribs.push_back(SexpAttributeKind::ATSPLICE);
                        nextiter += 1;
                        break;
                    }
                    case '.': {
                        attribs.push_back(SexpAttributeKind::DOTSPLICE);
                        nextiter += 1;
                        break;
                    default:
                        attribs.push_back(SexpAttributeKind::COMMASPLICE);
                        break;
                    }
                }
                break;
            }
            case '"': {
                auto i = iter+1;
                auto start = i;
                for(; i != str.end(); ++i) {
                    if(*i == '\\' && (i + 1 != str.end())) { ++i; continue; }
                    if(*i == '"') break;
                }
                if(i == str.end()) {
                    err = std::string{"Unterminated string literal"};
                    auto resultstr = std::string{iter + 1,str.end()};

                    addStringOrPathname(sexprstack,resultstr,iter - str.begin(), iter - str.begin() + resultstr.length() + 2, attribs, atomkind);
                    attribs.clear();
                    atomkind = SexpAtomKind::NONE;
                    int64_t len = static_cast<int64_t>(str.length() + 2 + errsymbol.length());
                    sexprstack.top().addChild(Sexp{errsymbol, static_cast<int64_t>(str.length() + 2), len});

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
                            addStringOrPathname(sexprstack,resultstr,iter - str.begin(),i - str.begin(), attribs, atomkind);
                            attribs.clear();
                            atomkind = SexpAtomKind::NONE;
                            int64_t len = static_cast<int64_t>(str.length() + errsymbol.length());
                            sexprstack.top().addChild(Sexp{errsymbol, static_cast<int64_t>(str.length()), len});

                            closeStack(sexprstack, nextiter - str.begin());
                            return std::move(sexprstack.top());
                        }
                        /* lets not test escape chars until the parser can keep going after errors like this, while
                          reporting back nonlethal problems it encountered.

                        auto pos = std::find(escape_chars.begin(), escape_chars.end(), *it);
                        if(pos == escape_chars.end()) {
                            err = std::string{"invalid escape char '"} + *it + '\'';
                            auto resultstr = std::string{iter + 1,i};
                            addString(sexprstack,resultstr,iter - str.begin(),i - str.begin(), attribs);
                            attribs.clear();
                            int64_t len = static_cast<int64_t>(str.length() + errsymbol.length());
                            sexprstack.top().addChild(Sexp{errsymbol, static_cast<int64_t>(str.length()), len});

                            closeStack(sexprstack, nextiter - str.begin());
                            return std::move(sexprstack.top());
                        }
                        resultstr.push_back(escape_vals[pos - escape_chars.begin()]);
                        */
                        resultstr.push_back('\\');
                        resultstr.push_back(*it);
                        break;
                    }
                    default:
                        resultstr.push_back(*it);
                    }
                }

                int64_t stringEndPos = i - str.begin() + 1;
                addStringOrPathname(sexprstack,resultstr,iter - str.begin(), stringEndPos,attribs,atomkind);
        //        sexprstack.top().addChildUnescaped(std::move(resultstr), SexpAtomKind::STRING, iter - str.begin(), stringEndPos);
        //        sexprstack.top().value.sexp.back().attributes = attribs;
                attribs.clear();
                sexpkind = SexpSexpKind::NONE;

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
                if(nextiter == str.end()) break;

                switch(*nextiter){
                    case '\'': {
                        attribs.push_back(SexpAttributeKind::FUNCQUOTE);
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
                    case 'P':
                    case 'p':{
                        atomkind = SexpAtomKind::PATHNAME;
                        nextiter += 1;
                        willbreak = true;
                        break;
                    }
                    case '|': {
                        auto nexti = iter;
                        int commentcount = 0;
                        for(auto i = nexti; i != str.end(); i=nexti){
                            nexti = i + 1;
                            if(nexti != str.end()){
                                if(*i == '#' && *nexti == '|'){
                                    ++commentcount;
                                }
                                else if(*i == '|' && *nexti == '#'){
                                    --commentcount;
                                }
                            }
                            if(commentcount == 0) break;
                        }

                        if(nexti == str.end()) {
                            err = std::string{"Unclosed block comment"};
                            int64_t len = static_cast<int64_t>(str.length() + errsymbol.length());
                            sexprstack.top().addChild(Sexp{errsymbol, static_cast<int64_t>(str.length()), len});

                            closeStack(sexprstack, nextiter - str.begin());
                            return std::move(sexprstack.top());
                            //return Sexp{};
                        }
                        nextiter = nexti + 1;
                        willbreak = true;
                    }
                }
                if(willbreak) break;
                [[clang::fallthrough]];
            }
             default:
                auto symend = std::find_if(iter, str.end(), [](char const& c) { return std::isspace(static_cast<unsigned char>(c)) || c == ')' || c == '('; });
                auto& top = sexprstack.top();
                auto x = symend - str.begin();
                top.addChild(Sexp{std::string{iter, symend}, iter - str.begin(),x});
                top.value.sexp.back().attributes = attribs;
                attribs.clear();
                if(atomkind != SexpAtomKind::NONE){
                    top.value.sexp.back().atomkind = atomkind;
                    atomkind = SexpAtomKind::NONE;
                }

                nextiter = symend;
            }
        }
        if(sexprstack.size() != 1) {
            err = std::string{"not enough s-expressions were closed by the end of parsing"};
            //return Sexp{};
            int64_t len = static_cast<int64_t>(str.length() + errsymbol.length() + 1);
            sexprstack.top().addChild(Sexp{errsymbol, static_cast<int64_t>(str.length() + 1), len});
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
