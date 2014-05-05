/*******************************************************************************
 Copyright 2001, 2002 Georges Menie (<URL snipped>)
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/*******************************************************************************
 putchar is the only external dependency for this file, 
 if you have a working putchar, just remove the following
 define. If the function should be called something else,
 replace outbyte(c) by your own function call.
*/
//*******************************************************************************
//  Updated by Daniel D Miller.  Changes to the original Menie code are
//  Copyright 2009-2010 Daniel D Miller
//  All such changes are distributed under the same license as the original,
//  as described above.
//  11/06/09 11:42 - adding floating-point support
//  03/19/10 19:06 - pad fractional portion of floating-point number with 0s
//  03/30/10 14:57 - document the \% bug
//*******************************************************************************
//  BUGS
//  If '%' is included in a format string, followed by a non-format character
//  (typically because the user desires to place the % char in the output string),
//  this function will mis-handle the data entirely!!  
//  Essentially, it will just discard the character following the percent sign.  
//  This bug is not easy to fix in the existing code; 
//  for now, I'll just try to remember to use %% instead of % for normal output.
//*******************************************************************************

// #define  TEST_PRINTF    1

#if 1
#include "kernel.h"
#undef  USE_INTERNALS
#else
#ifdef TEST_PRINTF
#include <stdio.h>
#undef  USE_INTERNALS
#else
//  USE_INTERNALS implements code for company-specific linkages
//  at the company that I currently work for.
//  Other users will *not* want this defined!!
#define  USE_INTERNALS
#endif
#endif

#ifdef   USE_INTERNALS
#include "91x_lib.h"
#include "common.h"
#else
typedef  unsigned int  uint ;
#endif

//****************************************************************************
static void printchar (char **str, int c)
{
	if (str) {
		**str = c;
		++(*str);
	}
//#ifdef TEST_PRINTF
#if 1
   else {
      extern int putchar (int c);
		(void) putchar (c);
   }
#endif
}

//****************************************************************************
uint my_strlen(char *str)
{
   if (str == 0)
      return 0;
   uint slen = 0 ;
   while (*str != 0) {
      slen++ ;
      str++ ;
   }
   return slen;
}

//****************************************************************************
//  This version returns the length of the output string.
//  It is more useful when implementing a walking-string function.
//****************************************************************************
unsigned dbl2stri(char *outbfr, double dbl, unsigned dec_digits)
{
   static char local_bfr[81] ;
   char *output = (outbfr == 0) ? local_bfr : outbfr ;

   //*******************************************
   //  extract negative info
   //*******************************************
   int negative = (dbl < 0.0) ? 1 : 0 ;
   if (negative) {
      *output++ = '-' ;
      dbl *= -1.0 ;
   }

   //**************************************************************************
   //  construct fractional multiplier for specified number of digits.
   //  Note that we draw one digit more than requested in order
   //  to handle last-digit rounding.
   //**************************************************************************
   uint mult = 10 ;
   uint idx ;
   for (idx=0; idx < dec_digits; idx++)
      mult *= 10 ;

   // printf("mult=%u\n", mult) ;
   int wholeNum = (int) dbl ;
   int decimalNum = (int) ((dbl - wholeNum) * mult);
   //  round off low digit
   decimalNum += 5 ;
   decimalNum /= 10 ;

   //*******************************************
   //  convert integer portion
   //*******************************************
   char tbfr[40] ;
   idx = 0 ;
   while (wholeNum != 0) {
      tbfr[idx++] = '0' + (wholeNum % 10) ;
      wholeNum /= 10 ;
   }
   // printf("%.3f: whole=%s, dec=%d\n", dbl, tbfr, decimalNum) ;
   if (idx == 0) {
      *output++ = '0' ;
   } else {
      while (idx > 0) {
         *output++ = tbfr[idx-1] ;  //lint !e771
         idx-- ;
      }
   }
   if (dec_digits > 0) {
      *output++ = '.' ;

      //*******************************************
      //  convert fractional portion
      //*******************************************
      idx = 0 ;
      while (decimalNum != 0) {
         tbfr[idx++] = '0' + (decimalNum % 10) ;
         decimalNum /= 10 ;
      }
      //  pad the decimal portion with 0s as necessary;
      //  We wouldn't want to report 3.093 as 3.93, would we??
      while (idx < dec_digits) {
         tbfr[idx++] = '0' ;
      }
      // printf("decimal=%s\n", tbfr) ;
      if (idx == 0) {
         *output++ = '0' ;
      } else {
         while (idx > 0) {
            *output++ = tbfr[idx-1] ;
            idx-- ;
         }
      }
   }
   *output = 0 ;

   //  prepare output
   output = (outbfr == 0) ? local_bfr : outbfr ;
   return my_strlen(output) ;
}

