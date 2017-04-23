/* testDragpack100() */
/* Rip_Dragpack100() */

#include "globals.h"
#include "extern.h"


int16_t	 testDragpack100 ( void )
{
  PW_Start_Address = PW_i;

  if ( (in_data[PW_Start_Address+16] != 0x20 ) ||
       (in_data[PW_Start_Address+17] != 0x18 ) ||
       (in_data[PW_Start_Address+18] != 0x22 ) ||
       (in_data[PW_Start_Address+19] != 0x18 ) ||
       (in_data[PW_Start_Address+20] != 0x2A ) ||
       (in_data[PW_Start_Address+21] != 0x18 ) ||
       (in_data[PW_Start_Address+22] != 0x24 ) ||
       (in_data[PW_Start_Address+23] != 0x49 ) ||
       (in_data[PW_Start_Address+24] != 0xD1 ) ||
       (in_data[PW_Start_Address+25] != 0xC0 ) ||
       (in_data[PW_Start_Address+26] != 0xD5 ) ||
       (in_data[PW_Start_Address+27] != 0xC1 ) )
  {
    /* should be enough :))) */
/*printf ( "#2 Start:%ld\n" , PW_Start_Address );*/
    return BAD;
    
  }


  /* packed size */
  PW_l = ( (in_data[PW_Start_Address+0x2ec]*256*256*256) +
           (in_data[PW_Start_Address+0x2ed]*256*256) +
           (in_data[PW_Start_Address+0x2ee]*256) +
           in_data[PW_Start_Address+0x2ef] );

  PW_l += 0x330;

  if ( PW_i >= 32 )
  {
    if ( (in_data[PW_Start_Address-40]  != 0x00 ) ||
         (in_data[PW_Start_Address-39]  != 0x00 ) ||
         (in_data[PW_Start_Address-38]  != 0x03 ) ||
         (in_data[PW_Start_Address-37]  != 0xF3 ) ||
         (in_data[PW_Start_Address-36]  != 0x00 ) ||
         (in_data[PW_Start_Address-35]  != 0x00 ) ||
         (in_data[PW_Start_Address-34]  != 0x00 ) ||
         (in_data[PW_Start_Address-33]  != 0x00 ) ||
         (in_data[PW_Start_Address-32]  != 0x00 ) ||
         (in_data[PW_Start_Address-31]  != 0x00 ) ||
         (in_data[PW_Start_Address-30]  != 0x00 ) ||
         (in_data[PW_Start_Address-29]  != 0x03 ) ||
         (in_data[PW_Start_Address-28]  != 0x00 ) ||
         (in_data[PW_Start_Address-27]  != 0x00 ) ||
         (in_data[PW_Start_Address-26]  != 0x00 ) ||
         (in_data[PW_Start_Address-25]  != 0x00 ) )
    {
      Amiga_EXE_Header = BAD;
    }
    else
      Amiga_EXE_Header = GOOD;
  }
  else
    Amiga_EXE_Header = BAD;

  return GOOD;
  /* PW_l is the size of the pack */
}


void Rip_Dragpack100 ( void )
{
  /* PW_l is still the whole size */

  uint8_t * Amiga_EXE_Header_Block;
  uint8_t * Whatever;

  OutputSize = PW_l;

  CONVERT = BAD;

  if ( Amiga_EXE_Header == BAD )
  {
    OutputSize -= 40;
    Amiga_EXE_Header_Block = (uint8_t *) malloc ( 40 );
    BZERO ( Amiga_EXE_Header_Block , 40 );
    Amiga_EXE_Header_Block[2]  = 0x03;
    Amiga_EXE_Header_Block[3]  = 0xF3;
    Amiga_EXE_Header_Block[11] = 0x03;
    Amiga_EXE_Header_Block[19] = 0x02;
    Amiga_EXE_Header_Block[23] = 0xA2;
    Amiga_EXE_Header_Block[34] = 0x03;
    Amiga_EXE_Header_Block[35] = 0xE9;
    Amiga_EXE_Header_Block[39] = 0xA2;

    /* WARNING !!! WORKS ONLY ON PC !!!       */
    /* 68k machines code : c1 = *(Whatever+2); */
    /* 68k machines code : c2 = *(Whatever+3); */
    PW_j = PW_l - 804;
    PW_j /= 4;
    Whatever = (uint8_t *) &PW_j;
    Amiga_EXE_Header_Block[24] = Amiga_EXE_Header_Block[28] = *(Whatever+3);
    Amiga_EXE_Header_Block[25] = Amiga_EXE_Header_Block[29] = *(Whatever+2);
    Amiga_EXE_Header_Block[26] = Amiga_EXE_Header_Block[30] = *(Whatever+1);
    Amiga_EXE_Header_Block[27] = Amiga_EXE_Header_Block[31] = *Whatever;
    Save_Rip_Special ( "DragPack 1.00 Exe-file", DragPack100,  Amiga_EXE_Header_Block , 40 );
    free ( Amiga_EXE_Header_Block );
  }
  else
  {
    PW_Start_Address -= 40;
    Save_Rip ( "DragPack 1.00 Exe-file", DragPack100 );
  }
  
  if ( Save_Status == GOOD )
    PW_i += 0x42;  /* 40 should do but call it "just to be sure" :) */
}

