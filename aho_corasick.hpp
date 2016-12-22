#ifndef AHO_CORASICK_HPP
#define AHO_CORASICK_HPP

#include <vector>
#include <memory>
#include <algorithm>
#include <map>
#include <list>
#include <deque>
#include <assert.h>

/* Algoritm for simultaneous
 * pattern matching in O(n + m),
 * where n - text length, m - patterns length.
 * A.V.Aho and M.Corasick. Efficient string matching.
 * CACM, 18(6):333-340, June 1975.
 */
template <typename T>
class AhoCorasick
{
public:
    typedef typename T::value_type value_type;
    typedef T pattern_type;

    /* User-space class to call match().
     */
    class Matcher
    {
        Matcher(AhoCorasick& automation);
        
    public:
        Matcher(const Matcher&) = default;
        ~Matcher() = default;
        Matcher& operator=(const Matcher&) = default;

        /* find all matches in [begin, end)
         * f - function(pattern_type, Iterator)
         * that is called on every match.
         * begin - iterator to begin
         * end - iterator to end
         */
        template <typename F, typename Iterator>
        void match(F f, Iterator begin, const Iterator& end);

    private:
        AhoCorasick& m_automation;

        friend class AhoCorasick<T>;

    }; // class AhoCorasick::Matcher

private:
    class State
    {
    public:
        typedef typename std::map<value_type, std::unique_ptr<State> > container;
        typedef typename container::iterator iterator;

    public:
        State();
        State(value_type label, const State* parent);
        State(const State&) = delete;
        ~State() = default;
        State& operator=(const State&) = delete;

        bool isRoot() const;
        value_type label() const;
        const State* parent() const;
        State* next(value_type next_label);
        template <typename F, typename Iterator>
        bool applyPatterns(F& f, const Iterator& i) const;
        
    private:
        State* addNext(value_type label);
        void setFailTransition(State* state);
        void addPattern(const pattern_type* pattern);

        value_type m_label;
        const State* m_parent;
        container m_next;
        State* m_failTransition;
        std::vector<const pattern_type*> m_patterns;

        friend class AhoCorasick<T>;

    }; // class AhoCorasick::State

public:
    AhoCorasick();
    AhoCorasick(const AhoCorasick&) = delete;
    ~AhoCorasick() = default;
    AhoCorasick& operator=(const AhoCorasick&) = delete;

    /* add pattern to match against.
     */
    AhoCorasick& addPattern(const pattern_type& pattern);
    /* get matcher that has match().
     */
    Matcher matcher();
    
private:
    template <typename Iterator>
    State* updateChain(const Iterator& current,
                       const Iterator& end, State* state);
    template <typename Iterator>
    State* findState(const Iterator& current,
                     const Iterator& end, State* state);
    void prepare();
    size_t prepareRecurcive(State* state,
                            size_t lvl, size_t max_lvl);
    void prepareState(State* state);
    void failSuffix(const State* state,
                    std::deque<value_type>& suffix);
    template <typename F, typename Iterator>
    void match(F& f, Iterator begin, const Iterator& end);

private:
    std::list<pattern_type> m_patterns;
    State m_root;
    bool m_prepaired;

}; // class AhoCorasick

template <typename T>
AhoCorasick<T>::Matcher::Matcher(AhoCorasick& automation)
: m_automation(automation)
{
    m_automation.prepare();
}

template <typename T>
template <typename F, typename Iterator>
void AhoCorasick<T>::Matcher::match(F f, Iterator begin, const Iterator& end)
{
    m_automation.match(f, begin, end);
}

template <typename T>
AhoCorasick<T>::State::State()
: m_parent(nullptr)
, m_failTransition(this)
{}

template <typename T>
AhoCorasick<T>::State::State(value_type label, const State* parent)
: m_label(label)
, m_parent(parent)
, m_failTransition(nullptr)
{}

template <typename T>
bool AhoCorasick<T>::State::isRoot() const
{
    return m_parent == nullptr;
}

template <typename T>
typename AhoCorasick<T>::value_type
AhoCorasick<T>::State::label() const
{
    return m_label;
}

template <typename T>
const typename AhoCorasick<T>::State*
AhoCorasick<T>::State::parent() const
{
    return m_parent;
}

template <typename T>
typename AhoCorasick<T>::State*
AhoCorasick<T>::State::next(value_type next_label)
{
    auto state = m_next.find(next_label);
    if (state != std::end(m_next))
    {
        return state->second.get();
    }
    else
    {
        if (isRoot())
            return this;
        return m_failTransition->next(next_label);
    }
}