//****************************************************************************
#define  PAD_RIGHT   1
#define  PAD_ZERO    2

static int prints (char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';
	if (width > 0) {
      int len = 0;
      const char *ptr;
		for (ptr = string; *ptr; ++ptr)
			++len;
		if (len >= width)
			width = 0;
		else
			width -= len;
		if (pad & PAD_ZERO)
			padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for (; width > 0; --width) {
			printchar (out, padchar);
			++pc;
		}
	}
	for (; *string; ++string) {
		printchar (out, *string);
		++pc;
	}
	for (; width > 0; --width) {
		printchar (out, padchar);
		++pc;
	}
	return pc;
}

//****************************************************************************
/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12
static int printi (char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
   char *s;
   int t, neg = 0, pc = 0;
   unsigned u = (unsigned) i;
	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}
	if (sg && b == 10 && i < 0) {
		neg = 1;
      u = (unsigned) -i;
	}
	s = print_buf + PRINT_BUF_LEN - 1;
	*s = '\0';
	while (u) {
      t = u % b;  //lint !e573  Warning 573: Signed-unsigned mix with divide
		if (t >= 10)
			t += letbase - '0' - 10;
		*--s = t + '0';
      u /= b;  //lint !e573  Warning 573: Signed-unsigned mix with divide
	}
	if (neg) {
		if (width && (pad & PAD_ZERO)) {
			printchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}
	return pc + prints (out, s, width, pad);
}

//****************************************************************************
/* the following should be enough for 32 bit int */
#define PRINTL_BUF_LEN 24
static int printl (char **out, long long i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINTL_BUF_LEN];
	char *s;
	long long t;
	int neg = 0, pc = 0;
	unsigned long long u = (unsigned long long) i;
	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}
	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = (unsigned long long) -i;
	}
	s = print_buf + PRINTL_BUF_LEN - 1;
	*s = '\0';
	while (u) {
		t = u % b;  //lint !e573  Warning 573: Signed-unsigned mix with divide
		if (t >= 10)
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;  //lint !e573  Warning 573: Signed-unsigned mix with divide
	}
	if (neg) {
		if (width && (pad & PAD_ZERO)) {
			printchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}
	return pc + prints (out, s, width, pad);
}

