/*
   1. Understand the following code.
   2. Print a character n times where the character and n are passed as command-line arguments.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void usage(void);

int main(int argc, char **argv)
{
  int num = 2;
  char disp = '?';
  char c;

  while ((c = getopt(argc, argv, "n:d:")) != -1)
  {

    switch (c)
    {

    case 'n':
      num = atoi(optarg);
      break;

    case 'd':
      disp = optarg[0];
      break;

    default:
      usage();
      exit(1);
    }
  }
  
  printf("number = %d, display char = %c\n", num, disp);
  while (num--)
  {
    printf("%c\n", disp);
  }
  return 0;
}

void usage(void)
{
  printf("Usage:\n");
  printf(" -n <repetitions>\n");
  printf(" -d <char to dispaly>\n");
  exit(8);
}
