#ifdef _cplusplus
extern "C" {
#endif
#include "dbadisplay.h"

/* Function:  show_pretty_dba_align(alb,one,two,ofp)
 *
 * Descrip:    Shows an alignment of from the dba algorithm in
 *             pretty formatted ascii text
 *
 *
 * Arg:        alb [UNKN ] Undocumented argument [AlnBlock *]
 * Arg:        one [UNKN ] Undocumented argument [Sequence *]
 * Arg:        two [UNKN ] Undocumented argument [Sequence *]
 * Arg:        ofp [UNKN ] Undocumented argument [FILE *]
 *
 * Return [UNKN ]  Undocumented return value [boolean]
 *
 */
# line 17 "dbadisplay.dy"
boolean show_pretty_dba_align(AlnBlock * alb,Sequence * one,Sequence * two,FILE * ofp)
{
  boolean ret;
  btCanvas * btc;

  btc = new_Ascii_btCanvas(ofp,20,50,0,3); /*+6 in case we want to put in numbers */

  ret = show_pretty_dba_align_btcanvas(btc,alb,one,two);

  free_btCanvas(btc);

  return ret;
}

/* Function:  is_unmatched_block(alc)
 *
 * Descrip:    tests whether this is a dba unmatched block
 *
 *
 * Arg:        alc [UNKN ] Undocumented argument [AlnColumn *]
 *
 * Return [UNKN ]  Undocumented return value [boolean]
 *
 */
# line 35 "dbadisplay.dy"
boolean is_unmatched_block(AlnColumn * alc)
{
  if( strcmp(alc->alu[0]->text_label,"UM") == 0 ) 
    return TRUE;
  if( strcmp(alc->alu[0]->text_label,"UI") == 0 ) 
    return TRUE;

  if( strcmp(alc->alu[1]->text_label,"UM") == 0 ) 
    return TRUE;
  if( strcmp(alc->alu[1]->text_label,"UI") == 0 ) 
    return TRUE;

  return FALSE;
}

/* Function:  show_pretty_dba_align_btcanvas(btc,alb,one,two)
 *
 * Descrip:    Shows the dba alignment on the block text
 *             canvas
 *
 *
 * Arg:        btc [UNKN ] Undocumented argument [btCanvas *]
 * Arg:        alb [UNKN ] Undocumented argument [AlnBlock *]
 * Arg:        one [UNKN ] Undocumented argument [Sequence *]
 * Arg:        two [UNKN ] Undocumented argument [Sequence *]
 *
 * Return [UNKN ]  Undocumented return value [boolean]
 *
 */
# line 55 "dbadisplay.dy"
boolean show_pretty_dba_align_btcanvas(btCanvas * btc,AlnBlock * alb,Sequence * one,Sequence * two)
{
  AlnColumn * alc;
  AlnUnit * q;
  AlnUnit * t;
  char buffer[14];
  boolean at_start = TRUE;
  boolean new_block;
  
  btPasteArea * btp;

  for(alc=alb->start;alc != NULL;) {

    /** loop over unmatched portions **/
    new_block = FALSE;
    for(; alc != NULL && is_unmatched_block(alc) == TRUE;alc = alc->next)
      new_block = TRUE;
    if( alc == NULL )
      break;
    if( strcmp(alc->alu[0]->text_label,"END") == 0 ) {
      alc = NULL;
      break;
    }
    
    if( new_block == TRUE && at_start == FALSE ) {
      /* add in block separator */
      btp = get_reserved_left_btCanvas(btc);
      paste_string_btPasteArea(btp,0,1,">-----------<",BC_RIGHT,0);
      free_btPasteArea(btp);
      advance_line_btCanvas(btc);
    }

    at_start = FALSE;

    /** put names in **/

    btp = get_reserved_left_btCanvas(btc);
    paste_string_btPasteArea(btp,0,0,one->name,BC_RIGHT,0);
    paste_string_btPasteArea(btp,0,2,two->name,BC_RIGHT,0);
    
    sprintf(buffer,"%d",alc->alu[0]->start+1+1);

    paste_string_btPasteArea(btp,12,0,buffer,BC_RIGHT,0);

    sprintf(buffer,"%d",alc->alu[1]->start+1+1);

    paste_string_btPasteArea(btp,12,2,buffer,BC_RIGHT,0);
 
    if( strcmp(alc->alu[0]->text_label,"MM65") == 0 ) {
      paste_string_btPasteArea(btp,17,1,"A",BC_RIGHT,0);
    } else if ( strcmp(alc->alu[0]->text_label,"MM75") == 0 ) {
      paste_string_btPasteArea(btp,17,1,"B",BC_RIGHT,0);
    } else if ( strcmp(alc->alu[0]->text_label,"MM85") == 0 ) {
      paste_string_btPasteArea(btp,17,1,"C",BC_RIGHT,0);
    } else if ( strcmp(alc->alu[0]->text_label,"MM95") == 0 ) {
      paste_string_btPasteArea(btp,17,1,"D",BC_RIGHT,0);
    } else {
      warn("Weird label in dba match block at start of block %s",alc->alu[0]->text_label);
      paste_string_btPasteArea(btp,17,1,"??",BC_RIGHT,0);
    }

    free_btPasteArea(btp);
    /** now loop over this block **/

    for(;alc != NULL &&  can_get_paste_area_btCanvas(btc,1) == TRUE;alc=alc->next) {
      
      q = alc->alu[0];
      t = alc->alu[1];

      /*
       * at the end, break
       */
      if( strcmp(q->text_label,"END") == 0 ) {
	alc = NULL;
	break;
      }

      /* Unmatched and break */

      if( q->text_label[0] == 'U' ) {
	break;
      }

      /*
       * Get the paste area, length 1, depth will be 3
       */

      btp = get_paste_area_btCanvas(btc,1);

      /*
       * Write in the query sequence
       *
       */

      if( strstartcmp(q->text_label,"MM") == 0 ) {
	paste_char_btPasteArea(btp,0,0,toupper((int)one->seq[q->start+1]),0);
      } else {
	/** is insert- we could check **/
	if( strstartcmp(q->text_label,"MI") != 0 ) {
	  warn("Got an uninterpretable label, %s",q->text_label);
	  paste_char_btPasteArea(btp,0,0,'?',0);
	} else {
	  paste_char_btPasteArea(btp,0,0,'-',0);
	}
      }

      /*
       * Write in the target sequence
       *
       */

      if( strstartcmp(t->text_label,"MM") == 0 ) {
	paste_char_btPasteArea(btp,0,2,toupper((int)two->seq[t->start+1]),0);
      } else {
	/** is insert- we could check **/
	if( strstartcmp(t->text_label,"MI") != 0 ) {
	  warn("Got an uninterpretable label, %s",t->text_label);
	  paste_char_btPasteArea(btp,0,2,'?',0);
	} else {
	  paste_char_btPasteArea(btp,0,2,'-',0);
	}
      }

      /*
       * Match line
       */



      if( strstartcmp(q->text_label,"MM") == 0 && strstartcmp(t->text_label,"MM") == 0 ) {
	if( one->seq[q->start+1] == two->seq[t->start+1] ) {
	  paste_char_btPasteArea(btp,0,1,two->seq[t->start+1],0);
	} else {	   
	  paste_char_btPasteArea(btp,0,1,' ',0);
	}
      } else 
	paste_char_btPasteArea(btp,0,1,' ',0);
      
      free_btPasteArea(btp);

    } /* end of for this block */

    advance_line_btCanvas(btc);
  } /* end of for the alignment */

  return TRUE; /* we never returned false. Ooops! */
}

  
# line 218 "dbadisplay.c"

#ifdef _cplusplus
}
#endif
