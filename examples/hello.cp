// This is single line comment

/* This is block comment that can 
   span across multiple lines.   */

main() {
  i32 a = 10;
  i32 b = 12;
  i32 c = 1+a*b+2;

  printf("Hello before scope!\n");
  {
    printf("Hello inside scope!\n");
  }
  printf("Hello after scope!\n");
}
