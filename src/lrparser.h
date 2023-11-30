#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <list>
#include <map>
#include <optional>
#include <regex>
#include <stack>
#include <string>
#include <variant>
#include "tools.h"

template<typename T>
ostream& operator<<(ostream& os, const std::list<T>& myList) {
    os << "{";
    if (!myList.empty()) {
        auto it = myList.begin();
        os << *it;
        ++it;
        for (; it != myList.end(); ++it) {
            os << ", " << *it;
        }
    }
    os << "}";
    return os;
}

template<typename K, typename V>
ostream& operator<<(ostream& os, const std::map<K, V>& myMap) {
    os << "{";
    if (!myMap.empty()) {
        auto it = myMap.begin();
        os << it->first << ": " << it->second;
        ++it;
        for (; it != myMap.end(); ++it) {
            os << ", " << it->first << ": " << it->second;
        }
    }
    os << "}";
    return os;
}

class SyntaxError : public std::exception {
private:
    std::string error_log;

public:
    SyntaxError(const std::string& msg, const std::string& file, int line) {
        error_log = file + ": SyntaxError: " + msg + " at line " + std::to_string(line);
    }

    const char* what() const noexcept override {
        return error_log.c_str();
    }
};

// Define a macro to create the exception with file and line
#define SyntaxError(msg) throw SyntaxError(msg, __FILE__, __LINE__)

typedef void (*standalone_function_t)();

/* class NonTerminal {
    private:
        const std::string name;
        std::map<std::string, standalone_function_t> mapping;

    public:
        NonTerminal(std::string name, std::map<std::string, std::string> mapping) : name(name), mapping(mapping) {}

        std::string getName () const {
            return name;
        }

        std::map<std::string, standalone_function_t> getMapping () {
            return mapping;
        }

        std::list<std::string> getMappedStrings () {
            std::list<std::string> key;
            for(std::map<std::string, standalone_function_t>::iterator it = mapping.begin(); it != mapping.end(); ++it) {
                key.push_back(it->first);           
            }
            return key;
        }

        void trigger(std::string function) {
            auto x = mapping.find(function);
            
            if (x != mapping.end()) {
                result = x -> second(); // Call the stored function directly
            }
        }
}; */

const std::string EPSILON = "''";

class Rule;

class Grammar {
    private:
        void initializeRulesAndAlphabetAndNonterminals (std::string text);

        void initializeAlphabetAndTerminals ();

        bool collectDevelopmentFirsts(std::list<std::string> const& development, std::list<std::string>& nonterminalFirsts);

        void initializeFirsts ();

        void initializeFollows();

    public:
        std::list<std::string> alphabet;
        std::list<std::string> nonterminals;
        std::list<std::string> terminals;
        std::list<Rule> rules;
        std::string text;
        std::map<std::string, std::list<std::string>> firsts;
        std::map<std::string, std::list<std::string>> follows;
        std::string axiom;

        explicit Grammar(std::string const& text) : axiom("") {
            initializeRulesAndAlphabetAndNonterminals(text);
            initializeAlphabetAndTerminals();
            initializeFirsts();
            initializeFollows();
        }

        std::list<Rule> getRulesForNonterminal(std::string nonterminal);

        std::list<std::string> getSequenceFirsts(std::list<std::string> sequence) const {
            std::list<std::string> result = {};
            bool epsilonInSymbolFirsts = true;

            for (const std::string& symbol : sequence) {
                epsilonInSymbolFirsts = false;

                if (std::find(terminals.begin(), terminals.end(), symbol) != terminals.end()) {
                    addUnique(symbol, result);

                    break;
                }
                
                for (const std::string& first : firsts.at(symbol)) {
                    epsilonInSymbolFirsts |= first == EPSILON;
                    addUnique(first, result);
                }
                
                epsilonInSymbolFirsts |= firsts.at(symbol).size() == 0;

                if (!epsilonInSymbolFirsts) break;                
            }

            if (epsilonInSymbolFirsts) addUnique(EPSILON, result);

            return result;
        }       

