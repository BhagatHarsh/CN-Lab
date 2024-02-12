#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{

     /* Pointer to the source file */
     FILE *src, *des;
     /* File is read one character at a time*/
     char c;
     char input_file[100] = "sample.txt";
     char output_file[100] = "sample_out.txt";

     while ((c = getopt(argc, argv, "i:o:")) != -1)
     {
          switch (c)
          {
          case 'i':
               strcpy(input_file, optarg);
               break;
          case 'o':
               strcpy(output_file, optarg);
               break;
          default:
               printf("Usage: %s -i <input_file> -o <output_file>\n", argv[0]);
               exit(EXIT_FAILURE);
          }
     }
     src = fopen(input_file, "r");
     des = fopen(output_file, "w");
     if (src == NULL)
     {
          printf("Source file not found. Exiting.\n");
          exit(EXIT_FAILURE);
     }
     if(des == NULL)
     {
          printf("Destination file not found. Exiting.\n");
          exit(EXIT_FAILURE);
     }
     /* Read src until end-of-file char is encountered */
     while ((c = fgetc(src)) != EOF)
     {
          fputc(c, des);
     }
     fclose(src);
     fclose(des);
     return 0;
}