//****************************************************************************
static int print (char **out, int *varg)
{
   int post_decimal ;
   int width, pad ;
   unsigned dec_width = 6 ;
   int pc = 0;
   char *format = (char *) (*varg++);
	char scr[2];
	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0')
				break;
			if (*format == '%')
				goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
         post_decimal = 0 ;
         if (*format == '.'  ||
            (*format >= '0' &&  *format <= '9')) {

            while (1) {
               if (*format == '.') {
                  post_decimal = 1 ;
                  dec_width = 0 ;
                  format++ ;
               } else if ((*format >= '0' &&  *format <= '9')) {
                  if (post_decimal) {
                     dec_width *= 10;
                     dec_width += *format - '0';
                  } else {
                     width *= 10;
                     width += *format - '0';
                  }
                  format++ ;
               } else {
                  break;
               }
            }
         }
	 int ll = 0;
	 if (*format == 'l') {
            ++format;
	    if (*format == 'l') {
	       ++format;
	       ll = 1;
	    }
	 }
         switch (*format) {
         case 's':
            {
            char *s = *((char **) varg++);   //lint !e740
            // printf("[%s] w=%u\n", s, width) ;
            pc += prints (out, s ? s : "(null)", width, pad);
            }
            break;
         case 'd':
	    if (ll) {
	       if (((unsigned int)varg & 7) != 0) varg++; // MIPS requires 8 byte aligned long longs
	       long long *llptr = (long long *) varg;
	       long long llval = *llptr++;
	       varg = (int *) llptr;
	       pc += printl (out, llval, 10, 1, width, pad, 'a');
	    } else {
	       pc += printi (out, *varg++, 10, 1, width, pad, 'a');
	    }
            break;
         case 'x':
	    if (ll) {
	       if (((unsigned int)varg & 7) != 0) varg++; // MIPS requires 8 byte aligned long longs
	       long long *llptr = (long long *) varg;
	       long long llval = *llptr++;
	       varg = (int *) llptr;
	       pc += printl (out, llval, 16, 0, width, pad, 'a');
	    } else {
	       pc += printi (out, *varg++, 16, 0, width, pad, 'a');
	    }
            break;
         case 'p':
	    pc += prints (out, "0x", 0, 0);
            pc += printi (out, *varg++, 16, 0, width ? width : 8, pad ? pad : PAD_ZERO , 'a');
            break;
         case 'X':
	    if (ll) {
	       if (((unsigned int)varg & 7) != 0) varg++; // MIPS requires 8 byte aligned long longs
	       long long *llptr = (long long *) varg;
	       long long llval = *llptr++;
	       varg = (int *) llptr;
	       pc += printl (out, llval, 16, 0, width, pad, 'A');
	    } else {
	       pc += printi (out, *varg++, 16, 0, width, pad, 'A');
	    }
            break;
         case 'u':
	    if (ll) {
	       if (((unsigned int)varg & 7) != 0) varg++; // MIPS requires 8 byte aligned long longs
	       long long *llptr = (long long *) varg;
	       long long llval = *llptr++;
	       varg = (int *) llptr;
	       pc += printl (out, llval, 10, 0, width, pad, 'a');
	    } else {
	       pc += printi (out, *varg++, 10, 0, width, pad, 'a');
	    }
            break;
         case 'c':
            /* char are converted to int then pushed on the stack */
            scr[0] = *varg++;
            scr[1] = '\0';
            pc += prints (out, scr, width, pad);
            break;

         case 'f':
            {
	    if (((unsigned int)varg & 7) != 0) varg++; // MIPS requires 8 byte aligned doubles
            double *dblptr = (double *) varg ;  //lint !e740 !e826  convert to double pointer
            double dbl = *dblptr++ ;   //  increment double pointer
            varg = (int *) dblptr ;    //lint !e740  copy updated pointer back to base pointer
            char bfr[81] ;
            // unsigned slen =
            dbl2stri(bfr, dbl, dec_width) ;
            // stuff_talkf("[%s], width=%u, dec_width=%u\n", bfr, width, dec_width) ;
            pc += prints (out, bfr, width, pad);
            }
            break;

         default:
            printchar (out, '%');
            printchar (out, *format);
            break;
         }
		}
		else {
		 out:
			printchar (out, *format);
			++pc;
		}
	}
	if (out)
		**out = '\0';
	return pc;
}

//****************************************************************************
//  assuming sizeof(void *) == sizeof(int)
//  This function is not valid in embedded projects which don't
//  have a meaningful stdout device.
//****************************************************************************
int termf (const char *format, ...)
{
#ifdef TEST_PRINTF
   int *varg = (int *) (char *) (&format);
   return print (0, varg);
#else
   return 0;
#endif
}  //lint !e715


//****************************************************************************
int stringf (char *out, const char *format, ...)
{
   //  create a pointer into the stack.
   //  Thematically this should be a void*, since the actual usage of the
   //  pointer will vary.  However, int* works fine too.
   //  Either way, the called function will need to re-cast the pointer
   //  for any type which isn't sizeof(int)
   int *varg = (int *) (char *) (&format);
	return print (&out, varg);
}