        /* bool validate(std::list symbols, Token nextTok) {
            std::string token_type = nextTok.getType();
            std::string grammarQuery;
            for (auto symbol : symbols) {
                std::string name = typeid(symbol) == typeid(Token) ? symbol.getType() : symbol.getName();                
                grammarQuery.append(name + ' ');
            }
            grammarQuery.append(token_type);

            for (NonTerminal rule : rules) {
                for (std::string s : rule.getMappedStrings()) {
                    if (s.rfind(grammarQuery, 0) == 0) {
                        return true;
                    }
                }
            }
            return false;
        }

        void reduce(std::stack symbols) {    
            std::string grammarQuery;
            for (auto symbol : symbols) {
                std::string name = typeid(symbol) == typeid(Token) ? symbol.getType() : symbol.getName();
                grammarQuery.append(name + ' ');
            }
            grammarQuery.pop_back();

            for (NonTerminal rule : rules) {
                for (std::string s : rule.getMappedStrings()) {
                    if (grammarQuery.find(s) != std::string::npos) {
                        NonTerminal reduced = new NonTerminal(rule.getName(), nullptr);
                        int reductions = std::ranges::count(s, ' ') + 1;
                        while (reductions != 0) {
                            symbols.pop();
                            reductions--;
                        }
                        symbols.push(reduced);
                    }
                }
            }
            // we know that we tried to shift tokens before reduce which didnt work
            // we know that none of our grammar rules matches the symbol stack
            // therefor the syntax isnt valid
            throw SyntaxError("");
        } */
};

class Rule {
    public:
        Grammar& grammar;
        int index;
        std::string nonterminal;
        std::list<std::string> pattern;
        std::list<std::string> development;        
        
        Rule(Grammar& grammar, const std::string& text) : grammar(grammar), index(static_cast<grammar.rules.size()>) {
            std::list<std::string> split = splitString(text, "->");
            nonterminal = boost::trim_copy(split.front());
            pattern = trimElements(splitString(nonterminal, " "));
            development = trimElements(splitString(boost::trim_copy(split.back()), " "));
        }
        
        bool operator==(const Rule& that) const {
            if (nonterminal != that.nonterminal) {
			    return false;
		    }
		
		    if (development.size() != that.development.size()) {
			    return false;
		    }
		
		    if (development != that.development) {
		        return false;
		    }
		
		    return true;
        }
        
        friend ostream& operator<<(ostream& os, const Rule& rule) {
            os << rule.nonterminal << " -> ";
            if (!rule.development.empty()) {
                auto it = rule.development.begin();
                os << *it;
                ++it;
                for (; it != rule.development.end(); ++it) {
                    os << " " << *it;
                }
            }
            return os;
        }
};

void Grammar::initializeRulesAndAlphabetAndNonterminals (std::string text) {
    std::list<std::string> lines = splitString(text, "\n");

    for (const std::string& _ : lines) {
        std::string line = boost::trim_copy(_);

        if (line != "") {
            // should use unique_pointr here later
            Rule rule(*this, line);
            rules.push_back(rule);

            if (axiom.empty()) {
                axiom = rule.nonterminal;
            }
            
            addUnique(rule.nonterminal, alphabet);
            addUnique(rule.nonterminal, nonterminals);
        }
    }
}

void Grammar::initializeAlphabetAndTerminals () {
    for (const Rule& rule : rules) {
        for (const std::string& symbol : rule.development) {
            if (symbol != EPSILON && std::find(nonterminals.begin(), nonterminals.end(), symbol) == nonterminals.end()) {
                addUnique(symbol, alphabet);
                addUnique(symbol, terminals);
            }
        }
    }
}

bool Grammar::collectDevelopmentFirsts(std::list<std::string> const& development, std::list<std::string>& nonterminalFirsts) {
    bool result = false;
    bool epsilonInSymbolFirsts = true;
    
    for (const std::string& symbol : development) {
        epsilonInSymbolFirsts = false;

        if (isElement(symbol, terminals)) {
            result |= addUnique(symbol, nonterminalFirsts);
            
            break;
        }

        if (firsts.find(symbol) != firsts.end()) {
            for (const std::string& first : firsts.at(symbol)) {
                epsilonInSymbolFirsts |= first == EPSILON;
                result |= addUnique(first, nonterminalFirsts);
            }
        }

        if (!epsilonInSymbolFirsts) break;
    }

    if (epsilonInSymbolFirsts) {
        result |= addUnique(EPSILON, nonterminalFirsts);
    }

    return result;
}

