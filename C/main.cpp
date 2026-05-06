#include <bits/stdc++.h>

using namespace std;

int A[1000][1000];
int ps[1010][1010];

typedef long long ll;

ll solve(int m, int n, int k) {
  for (int i = 1; i <= m; i++) {
    for (int j = 1; j <= n; j++) {
      ps[i][j] = A[i - 1][j - 1] + ps[i][j - 1];
    }
  }
  unordered_map<ll, ll> h;
  ll ans = 0;
  for (int j = 1; j <= n; j++) // O(m*n^2)
    for (int l = j; l <= n; l++) {
      h.clear();
      h[0] = 1;
      ll s = 0;
      for (int i = 1; i <= m; i++) {
        s += ps[i][l] - ps[i][j - 1];
        ans += h.find(s - k) != h.end() ? h[s - k] : 0;
        h[s]++;
      }
    }
  return ans;
}

int main() {
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
