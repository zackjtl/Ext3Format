#ifndef GlobalDefH
#define GlobalDefH

#define div_ceil(x, y) ((x + y - 1) / y)
#define min_of(x, y) (x > y ? y : x)
#define max_of(x, y) (x > y ? x : y)
#define append_vect(x, y) {x.insert(x.end(), y.begin(), y.end());}

#endif