void Grammar::initializeFirsts () {
    bool notDone;

    do{
        notDone = false;

        for (Rule& rule : rules) {
            std::list<std::string> nonterminalFirsts = getOrCreateArray(firsts, rule.nonterminal);

            if (rule.development.size() == 1 && rule.development.front() == EPSILON) {
                notDone |= addUnique(EPSILON, nonterminalFirsts);
            } else {
                notDone |= collectDevelopmentFirsts(rule.development, nonterminalFirsts);
            }
            firsts[rule.nonterminal] = nonterminalFirsts;
        }
    } while (notDone);
}

void Grammar::initializeFollows() {
    bool notDone;

    do {
        notDone = false;
        
        for (Rule& rule : rules) {
            if (rule == rules.front()) {
                std::list<std::string> nonterminalFollows = getOrCreateArray(follows, rule.nonterminal);
                notDone |= addUnique<std::string>("$", nonterminalFollows);
                follows[rule.nonterminal] = nonterminalFollows;
            }

            for (int i = 0; i < rule.development.size(); i++) {
                std::string symbol = getElement(i, rule.development);

                if (isElement(symbol, nonterminals)) {
                    std::list<std::string> symbolFollows = getOrCreateArray(follows, symbol);
                    
                    std::list<std::string> sliced = rule.development;
                    auto end = sliced.begin();
                    std::advance(end, i + 1);
                    sliced.erase(sliced.begin(), end);
                    
                    std::list<std::string> afterSymbolFirsts = getSequenceFirsts(sliced);

                    for (const std::string& first : afterSymbolFirsts) {
                        if (first == EPSILON) {
                            std::list<std::string> nonterminalFollows = follows.at(rule.nonterminal);

                            for (const std::string& _ : nonterminalFollows) {
                                notDone |= addUnique(_, symbolFollows);
                            }
                        } else {
                            notDone |= addUnique(first, symbolFollows);
                        }
                    }
                    follows[symbol] = symbolFollows;
                }
            }
        }
    } while (notDone);
}

std::list<Rule> Grammar::getRulesForNonterminal(std::string nonterminal) {
    std::list<Rule> result = {};

    for (const Rule& rule : rules) {
        if (nonterminal == rule.nonterminal) {
            result.push_back(rule);
        }
    }

    return result;
}

class Item;

class BasicItem {
    public:
        const Rule& rule;
        int dotIndex;
        std::list<std::string> lookAheads = {};
    
        BasicItem (const Rule& rule, int dotIndex) : rule(rule), dotIndex(dotIndex) {}

        bool addUniqueTo(std::list<BasicItem> items) const {
            return addUnique(*this, items);
        }

        std::list<Item> newItemsFromSymbolAfterDot();

        std::optional<Item> newItemAfterShift();

        bool operator==(const BasicItem& that) const {
            return rule == that.rule && dotIndex == that.dotIndex;
        }
};

class BasicLR1Item : public BasicItem {
    public:
        BasicLR1Item(const Rule& rule, int dotIndex) : BasicItem(rule, dotIndex) {
            if (rule.index == 0) {
                lookAheads.push_back("$");
            } else {
                lookAheads = {};
            }
        }

        std::list<Item> newItemsFromSymbolAfterDot();

        std::optional<Item> newItemAfterShift();

        bool addUniqueTo(std::list<BasicItem> items) const {
            bool result = false;

            for (BasicItem& item : items) {
                if (BasicItem::operator==(item)) {
                    for (const std::string& _ : lookAheads) {
                        result |= addUnique(_, item.lookAheads);
                    }

                    return result;
                }
            }

            items.push_back(*this);

            return true;
        }

        bool operator==(const BasicLR1Item& that) {
            return BasicItem::operator==(that) && includeEachOther(lookAheads, that.lookAheads);
        }
};

