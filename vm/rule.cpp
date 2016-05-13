#include <map>
#include <unordered_map>
#include <functional>
#include <cstring>
#include <iostream>
#include <vector>
using namespace std;

typedef const char* pcch;
typedef map<pcch, struct term*> env;
typedef const struct term* pcterm;

struct term {
	pcch p;
	term **args;
	uint sz;
	const bool var, list;
	term(pcch p) 
		: p(strdup(p)), args(0)
		, sz(0), var(p && *p == '?'), list(false) {}
	term(pcch s, pcch _p, pcch o)
		: p(0), args(new term*[3])
		, sz(3), var(false), list(true) {
		args[0] = new term(s);
		args[1] = new term(_p);
		args[2] = new term(o);
	}
	term(const vector<term*>& v)
		: p(0), args(new term*[v.size()])
		, sz(v.size()), var(false), list(true) {
		uint n = 0;
		for (auto x : v) args[n++] = x;
	}
	term(const vector<pcch>& v) 
		: p(0), args(new term*[v.size()])
		, sz(v.size()), var(false), list(true) {
		uint n = 0;
		for (auto x : v) args[n++] = new term(x);
	}
	term(term *t)
		: p(0), args(new term*[1])
		, sz(1), var(false), list(true) { *args = t; }
};

template<typename T>
struct dsds {
	unordered_map<T, T> w;
	T rep(T t) {
		auto it = w.find(t);
		return it == w.end() ? t : (w[t] = rep(it->second));
	}
	bool rep(T a, T b, function<bool(T, T)> p) {
		T ra = rep(a), rb = rep(b);
		return p(a, b) && (w[a] = w[ra] = rb, true);
	}
};

class rule {
	term *h;
	typedef dsds<pcterm> match;
	typedef map<rule*, match> premise;
	vector<premise> b;
	map<term*, term*> vars;
	term* get(term *x) {
		if (!x->var) return x;
		auto it = vars.find(x);
		if (it != vars.end()) return it->second;
		return vars[x];// = fresh(x);
	}
	bool put(match& m, term *s, term *d) {
		return m.rep(s, d, [this] (pcterm s, pcterm d) {
			return	(s->list && d->list)
				? (s->sz == d->sz
				&& add(s->args, d->args, s->sz))
				: (s->var || d->var);
		});
	}
public:
	rule(){}
	bool add(term **x, term **y, uint sz) {
		while (--sz)
			for (auto p : b) {
				for (auto m : p)
					if (!put(m.second, get(x[sz]), get(y[sz])))
						p.erase(m.first);
				if (p.empty()) return false;
			}
		return true;
	}
} *rules;

#ifdef PREV
#define ifnot(x) if (!(x))
#define ifnull(x) if (!(x))

bool prove(rule *r, uint p) {
	if (p == r->b.size()) return true;
	rule *h;
	bool f = false;
	for (auto m : r->b[p]) {
		env e;
		ifnull (h = rules[m.r]->mut(m.first, m.second, e)) continue;
		f &= 	prove(h, 0) &&
			prove(rules[m.r]->mut(m.first, m.second, e), p + 1);
	}
	return f;
}

namespace ir {
typedef function<void(env)> rule;
inline term* rep(term* x, env& e) {
	if (!x || !x->var) return x;
	env::iterator it = e.find(x->p);
	return it == e.end() ? x : (e[x->p] = rep(it->second, e));
}

const rule* compile(match& m, const rule *t, const rule *f) {
	auto _s = c->x, _d = c->y;
	return &(*new rule = [_s, _d, t, f](env e) {
		auto s = rep(_s, e), d = rep(_d, e);
		const rule *r;
		rule tt;
		uint sz;
		if (!_s || !_d) { if (!_s && !_d) goto pass; else goto fail; }
		if (s->list && d->list) goto lists;
		if (s->var) goto svar;
		if (d->var) goto dvar;
		if (s->p && d->p && !strcmp(s->p, d->p)) goto pass;
	fail:	(*f)(e); return;
	pass:	(*t)(e); return;
	svar: 	//rule tt = [s, d, t, f](env e) {
//			term *rd = rep(d, e);
//			if (!rd->var) (*compile(rep(s, e), rep(d, e), t, f))();
//		};
		if (!d->var) es[s->p] = d;
		goto pass;
	dvar:	ed[d->p] = s;
		goto pass;
	lists:	sz = s->sz;
		if (sz != d->sz) goto fail;
		r = t;
		while (sz--) r = compile(rep(s->args[sz], e), rep(d->args[sz], e), r, f);
		(*r)(e);
	});
}
}
ostream& operator<<(ostream& os, const term& t) {
	if (!t.sz) return os << t.p;
	os << '(';
	for (uint n = 0; n < t.sz; ++n) os << *t.args[n] << ' ';
	return os << ')';
}

ostream& operator<<(ostream& os, const env& e) {
	for (auto x : e) os << x.first << '=' << (x.second ? *x.second : "(null)") << ';'; return os;
}

int main() {
	term *x = new term("x", "p", "o");
	term *z = new term("?x", "p", "?x");
//	term *x = new term( vector<pcch>{ "?x" });
//	term *y = new term( { new term("x"), new term("?y"), new term("?ro") } );
//	term *z = new term( { y } );
//	term *y = new term(x);
	rule *t = new rule, *f = new rule;
	*t = [](env e) {
//	 	rule tt = [s, d, t, f](env e) {
//			term *rd = rep(d, e);
//			if (!rd->var) (*compile(rep(s, e), rep(d, e), t, f))();
//		};
		cout << "true " << e << endl;
	};
	*f = [](env e) { cout << "false " << e << endl; };
	(*compile(z, x, t, f))(env());
//	(*compile(x, z, t, f))(env());

	return 0;
}
#endif
