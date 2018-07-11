#ifndef LERARQUI_H_INCLUDED
#define LERARQUI_H_INCLUDED

#include <conio.h>
#include <iostream>
#include <fstream>
using namespace std;

int main () {
 int length;
 char * buffer;

 ifstream is;
 is.open ("matriz2_pcv.txt", ios::in);

 is.seekg (0, ios::end);
 length = is.tellg();
 is.seekg (0, ios::beg);

 if (is.is_open())
 {
    int n = 0;
    buffer = new char[length];
    is.read(buffer,length);

   if (length > 0)
   {
      while(n < length) //isso substitue o ";" para um espaço
      {
         n++;
         if (buffer[n] == ';') buffer[n] = '\t';
      }

      printf("%s", buffer);
      getch();
      delete []buffer;
   }
 }
 else {
   printf("Arquivo não foi aberto");
   getch();
}

  is.close();
}




#endif // LERARQUI_H_INCLUDED