class Item : public BasicLR1Item {
    public:
        const std::string grammarType = "LR(1)";

        Item (const Rule& rule, int dotIndex) : BasicLR1Item(rule, dotIndex) {}
        
        bool addUniqueTo(std::list<Item>& items) const {
            //return addUnique(*this, items);
            bool result = false;

            for (BasicItem& item : items) {
                if (BasicItem::operator==(item)) {
                    for (const std::string& _ : lookAheads) {
                        result |= addUnique(_, item.lookAheads);
                    }

                    return result;
                }
            }

            items.push_back(*this);

            return true;
        }
        
        bool operator==(const Item& that) {
            return rule == that.rule && dotIndex == that.dotIndex && includeEachOther(lookAheads, that.lookAheads);
        }
};

std::list<Item> BasicItem::newItemsFromSymbolAfterDot() {
    std::list<Item> result = {};
    std::string ntR = getElement(dotIndex, rule.development);
    std::list<Rule> nonterminalRules = rule.grammar.getRulesForNonterminal(ntR);

    for (const Rule& rule : nonterminalRules) {
        addUnique(Item(rule, 0) , result);
    }

    return result;
}

std::optional<Item> BasicItem::newItemAfterShift() {
    if (dotIndex < rule.development.size() && getElement(dotIndex, rule.development) != EPSILON) {
        return std::optional<Item>(Item(rule, dotIndex + 1));
    }

    return std::nullopt;
}

std::list<Item> BasicLR1Item::newItemsFromSymbolAfterDot() {
    std::list<Item> result = BasicItem::newItemsFromSymbolAfterDot();

    if (result.empty()) return result;

    std::list<std::string> newLookAheads = {};
    bool epsilonPresent = false;
    std::list<std::string> firstsAfterSymbolAfterDot = rule.grammar.getSequenceFirsts(slice(rule.development, dotIndex + 1));

    for (const std::string& first : firstsAfterSymbolAfterDot) {
        if (EPSILON == first) {
            epsilonPresent = true;
        } else {
            addUnique(first, newLookAheads);
        }
    }

    if (epsilonPresent) {
        for (const std::string& _ : lookAheads) {
            addUnique(_, newLookAheads);
        }
    }

    for (Item& item : result) {
        item.lookAheads = slice(newLookAheads, 0);
    }
    
    return result;
}

std::optional<Item> BasicLR1Item::newItemAfterShift() {
    std::optional<Item> result = BasicItem::newItemAfterShift();

    if (result != std::nullopt) {
        result.value().lookAheads = slice(lookAheads, 0);
    }

    return result;
}

class Kernel {
    public:
        int index;
        std::list<Item> items;
        std::list<Item> closure;
        std::map<std::string, int> gotos;
        std::list<std::string> keys;
        
        //maybe initialize with grammar
        Kernel (int index, std::list<Item> items) : index(index), items(items), closure(slice(items, 0)) {
            gotos = {};
            keys = {};
        }

        bool operator==(const Kernel& that) {
            return includeEachOther(items, that.items);
        }
};

class LRClosureTable {
    public:
        Grammar& grammar;
        std::list<Kernel> kernels;

        explicit LRClosureTable(Grammar& grammar) : grammar(grammar) {
            kernels.push_back(Kernel(0, {Item(grammar.rules.front(), 0)}));

            for (int i = 0; i < kernels.size();) {
                Kernel& kernel = getUpdateableElement(i, kernels);

                updateClosure(kernel);

                if (addGotos(kernel, kernels)) {
                    i = 0;
                } else {
                    ++i;
                }
            }
        }

        void updateClosure(Kernel& kernel) {
            for (Item& closure : kernel.closure) {
            //for (size_t i = 0; i < kernel.closure.size(); i++) {
                //cout << "newItemsFromSymbolAfterDot" << endl;
                std::list<Item> newItemsFromSymbolAfterDot = closure.newItemsFromSymbolAfterDot();
                //std::list<Item> newItemsFromSymbolAfterDot = getElement(i, kernel.closure).newItemsFromSymbolAfterDot();
                for (const Item& item : newItemsFromSymbolAfterDot) {
                    
                    //cout << kernel.closure.size() << endl;
                    item.addUniqueTo(kernel.closure);
                }    
            }
        }

