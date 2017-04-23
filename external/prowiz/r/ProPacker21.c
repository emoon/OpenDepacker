/*
 * 5th, oct 2001 : strenghtened a test that hanged the prog sometimes
*/

/* testPP21() */
/* Rip_PP21() */
/* Depack_PP21() */

#include "globals.h"
#include "extern.h"


int16_t	 testPP21 ( void )
{
  /* test #1 */
  if ( PW_i < 3 )
  {
/*printf ( "#1 (PW_i:%ld)\n" , PW_i );*/
    return BAD;
  }
  if ( (PW_i+891) >= PW_in_size)
  {
/*printf ( "#1 (PW_i:%ld)\n" , PW_i );*/
    return BAD;
  }

  /* test #2 */
  PW_Start_Address = PW_i-3;
  PW_WholeSampleSize=0;
  for ( PW_j=0 ; PW_j<31 ; PW_j++ )
  {
    PW_k = (((in_data[PW_Start_Address+PW_j*8]*256)+in_data[PW_Start_Address+1+PW_j*8])*2);
    PW_WholeSampleSize += PW_k;
    /* finetune > 0x0f ? */
    if ( in_data[PW_Start_Address+2+8*PW_j] > 0x0f )
    {
/*printf ( "#2 (start:%ld)\n" , PW_Start_Address );*/
      return BAD;
    }
    /* loop start > size ? */
    PW_l = ((in_data[PW_Start_Address+4+PW_j*8]*256)+in_data[PW_Start_Address+5+PW_j*8])*2;
    if ( PW_l != 0xFFFF){
    if ( (((in_data[PW_Start_Address+4+PW_j*8]*256)+in_data[PW_Start_Address+5+PW_j*8])*2) > PW_k )
    {
/*printf ( "#2,1 (start:%ld)\n" , PW_Start_Address );*/
      return BAD;
    }}
  }
  if ( PW_WholeSampleSize <= 2 )
  {
/*printf ( "#2,2 (start:%ld)\n" , PW_Start_Address );*/
    return BAD;
  }

  /* test #3   about size of pattern list */
  PW_l = in_data[PW_Start_Address+248];
  if ( (PW_l > 127) || (PW_l==0) )
  {
/*printf ( "#3 (start:%ld)\n" , PW_Start_Address );*/
    return BAD;
  }

  /* get the highest track value */
  PW_k=0;
  for ( PW_j=0 ; PW_j<512 ; PW_j++ )
  {
    PW_l = in_data[PW_Start_Address+250+PW_j];
    if ( PW_l>PW_k )
      PW_k = PW_l;
  }
  /* PW_k is the highest track number */
  PW_k += 1;
  PW_k *= 64;
  if ( (PW_k*2) + PW_Start_Address + 763 > PW_in_size )
  {
/* printf ( "#3,5 (start:%ld)\n" , PW_Start_Address)*/
    return BAD;
  }

  /* test #4  track data value > $4000 ? */
  PW_m = 0;
  for ( PW_j=0 ; PW_j<PW_k ; PW_j++ )
  {
    PW_l = (in_data[PW_Start_Address+762+PW_j*2]*256)+in_data[PW_Start_Address+763+PW_j*2];
    if ( PW_l > PW_m )
      PW_m = PW_l;
    if ( PW_l > 0x4000 )
    {
/*printf ( "#4 (start:%ld)(where:%ld)\n" , PW_Start_Address,PW_Start_Address+PW_j*2+762 );*/
      return BAD;
    }
  }

  /* test #5  reference table size *4 ? */
  PW_k *= 2;
  PW_l = (in_data[PW_Start_Address+PW_k+762]*256*256*256)
    +(in_data[PW_Start_Address+PW_k+763]*256*256)
    +(in_data[PW_Start_Address+PW_k+764]*256)
    +in_data[PW_Start_Address+PW_k+765];
  if ( PW_l != ((PW_m+1)*4) )
  {
/*printf ( "#5 (start:%ld)(where:%ld)\n" , PW_Start_Address,(PW_Start_Address+PW_k+762) );*/
    return BAD;
  }

  return GOOD;
}



