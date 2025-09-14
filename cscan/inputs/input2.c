#ifdef DEFAULT_CLANG
int print(const char *fmt, ...);
#endif

/* 
 * author: Michael Jantz
 *
 * this simple program initializes and prints a couple integers and a couple
 * double values
 */

int main()
{
  int a, c;
  double b, d;

  // initialize
  a = 3;
  b = 4;
  c = 5;
  d = 6;

  // print
  print("%d %3.2f %d %3.2f\n", a, b, c, d);
  return 0;
}
