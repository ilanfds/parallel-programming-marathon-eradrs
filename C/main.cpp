#include <bits/stdc++.h>
#include <omp.h>

using namespace std;

int A[1000][1000];
int ps[1010][1010];

typedef long long ll;

// Hashmap customizado em open addressing + linear probing.
struct HMap {
    static const int CAP = 2048;
    static const int MASK = CAP - 1;
    ll keys[CAP];
    ll vals[CAP];
    bool used[CAP];

    void clear() { memset(used, 0, sizeof(used)); }

    static inline size_t hash_(ll k) {
        unsigned long long x = (unsigned long long)k;
        x ^= x >> 33;
        x *= 0xff51afd7ed558ccdULL;
        x ^= x >> 33;
        return (size_t)(x & MASK);
    }

    // Soma val ao bucket de key (insere se nao existe).
    inline void add(ll key, ll val) {
        size_t i = hash_(key);
        while (used[i]) {
            if (keys[i] == key) { vals[i] += val; return; }
            i = (i + 1) & MASK;
        }
        keys[i] = key;
        vals[i] = val;
        used[i] = true;
    }

    // Retorna o valor de key, ou 0 se nao existe.
    inline ll get(ll key) const {
        size_t i = hash_(key);
        while (used[i]) {
            if (keys[i] == key) return vals[i];
            i = (i + 1) & MASK;
        }
        return 0;
    }
};

ll solve(int m, int n, int k) {
  #pragma omp parallel for
  for (int i = 1; i <= m; i++) {
    for (int j = 1; j <= n; j++) {
      ps[i][j] = A[i - 1][j - 1] + ps[i][j - 1];
    }
  }

  ll ans = 0;
  #pragma omp parallel for reduction(+:ans) schedule(dynamic, 1)
  for (int j = 1; j <= n; j++) {
    // 1 mapa por thread (declarado dentro do parallel for = privado)
    HMap h;

    for (int l = j; l <= n; l++) {
      h.clear();
      h.add(0, 1);     // prefix vazio
      ll s = 0;

      for (int i = 1; i <= m; i++) {
        s += ps[i][l] - ps[i][j - 1];
        ans += h.get(s - k);
        h.add(s, 1);
      }
    }
  }
  return ans;
}

int main() {
  ios_base::sync_with_stdio(false);   
  cin.tie(NULL);                       
  int k, m, n;
  cin >> k >> m >> n;
  memset(A, 0, sizeof A);
  memset(ps, 0, sizeof ps);

  for (int i = 0; i < m; i++)
    for (int j = 0; j < n; j++)
      cin >> A[i][j];

  cout << solve(m, n, k) << endl;
  return 0;
}