#ifndef GlobalDefH
#define GlobalDefH

#define div_ceil(x, y) (x ? (((x -1) / y) + 1) : 0)
#define min_of(x, y) (x > y ? y : x)
#define max_of(x, y) (x > y ? x : y)
#define append_vect(x, y) {x.insert(x.end(), y.begin(), y.end());}

#endif
