#include<stdio.h>
#include <string.h>

int main() {
   char string[50] = "2222:6:3";
   char* token;

int n1,n2;
token = strtok (string, ":");
sscanf (token, "%d", &n1);
token = strtok (NULL, ":");
sscanf (token, "%d", &n2);
printf("%d\n%d\n",n1,n2);

   return 0;
}