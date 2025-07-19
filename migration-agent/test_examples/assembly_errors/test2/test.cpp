void f(int x)
{
  __asm__ ( "123456" : : "r" (x));
}
void g (void)
{
  __asm__ ("simple asm not discarded 123456");
}