        bool addGotos(Kernel& kernel, std::list<Kernel>& kernels) {
            bool lookAheadsPropagated = false;
            std::map<std::string, std::list<Item>> newKernels;
            
            for (Item& item : kernel.closure) {
                std::optional<Item> newItem = item.newItemAfterShift();

                if (newItem != std::nullopt) {
                    std::string symbolAfterDot = getElement(item.dotIndex, item.rule.development);

                    addUnique(symbolAfterDot, kernel.keys);
                    getOrCreateArray(newKernels, symbolAfterDot);
                    std::list<Item>& newItems = newKernels.at(symbolAfterDot);
                    newItem.value().addUniqueTo(newItems);
                }
            }
            
            for (const std::string& key : kernel.keys) {
                Kernel newKernel(static_cast<kernels.size()>, newKernels.at(key));
                int targetKernelIndex = indexOf(newKernel, kernels);
                
                if (targetKernelIndex < 0) {
                    kernels.push_back(newKernel);
                    targetKernelIndex = newKernel.index;
                } else {
                    for (const Item& item : newKernel.items) {
                        std::list<Item>& items = getUpdateableElement(targetKernelIndex, kernels).items;
                        lookAheadsPropagated |= item.addUniqueTo(items);
                    }
                }

                if (kernel.gotos.find(key) != kernel.gotos.end()) kernel.gotos.at(key) = targetKernelIndex;
            }
            
            return lookAheadsPropagated;
        }
};

class LRAction {
    public:
        std::string actionType; // could be optimized to char
        int actionValue;

        LRAction(std::string const& actionType, int actionValue) : actionType(actionType), actionValue(actionValue) {}

        std::string toString() const {
            return actionType + std::to_string(actionValue);
        }
};

class State {
    public:
        int index;
        std::map<std::string, std::list<LRAction>> mapping;

        explicit State(std::list<State> const& states) : index(static_cast<states.size()>) {}
};

class LRTable {
    public:
        Grammar& grammar;
        std::list<State> states = {};

        explicit LRTable(LRClosureTable& closureTable) : grammar(closureTable.grammar) {
            for (const Kernel& kernel : closureTable.kernels) {
                State state(states);

                for (const std::string& key : kernel.keys) {
                    int nextStateIndex = kernel.gotos.at(key);

                    state.mapping[key].push_back(LRAction((isElement(key, grammar.terminals) ? "s" : ""), nextStateIndex));
                }

                for (const Item& item : kernel.closure) {
                    if (item.dotIndex == item.rule.development.size() || item.rule.development.front() == EPSILON) {
                        for (const std::string& lookAhead : item.lookAheads) {
                            state.mapping[lookAhead].push_back(LRAction("r", item.rule.index));
                        }
                    }
                }
                
                states.push_back(state);
            }
        }
};

std::optional<LRAction> chooseActionElement(State& state, std::string token) {
    if (state.mapping.find(token) == state.mapping.end()) {
        return std::nullopt;
    }

    std::list<LRAction> action = state.mapping.at(token);

    return action.front();
}


int main()
{
    Grammar grammar("A' -> A\nA -> a A\nA -> a");
    LRClosureTable lrClosureTable(grammar);
    assert("A'" == grammar.axiom);
    assert(3 == grammar.rules.size());
    std::list<std::string> a = {"a"};
	assert(a == grammar.firsts.at("A"));
	assert(Item(Rule(grammar, "A -> a A"), 1) == Item(Rule(grammar, "A -> a A"), 1));
	assert(0 == indexOf(Item(Rule(grammar, "A -> a A"), 1), {Item(Rule(grammar, "A -> a A"), 1)}));
	
	//cout << "actual: " << lrClosureTable.kernels.front().closure.size() << endl;
	//cout << "actual: " << lrClosureTable.kernels.size() << endl;
	assert(3 == lrClosureTable.kernels.front().closure.size());
	assert(4 == lrClosureTable.kernels.size());
    //cout << "Hello World";

    return 0;
}