template <typename T>
template <typename F, typename Iterator>
bool AhoCorasick<T>::State::applyPatterns(F& f, const Iterator& i) const
{
    if (isRoot())
        return false;
    for (auto pattern : m_patterns)
        if (f(*pattern, std::prev(i, pattern->size() - 1)))
            return true;
    return false;
}

template <typename T>
typename AhoCorasick<T>::State*
AhoCorasick<T>::State::addNext(AhoCorasick<T>::value_type label)
{
    return m_next.emplace(label,
                          std::unique_ptr<State>(new State{label, this}))
    .first->second.get();
}

template <typename T>
void AhoCorasick<T>::State::setFailTransition(State* state)
{
    m_failTransition = state;
}

template <typename T>
void AhoCorasick<T>::State::addPattern(const AhoCorasick<T>::pattern_type* pattern)
{
    m_patterns.push_back(pattern);
}

template <typename T>
AhoCorasick<T>::AhoCorasick()
: m_prepaired(false)
{}

template <typename T>
AhoCorasick<T>& AhoCorasick<T>::addPattern(const AhoCorasick<T>::pattern_type& pattern)
{
    auto state = updateChain(std::begin(pattern), std::end(pattern), &m_root);
    if (state->isRoot())
        return *this;
    state->addPattern(&(*m_patterns.emplace(std::end(m_patterns), pattern)));
    m_prepaired = false;
    return *this;
}

template <typename T>
typename AhoCorasick<T>::Matcher AhoCorasick<T>::matcher()
{
    return Matcher(*this);
}

template <typename T>
template <typename Iterator>
typename AhoCorasick<T>::State*
AhoCorasick<T>::updateChain(const Iterator& current,
                            const Iterator& end, State* state)
{
    if (current == end)
        return state;
    auto next = state->m_next.find(*current);
    if (next == std::end(state->m_next))
        return updateChain(std::next(current), end, state->addNext(*current));
    else
        return updateChain(std::next(current), end, next->second.get());
}

template <typename T>
template <typename Iterator>
typename AhoCorasick<T>::State*
AhoCorasick<T>::findState(const Iterator& current,
                          const Iterator& end, State* state)
{
    if (current == end)
        return state;
    auto next = state->m_next.find(*current);
    if (next != std::end(state->m_next))
        return findState(std::next(current), end, next->second.get());
    else
        return nullptr;
}

template <typename T>
void AhoCorasick<T>::prepare()
{
    if (m_prepaired)
        return;
    size_t max_lvl = 1;
    while(prepareRecurcive(&m_root, 0, max_lvl) > 0)
        ++max_lvl;
    m_prepaired = true;
}

template <typename T>
size_t AhoCorasick<T>::prepareRecurcive(State* state,
                                        size_t lvl, size_t max_lvl)
{
    if (lvl == max_lvl)
    {
        prepareState(state);
        return 1;
    }
    size_t prepared = 0;
    for (auto& next : state->m_next)
        prepared += prepareRecurcive(next.second.get(), lvl + 1, max_lvl);
    return prepared;
}

template <typename T>
void AhoCorasick<T>::prepareState(State* state)
{
    if (state->isRoot())
        return;
    if (state->parent()->isRoot())
    {
        state->setFailTransition(&m_root);
        return;
    }
    std::deque<value_type> suffix;
    failSuffix(state, suffix);
    while (!suffix.empty())
    {
        State* transition = findState(std::begin(suffix),
                                      std::end(suffix), &m_root);
        if (transition != nullptr)
        {
            state->setFailTransition(transition);
            for (auto pattern : transition->m_patterns)
                state->addPattern(pattern);
            break;
        }
        suffix.pop_front();
    }
    if (suffix.empty())
        state->setFailTransition(&m_root);
}

template <typename T>
void AhoCorasick<T>::failSuffix(const State* state,
                                std::deque<value_type>& suffix)
{
    if (state->parent()->isRoot())
        return;
    suffix.emplace_front(state->label());
    failSuffix(state->parent(), suffix);
}

template <typename T>
template <typename F, typename Iterator>
void AhoCorasick<T>::match(F& f, Iterator begin, const Iterator& end)
{
    assert(m_prepaired);
    State* state = &m_root;
    for (; begin != end; ++begin)
    {
        state = state->next(*begin);
        if (state->applyPatterns(f, begin))
            break;
    }
}

#endif // AHO_CORASICK_HPP