void Rip_PP21 ( void )
{
  /* PW_k is still the size of the track "data" ! */
  /* PW_WholeSampleSize is still the whole sample size */

  PW_l = (in_data[PW_Start_Address+762+PW_k]*256*256*256)
    +(in_data[PW_Start_Address+763+PW_k]*256*256)
    +(in_data[PW_Start_Address+764+PW_k]*256)
    +in_data[PW_Start_Address+765+PW_k];

  OutputSize = PW_WholeSampleSize + PW_k + PW_l + 766;

  CONVERT = GOOD;
  Save_Rip ( "ProPacker v2.1 module", Propacker_21 );
  
  if ( Save_Status == GOOD )
    PW_i += 4;  /* 3 should do but call it "just to be sure" :) */
}



/*
 *   ProPacker_21.c   1997 (c) Asle / ReDoX
 *
 * Converts PP21 packed MODs back to PTK MODs
 * thanks to Gryzor and his ProWizard tool ! ... without it, this prog
 * would not exist !!!
 *
 * update : 26/11/1999 by Sylvain "Asle" Chipaux
 *   - reduced to only one FREAD.
 *   - Speed-up and Binary smaller.
 * update : 8 dec 2003
 *   - no more fopen ()
 * update : 20100814
 *   - rewrote depacker (no more useless patterns)
 * update : 20100815
 *   - removed unused var and ;
 * update : 23 aug 2010
 *   - fixed yet another patternlist bug
*/