int sprintf (char *out, const char *format, ...)
{
   //  create a pointer into the stack.
   //  Thematically this should be a void*, since the actual usage of the
   //  pointer will vary.  However, int* works fine too.
   //  Either way, the called function will need to re-cast the pointer
   //  for any type which isn't sizeof(int)
   int *varg = (int *) (char *) (&format);
	return print (&out, varg);
}

//********************************************************************
//  On Windows platform, try to redefine printf/fprintf
//  so we can output code to a debug window.
//  Also, shadow syslog() within OutputDebugStringA()
//  Note: printf() remapping was unreliable,
//  but syslog worked great.
//********************************************************************
#ifdef   USE_INTERNALS
int stuff_talkf(const char *format, ...)
{
   char consoleBuffer[TALK_LINE_LEN+1] ;
   char *out = consoleBuffer ;
   int *varg = (int *) (char *) (&format);
   print (&out, varg);
   stuff_talk(consoleBuffer) ;
   return 1;
}
#endif

//****************************************************************************
#ifdef TEST_PRINTF
int main (void)
{
   int slen ;
   char *ptr = "Hello world!";
   char *np = 0;
   char buf[128];
   termf ("ptr=%s, %s is null pointer, char %c='a'\n", ptr, np, 'a');
   termf ("hex %x = ff, hex02=%02x\n", 0xff, 0);   //  hex handling
   termf ("signed %d = unsigned %u = hex %X\n", -3, -3, -3);   //  int formats
   termf ("%d %s(s) with %%\n", 0, "message");

   slen = stringf (buf, "justif: left=\"%-10s\", right=\"%10s\"\n", "left", "right");
   termf ("[%d] %s", slen, buf);
   
   stringf(buf, "padding (pos): zero=[%04d], left=[%-4d], r=[%4d]\n", 3, 3, 3) ;
   termf ("%s", buf);

   //  test walking string builder
   slen = 0 ;
   slen += stringf(buf+slen, "padding (neg): zero=[%04d], ", -3) ;
   slen += stringf(buf+slen, "left=[%-4d], ", -3) ;
   slen += stringf(buf+slen, "right=[%4d]\n", -3) ;
   termf ("[%d] %s", slen, buf);

   stringf (buf, "%.2f is a double\n", 3.31) ;
   termf ("%s", buf);

   stringf (buf, "multiple unsigneds: %u %u %2u %X\n", 15, 5, 23, 0xB38F) ;
   termf ("%s", buf);

   stringf (buf, "multiple chars: %c %c %c %c\n", 'a', 'b', 'c', 'd') ;
   termf ("%s", buf);

   stringf (buf, "multiple doubles: %.2f %2.0f %.2f %.3f %.2f [%-8.3f]\n",
                  3.31, 2.45, -1.1, 3.093, 13.72, -4.382) ;
   termf ("%s", buf);

   return 0;
}
#endif

#include "kernel.h"

int printf_u (const char *format, ...)
{
  int *varg = (int *) (char *) (&format);
  return print (0, varg);
}

int printf_i (const char *format, ...)
{
  int *varg = (int *) (char *) (&format);
  int level = intr_disable();
  int ret = print (0, varg);
  intr_restore(level);
  return ret;
}

/*
static int printf_mutex = 0;
int printf_m (const char *format, ...)
{
  int *varg = (int *) (char *) (&format);
  mutex_lock(&printf_mutex);
  int ret = print (0, varg);
  mutex_unlock(&printf_mutex);
  return ret;
}

int printf_im (const char *format, ...)
{
  int *varg = (int *) (char *) (&format);
  int level = intr_disable();
  mutex_lock(&printf_mutex);
  int ret = print (0, varg);
  mutex_unlock(&printf_mutex);
  intr_restore(level);
  return ret;
}

int printf (const char *format, ...)
{
  int *varg = (int *) (char *) (&format);
  int level = intr_disable();
  mutex_lock(&printf_mutex);
  int ret = print (0, varg);
  mutex_unlock(&printf_mutex);
  intr_restore(level);
  return ret;
}

*/

// default printf is the same as printf_i, for now
int printf (const char *format, ...)
{
  int *varg = (int *) (char *) (&format);
  int level = intr_disable();
  int ret = print (0, varg);
  intr_restore(level);
  return ret;
}
