/*
* @file encip.c
* @author Renat Kagal <kagal@itspartner.net>
*
* Assembling : gcc -Wall encip.c -o encip
*
* Description : Huffman algo
*
* Copyright (c) 2021, ITS Partner LLC.
* All rights reserved.
*
* This software is the confidential and proprietary information of
* ITS Partner LLC. ("Confidential Information"). You shall not
* disclose such Confidential Information and shall use it only in
* accordance with the terms of the license agreement you entered into
* with ITS Partner.
*/
#define N 16

#include "head.h"

#define SIZE_OF_READ 17
#define CHAR_SIZE 8

#define LONG_FROM_CHAR(buff, offset) (((unsigned long)((unsigned char)buff[(offset)])) | ((unsigned long)((unsigned char)buff[(offset) + 1]) << 8) | ((unsigned long)((unsigned char)buff[(offset) + 2]) << 16) | ((unsigned long)((unsigned char)buff[(offset) + 3]) << 24) | ((unsigned long)((unsigned char)buff[(offset) + 4]) << 32) | ((unsigned long)((unsigned char)buff[(offset) + 5]) << 40) | ((unsigned long)((unsigned char)buff[(offset) + 6]) << 48) | ((unsigned long)((unsigned char)buff[(offset) + 7]) << 56))

int blowfish (unsigned int add_sum, int enc_or_dec) {
   FILE*          fp_write;
   int            offset = 0;
   int            sum = 0;
   char           read_buff [SIZE_OF_READ];
   char*          key = "aaasfasfsafs";
   int            keybytes = strlen (key);
   unsigned long  xr = 0;
   unsigned long  xl = 0;
   static int flag_init = 0;

   memset (read_buff, '\0', SIZE_OF_READ);
   if (flag_init == 0) {
      blowfish_init (key, keybytes);
      flag_init = 1;
   }
   

   if (enc_or_dec == 1) {
      if ((fp_write = fopen ("encip.txt", "r")) == NULL) {
         puts ("Failed fopen");
         exit (EXIT_FAILURE);
      }

      fscanf (fp_write, "%ld%ld", &xl, &xr);
      blowfish_decipher (&xl, &xr);

      offset = 0;
      for (int j = 0; j < sizeof (unsigned long); j++, offset++) {
         read_buff[offset] = (0xff & (xl >> (j * CHAR_SIZE)));
      }
      for (int j = 0; j < sizeof (unsigned long); j++, offset++) {
         read_buff[offset] = (0xff & (xr >> (j * CHAR_SIZE)));
      }
      add_sum += atoi (read_buff);
      memset (read_buff, '\0', SIZE_OF_READ);
      sprintf (read_buff, "%d", add_sum);
      
      offset = 0;
      xl = LONG_FROM_CHAR (read_buff, offset);  
      offset += 8;
      xr = LONG_FROM_CHAR (read_buff, offset);

      blowfish_encipher (&xl, &xr);

      fclose (fp_write);

      if ((fp_write = fopen ("encip.txt", "w+")) == NULL) {
            puts ("Failed fopne");
            exit (EXIT_FAILURE);
      }
      fprintf(fp_write, "%ld %ld", xl, xr);
           
      if (fclose (fp_write)) {
         puts ("Failed fclose");
         exit (EXIT_FAILURE);
      }
      return add_sum;
   }
   else {
      if ((fp_write = fopen ("encip.txt", "r")) == NULL) {
         puts ("Failed fopen");
         exit (EXIT_FAILURE);
      }
      
      fscanf (fp_write, "%ld%ld", &xl, &xr);
      blowfish_decipher (&xl, &xr);
      
      for (int j = 0; j < sizeof (unsigned long); j++, offset++) {
         read_buff[offset] = (0xff & (xl >> (j * CHAR_SIZE)));
      }
      for (int j = 0; j < sizeof (unsigned long); j++, offset++) {
         read_buff[offset] = (0xff & (xr >> (j * CHAR_SIZE)));
      }
      //puts (read_buff);
      sum = atoi (read_buff);

      if (add_sum > sum) {
         return -1;
      }

      add_sum = sum - add_sum;

      memset (read_buff, '\0', SIZE_OF_READ);
      sprintf (read_buff, "%d", add_sum);
      //puts (read_buff);
      offset = 0;
      xl = LONG_FROM_CHAR (read_buff, offset);  
      offset += 8;
      xr = LONG_FROM_CHAR (read_buff, offset);
      
      blowfish_encipher (&xl, &xr);

      fclose (fp_write);

      if ((fp_write = fopen ("encip.txt", "w+")) == NULL) {
            puts ("Failed fopne");
            exit (EXIT_FAILURE);
      }

      fprintf(fp_write, "%ld %ld", xl, xr);

      if (fclose (fp_write)) {
         puts ("Failed fclose");
         exit (EXIT_FAILURE);
      }

      return add_sum;
   }
}

void blowfish_encipher(unsigned long *xl, unsigned long *xr) {
   unsigned long  Xl;
   unsigned long  Xr;
   unsigned long  temp;
   short          i;

   Xl = *xl;
   Xr = *xr;

   for (i = 0; i < N; ++i) {
      Xl = Xl ^ P[i];
      Xr = f(Xl) ^ Xr;

      temp = Xl;
      Xl = Xr;
      Xr = temp;
   }

   temp = Xl;
   Xl = Xr;
   Xr = temp;

   Xr = Xr ^ P[N];
   Xl = Xl ^ P[N + 1];
  
   *xl = Xl;
   *xr = Xr;
}

void blowfish_decipher(unsigned long *xl, unsigned long *xr) {
   unsigned long  Xl;
   unsigned long  Xr;
   unsigned long  temp;
   short          i;

   Xl = *xl;
   Xr = *xr;

   for (i = N + 1; i > 1; --i) {
      Xl = Xl ^ P[i];
      Xr = f(Xl) ^ Xr;

      temp = Xl;
      Xl = Xr;
      Xr = temp;
   }

   temp = Xl;
   Xl = Xr;
   Xr = temp;

   Xr = Xr ^ P[1];
   Xl = Xl ^ P[0];

   *xl = Xl;
   *xr = Xr;
}

void blowfish_init (char key[], int keybytes) {
   int               bytes_count;
   unsigned long     l;
   unsigned long     r;
   unsigned long     data;

   bytes_count = 0;

   for (int i = 0; i < N + 2; i++ ) {
      data = 0;
      for (int k = 0; k < 4; k++) {
         data = (data << 8) | key[bytes_count++];

         if (bytes_count >= keybytes) {
            bytes_count = 0;
         }
      }
      P[i] = P[i] ^ data;
   }
   l = 0;
   r = 0;

   for (int i = 0; i < N + 2; i+=2 ) {
      blowfish_encipher (&l, &r);

      l = P[i];
      r = P[i + 1];
   }

   for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 256; j+=2 ) {
         blowfish_encipher(&l, &r);
   
         S[i][j] = l;
         S[i][j + 1] = r;
      }
   }
}

unsigned long f(unsigned long x) {
   unsigned short a;
   unsigned short b;
   unsigned short c;
   unsigned short d;
   unsigned long  y;

   d = x & 0x00FF;
   x >>= 8;
   c = x & 0x00FF;
   x >>= 8;
   b = x & 0x00FF;
   x >>= 8;
   a = x & 0x00FF;
  
   y = S[0][a] + S[1][b];
   y = y ^ S[2][c];
   y = y + S[3][d];

   return y;
}