void Depack_PP21 ( void )
{
  uint8_t *Header, *Pattern;
  uint32_t	 ReadTrkPat[128][4], ReadPat[128];
  int32_t	 Highest_Track = 0;
  int32_t	 whereTableRef;
  int32_t	 i=0,j=0,k=0,l=0,m=0;
  int32_t	 Total_Sample_Size=0;
  int32_t	 Where=PW_Start_Address;
  FILE *out;
  /*FILE *info;*/

  if ( Save_Status == BAD )
    return;


  sprintf ( Depacked_OutName , "%d.mod" , Cpt_Filename-1 );
  out = PW_fopen ( Depacked_OutName , "w+b" );
  /*info = PW_fopen ( "info.txt" , "w+b");*/

  Header = (uint8_t *)malloc(1084);
  Pattern = (uint8_t *)malloc(1024);
  BZERO ( Header , 1084 );
  BZERO ( Pattern , 1024 );

  for ( i=0 ; i<31 ; i++ )
  {
    Total_Sample_Size += (((in_data[Where]*256)+in_data[Where+1])*2);
    /* siz,fine,vol,lstart,lsize */
    Header[42+i*30] = in_data[Where];
    Header[43+i*30] = in_data[Where+1];
    Header[44+i*30] = in_data[Where+2];
    Header[45+i*30] = in_data[Where+3];
    Header[46+i*30] = in_data[Where+4];
    Header[47+i*30] = in_data[Where+5];
    Header[48+i*30] = in_data[Where+6];
    Header[49+i*30] = in_data[Where+7];
    Where += 8;
  }

  /* pattern table lenght */
  Header[950] = in_data[Where];
  Where += 1;

  /*printf ( "Number of patterns : %d\n" , Header[950] );*/

  /* NoiseTracker restart byte */
  Header[951] = in_data[Where];
  Where += 1;

  /* now, where = 0xFA*/
  for (i=0;i<128;i++)
  {
    ReadPat[i] = (in_data[Where+i]*256*256*256) + 
      (in_data[Where+i+128]*256*256) +
      (in_data[Where+i+256]*256) +
      in_data[Where+i+384];
    ReadTrkPat[i][0] = in_data[Where+i];
    ReadTrkPat[i][1] = in_data[Where+i+128];
    ReadTrkPat[i][2] = in_data[Where+i+256];
    ReadTrkPat[i][3] = in_data[Where+i+384];
    if (ReadTrkPat[i][0] > Highest_Track)
      Highest_Track = ReadTrkPat[i][0];
    if (ReadTrkPat[i][1] > Highest_Track)
      Highest_Track = ReadTrkPat[i][1];
    if (ReadTrkPat[i][2] > Highest_Track)
      Highest_Track = ReadTrkPat[i][2];
    if (ReadTrkPat[i][3] > Highest_Track)
      Highest_Track = ReadTrkPat[i][3];
    /*fprintf (info,"%x-%x-%x-%x (%ld)\n",ReadTrkPat[i][0],ReadTrkPat[i][1],ReadTrkPat[i][2],ReadTrkPat[i][3],ReadPat[i]);*/
  }
  /*fprintf ( info,"Number of tracks : %d\n\n" , Highest_Track+1 );*/

  /* sorting ?*/
  k = 0; /* next min */
  l = 0;

  /* put the first pattern number */
  /* could be stored several times */
  for (j=0; j<Header[950] ; j++)
  {
    m = 0x7fffffff; /* min */
    /*fprintf (info,"[%ld]pattern[%ld]",l,j);*/
    /*search for min */
    for (i=0; i<Header[950] ; i++)
      if ((ReadPat[i]<m) && (ReadPat[i]>k))
      {
	    m = ReadPat[i];
	    /*fprintf (info,"(i:%ld)(min:%ld)(k:%ld)",i,m,k);*/
      }
    /* if k == m then an already existing ref was found */
    if (k==m)
    {
      /*fprintf (info," <- pattern known (k:%ld - m:%ld)\n",k,m);*/
      continue;
    }
/*    else
      fprintf (info,"\n");*/
    /* m is the next minimum */
    k = m;
    for (i=0; i<Header[950] ; i++)
      if (ReadPat[i] == k)
      {
        /*fprintf (info,"-> pos %ld gets %ld\n",i,l);*/
        Header[952+i] = (unsigned char)l;
        j++;
      }
    j--;
    l++;
  }
  /*fprintf (info,"\nnumber of pattern stored : %ld",l);*/
  /*fprintf (info,"number of pattern stored : %ld\n\n",l);*/
  /*fprintf (info," (%ld)\n",l);*/

  /* write ptk's ID */
  Header[1080] = 'M';
  Header[1081] = Header[1083] = '.';
  Header[1082] = 'K';
  fwrite (Header, 1084, 1, out);


  /* put 'where' at track data level - after track list */
  Where = PW_Start_Address + 762;


  /* rebuild patterns now */
  /* l is the number of stored patterns */
  /* rebuild pattern data now */
  whereTableRef = ((Highest_Track + 1)*128) + 4 + Where;
  /*printf ( "\nwhereTableRef : %ld\n",whereTableRef);*/
  for (i=0;i<l;i++)
  {
    /*fprintf (info,"pattern %ld (/%ld)\n",i,l);*/
    /*fflush (info);*/
    BZERO(Pattern,1024);
    /* which pattern is it now ? */
    for (j=0;j<Header[950];j++)
    {
      if (Header[952+j] == i)
        break; /* found */
    }
    for (k=0;k<4;k++) /* loop on 4 tracks' refs*/
    {
      int32_t	 d;

      /* loop on notes */
      for (d=0;d<64;d++)
      {
        /* read one ref value to be fetch in the reference table */
        int32_t	 val = (in_data[Where+(ReadTrkPat[j][k]*128)+(d*2)])*256
             + in_data[Where+(ReadTrkPat[j][k]*128)+(d*2)+1];

	    Pattern[k*4+d*16] = in_data[whereTableRef + (val*4)];
        Pattern[k*4+d*16+1] = in_data[whereTableRef + (val*4) + 1];
        Pattern[k*4+d*16+2] = in_data[whereTableRef + (val*4) + 2];
        Pattern[k*4+d*16+3] = in_data[whereTableRef + (val*4) + 3];
      }
    }
    fwrite ( Pattern , 1024 , 1 , out );
  }
  free ( Pattern );
  free ( Header );

  /* locate sample start addy - after reference table */
  j = ((in_data[whereTableRef-4]*256*256*256)+
       (in_data[whereTableRef-3]*256*256)+
       (in_data[whereTableRef-2]*256)+
        in_data[whereTableRef-1]);
  /*printf ( "\nj:%ld - Where:%ld\n",j,Where );*/

  /* sample data */
  fwrite ( &in_data[j+whereTableRef] , Total_Sample_Size , 1 , out );

  Crap ( "  ProPacker v2.1  " , BAD , BAD , out );

  fflush ( out );
  fclose ( out );
  /*fflush (info);
  fclose (info);*/

  printf ( "done\n" );
  return; /* useless ... but */
}
