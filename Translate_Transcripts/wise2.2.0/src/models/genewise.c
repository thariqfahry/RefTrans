/* has to be before the others due to nasty namespace clashes */
#define WISE2_CROSS_HMMER2
#include "wise2xhmmer2.h"


#include "dyna.h"
#include "version.h" 

char * program_name = "genewise";

/*
 * program specific includes
 */


#include "gwrap.h"
#include "genedisplay.h"
#include "matchsum.h"


/*
 * program specific variables
 */

char * dna_seq_file  = NULL;
Genomic * gen        = NULL;
boolean is_embl      = FALSE;

char * tstart_str    = NULL;
int tstart           = -1;

char * tend_str      = NULL;
int tend             = -1;

boolean reverse      = FALSE;
boolean target_abs   = FALSE;
boolean do_both      = FALSE;

boolean use_tsm      = FALSE;

char * protein_file  = NULL;
Protein * pro        = NULL;

char * hmm_file       = NULL;
ThreeStateModel * tsm = NULL;
char * hmm_name       = NULL;

char * potential_file = NULL;
PotentialGene * pg    = NULL;

char * qstart_str    = NULL;
int qstart           = -1;

char * qend_str      = NULL;
int qend             = -1;

char * gene_file        = NULL;
GeneFrequency21 * gf    = NULL;

char * new_gene_file    = NULL;
GeneStats * gs          = NULL;

GeneModelParam * gmp    = NULL;

boolean use_new_stats   = 0;

char * matrix_file      = NULL;
CompMat * mat           = NULL;

char * gap_str          = "12";
int gap                 = 12;

char * ext_str          = "2";
int ext                 = 2;

char * length_of_N_str  = "10";
int length_of_N         = 10;

boolean flat_insert     = TRUE;

char * codon_file       = NULL;
CodonTable * ct         = NULL;

char * output_file      = "-";
FILE * ofp              = NULL;

RandomModelDNA * rmd    = NULL;

char * allN_string      = "1.0";
Probability allN        = 1.0;

char * subs_string      = "0.00001";
double subs_error       = 0.00001;

char * indel_string      = "0.00001";
double indel_error       = 0.00001;

char * cfreq_string      = "flat";
boolean model_codon      = FALSE;

char * splice_string     = "model";
boolean model_splice     = TRUE;

char * startend_string   = "default";
int startend             = TSM_default;

char * null_string       = "syn";
boolean use_syn          = FALSE;

char * intron_string     = "tied";
boolean use_tied_model   = FALSE;

int alg                  = GWWRAP_623;
char * alg_str           = NULL;

char * kbyte_str         = NULL;
int kbyte                = 10000; /* will be reset in build_defaults */

boolean show_PackAln     = FALSE;
boolean show_cumlative_PackAln = FALSE;
boolean show_AlnBlock    = FALSE;
boolean show_ace         = FALSE;
boolean show_gff         = FALSE;
boolean show_trans       = FALSE;
boolean show_pep         = FALSE;
boolean show_cdna        = FALSE;
boolean show_pretty      = FALSE;
boolean show_pretty_gene = FALSE;
boolean show_supp_gene   = FALSE;
boolean show_gene_plain  = FALSE;
boolean show_match_sum   = FALSE;
boolean show_para        = FALSE;
boolean show_overlap     = FALSE;
boolean show_embl        = FALSE;
boolean show_diana       = FALSE;

boolean pseudo           = FALSE;

char * main_block_str      = "50";
int main_block           = 50;

char * divide_str        = "//";

Probability rnd_loop      = 0.99;
Probability cds_loop      = 0.97;
Probability rnd_to_model  = (1 - 0.99) / 3;
Probability link_loop     = 0.98;
Probability link_to_model = (1- 0.98) / 3;

AlnBlock * alb;
PackAln  * pal;

GenomicRegion * gr;
GenomicRegion * embl;

MatchSummarySet * mss;

RandomModel * rm;

DPRunImpl * dpri;

void show_parameters(void)
{
  fprintf(ofp,"%s %s (%s release)\n",program_name,VERSION_NUMBER,RELEASE_DAY);
  fprintf(ofp,"This program is freely distributed under a GPL. See source directory\n");
  fprintf(ofp,"Copyright (c) GRL limited: portions of the code are from separate copyright\n\n");
  if( use_tsm == FALSE) {
    fprintf(ofp,"Query protein:       %s\n",pro->baseseq->name);
    fprintf(ofp,"Comp Matrix:         %s\n",matrix_file);
    fprintf(ofp,"Gap open:            %d\n",gap);
    fprintf(ofp,"Gap extension:       %d\n",ext);
  }
  else 
    fprintf(ofp,"Query model:         %s\n",tsm->name);
  fprintf(ofp,"Start/End            %s\n",startend_string);
  fprintf(ofp,"Target Sequence      %s\n",gen->baseseq->name);
  fprintf(ofp,"Strand:              %s\n",do_both == TRUE ? "both" : reverse == TRUE ? "reverse" : "forward");
  fprintf(ofp,"Start/End (protein)  %s\n",startend_string);
  fprintf(ofp,"Gene Paras:          %s\n",gene_file);
  fprintf(ofp,"Codon Table:         %s\n",codon_file);
  fprintf(ofp,"Subs error:          %2.2g\n",subs_error);
  fprintf(ofp,"Indel error:         %2.2g\n",indel_error);
  fprintf(ofp,"Model splice?        %s\n",splice_string);
  fprintf(ofp,"Model codon bias?    %s\n",cfreq_string);
  fprintf(ofp,"Model intron bias?   %s\n",intron_string);
  fprintf(ofp,"Null model           %s\n",null_string);
  fprintf(ofp,"Algorithm            %s\n",alg_str);
}

boolean show_pretty_aln(void)
{
  Protein * ps;

  fprintf(ofp,"\n%s output\nScore %4.2f bits over entire alignment\n",program_name,Score2Bits(pal->score));
  
  
  if( alg == GWWRAP_2193L || alg == GWWRAP_2193) {
    fprintf(ofp,"Entrie alignment score contains unseen 'random' score segments\nYou should only use the per-alignments score printed below\nfor the bits score of the alignment\n\n");
  }

  if( use_syn == FALSE ) {
    fprintf(ofp,"Scores as bits over a flat simple random model\n\n");
  } else {
    fprintf(ofp,"Scores as bits over a synchronous coding model\n\n");
  }

  
  
  if( use_tsm == FALSE ) {
    fprintf(ofp,"Warning: The bits scores is not probablistically correct for single seqs\nSee WWW help for more info\n\n");

    protgene_ascii_display(alb,pro->baseseq->seq,pro->baseseq->name,pro->baseseq->offset,gen,ct,15,main_block,(alg == GWWRAP_623L || alg == GWWRAP_2193L || alg == GWWRAP_2193) ? TRUE : FALSE, ofp);
  } else {
    ps = pseudo_Protein_from_ThreeStateModel(tsm);
    protgene_ascii_display(alb,ps->baseseq->seq,ps->baseseq->name,ps->baseseq->offset,gen,ct,15,main_block,(alg == GWWRAP_623L || alg == GWWRAP_2193L || alg == GWWRAP_2193) ? TRUE : FALSE,ofp);
    free_Protein(ps);
  }
  fprintf(ofp,"%s\n",divide_str);

  return TRUE;
}


boolean show_output(void)
{
  int i;

  cDNA * cdna;
  Protein * trans;
  GenomicOverlapResults * gor;
  AlnColumn * alt;
  

 

  if( show_pretty == TRUE ) {
    show_pretty_aln();
  }

  if( show_match_sum == TRUE ) {
    show_MatchSummary_genewise_header(ofp);
    show_MatchSummarySet_genewise(mss,ofp);
    fprintf(ofp,"%s\n",divide_str);
  }

  if( show_pretty_gene == TRUE ) {
    show_pretty_GenomicRegion(gr,0,ofp);
    fprintf(ofp,"%s\n",divide_str);
  }

  if( show_supp_gene == TRUE ) {
    show_pretty_GenomicRegion(gr,1,ofp);
    fprintf(ofp,"%s\n",divide_str);
  }

  if( show_embl == TRUE ) {
    write_Embl_FT_GenomicRegion(gr,ofp);
    fprintf(ofp,"%s\n",divide_str);
  }

  if( show_diana == TRUE ) {
    write_Diana_FT_GenomicRegion(gr,ofp);
    fprintf(ofp,"%s\n",divide_str);
  }

  if( show_overlap == TRUE ) {
    gor = Genomic_overlap(gr,embl);
    show_GenomicOverlapResults(gor,ofp);
    fprintf(ofp,"%s\n",divide_str);
  }


  if( show_trans == TRUE ) {
    for(i=0;i<gr->len;i++) {
      if( gr->gene[i]->ispseudo == TRUE ) {
	fprintf(ofp,"#Gene %d is a pseudo gene - no translation possible\n",i);
      } else {
	trans = get_Protein_from_Translation(gr->gene[i]->transcript[0]->translation[0],ct);
	write_fasta_Sequence(trans->baseseq,ofp);
      }
    } 
    fprintf(ofp,"%s\n",divide_str);
  }

  if( show_pep == TRUE ) {
    alt = alb->start;
    for(;alt != NULL;) {
      trans = Protein_from_GeneWise_AlnColumn(gen->baseseq,alt,1,&alt,ct,is_random_AlnColumn_genewise);
      if ( trans == NULL ) 
	break;
      write_fasta_Sequence(trans->baseseq,ofp);
      free_Protein(trans);
    }
    fprintf(ofp,"%s\n",divide_str);
  }

  if( show_cdna == TRUE ) {
    for(i=0;i<gr->len;i++) {
      cdna = get_cDNA_from_Transcript(gr->gene[i]->transcript[0]);
      write_fasta_Sequence(cdna->baseseq,ofp);
    } 
    fprintf(ofp,"%s\n",divide_str);
  }


  if( show_ace == TRUE ) {
    show_ace_GenomicRegion(gr,gen->baseseq->name,ofp);
    fprintf(ofp,"%s\n",divide_str);
  }

  if( show_gff == TRUE ) {
    show_GFF_GenomicRegion(gr,gen->baseseq->name,"GeneWise",ofp);
    fprintf(ofp,"%s\n",divide_str);
  }

  if( show_gene_plain == TRUE ) {
    show_GenomicRegion(gr,ofp);
    fprintf(ofp,"%s\n",divide_str);
  }

  if( show_AlnBlock == TRUE ) {
    mapped_ascii_AlnBlock(alb,Score2Bits,0,ofp);
    fprintf(ofp,"%s\n",divide_str);
  }

  if( show_cumlative_PackAln == TRUE ) {
    show_bits_and_cumlative_PackAln(pal,ofp);
    fprintf(ofp,"%s\n",divide_str);
  }

  if( show_PackAln == TRUE ) {
    show_simple_PackAln(pal,ofp);
    fprintf(ofp,"%s\n",divide_str);
  }
  


  return TRUE;
}

boolean build_alignment(void)
{
  GeneParameter21 * gpara;
  GeneModel * gm;

  int is_global;

  if( use_new_stats == 0 ) {
    gpara = GeneParameter21_wrap(gf,subs_error,indel_error,rmd,model_codon,model_splice,use_tied_model,ct,rnd_loop,cds_loop,rnd_to_model,link_loop,link_to_model);
  } else {
    gm = GeneModel_from_GeneStats(gs,gmp);
    
    gpara= GeneParameter21_from_GeneModel(gm,ct,rnd_loop,cds_loop,rnd_to_model,link_loop,link_to_model);
  }
  

  if( gpara == NULL ) {
    warn("Sorry - could not build gene parameters. Must be a bug of some sort");
    return FALSE;
  }

  if( use_tsm == FALSE ) {

    /* Dirty hack to pass in the ability now to
       set startend to TSM_endbiased */

    if( startend == TSM_endbiased ) {
      is_global = TSM_endbiased;
    } else if ( startend == TSM_global ) {
      is_global = 1;
    } else {
      is_global = 0;
    }

    alb =  AlnBlock_from_protein_genewise_wrap(pro,gen,mat,-gap,-ext,gpara,rmd,rmd,alg,is_global,use_syn,rm,allN,pg,dpri,&pal);
  } else {
    alb =  AlnBlock_from_TSM_genewise_wrap(tsm,gen,gpara,rmd,rmd,use_syn,alg,allN,flat_insert,pg,dpri,&pal);
  }

  free_GeneParameter21(gpara);



  if( alb == NULL )
    return FALSE;


  gr = new_GenomicRegion(gen);


  add_Genes_to_GenomicRegion_GeneWise(gr,gen->baseseq->offset,gen->baseseq->end,alb,use_tsm == TRUE ? tsm->name : pro->baseseq->name,pseudo,NULL);

  if( use_tsm == TRUE) 
    mss = MatchSummarySet_from_AlnBlock_genewise(alb,tsm->name,1,gen->baseseq);
  else
    mss = MatchSummarySet_from_AlnBlock_genewise(alb,pro->baseseq->name,pro->baseseq->offset,gen->baseseq);

  return TRUE;
}

boolean free_temporary_objects(void)
{
  alb = free_AlnBlock(alb);
  pal = free_PackAln(pal);
  mss = free_MatchSummarySet(mss);
  gr = free_GenomicRegion(gr);

  return TRUE;
}

boolean free_io_objects(void)
{
  if( use_tsm == TRUE) {
    free_ThreeStateModel(tsm);
  } else {
    free_Protein(pro);
  }

  free_CodonTable(ct);
  free_GeneFrequency21(gf);
  free_RandomModelDNA(rmd);
  if( is_embl ) {
    free_GenomicRegion(embl);
  } 
  free_Genomic(gen);
  

  return TRUE;
}
  


boolean build_objects(void)
{
  boolean ret = TRUE;
  Protein * pro_temp;
  Genomic * gen_temp;
  FILE * ifp;



  startend = threestatemodel_mode_from_string(startend_string);
  if( startend == TSM_unknown ) {
    warn("String %s was unable to converted into a start/end policy\n",startend_string);
    ret = FALSE;
  }


  if( tstart_str != NULL ) {
    if( is_integer_string(tstart_str,&tstart) == FALSE || tstart < 0) {
      warn("Could not make %s out as target start",tstart);
      ret = FALSE;
    }
  }

  if( tend_str != NULL ) {
    if( is_integer_string(tend_str,&tend) == FALSE || tend < 0) {
      warn("Could not make %s out as target end",tend);
      ret = FALSE;
    }
  }

  if( is_integer_string(gap_str,&gap) == FALSE ) {
      warn("Could not make %s out as gap penalty (must be integer at the moment)",gap_str);
      ret = FALSE;
  }
  

  if( is_integer_string(ext_str,&ext) == FALSE ) {
    warn("Could not make %s out as gap penalty (must be integer at the moment)",ext_str);
    ret = FALSE;
  }

  if( is_embl == FALSE ) {
    if( (gen = read_fasta_file_Genomic(dna_seq_file,length_of_N)) == NULL ) {
      ret = FALSE;
      warn("Could not read genomic sequence in %s",dna_seq_file);
      gen = NULL;
    } 
  } else {
    embl = read_EMBL_GenomicRegion_file(dna_seq_file);
    if( embl == NULL ) {
      warn("Could not read genomic EMBL file in %s",dna_seq_file);
      gen = NULL;
      ret = FALSE;
    } else {
      gen = hard_link_Genomic(embl->genomic);
      
    }
  }

  if( gen != NULL ) {
    if( tstart != -1 || tend != -1 ) {
      if( tstart == -1 )
	tstart = 0;
      if( tend == -1 ) 
	tend = gen->baseseq->len;
      
      gen_temp = truncate_Genomic(gen,tstart-1,tend);
      if( gen_temp == NULL ){
	ret = FALSE;
      } else {
	free_Genomic(gen);
	gen = gen_temp;
      }
    }
  

    if( reverse == TRUE ) {
      if( tstart > tend ) {
	warn("You have already reversed the DNA by using %d - %d truncation. Re-reversing",tstart,tend);
    }
      
      gen_temp = reverse_complement_Genomic(gen); 
      free_Genomic(gen);
      gen = gen_temp;
    }
  }

  /*
   * Can't truncate on GenomicRegion (for good reasons!).
   * but we want only a section of the EMBL file to be used
   * 
   * So... swap genomic now. Positions in EMBL are still valid,
   * however - some genes will loose their sequence, which will be damaging. ;)
   */

  
  if( is_embl ) {
    free_Genomic(embl->genomic);
    embl->genomic = hard_link_Genomic(gen); /* pointer could be dead anyway ;) */
  }


  if( target_abs == TRUE ) {
    if( is_embl == TRUE ) {
      warn("Sorry you can't both use absolute positioning and EMBL files as I can't cope with all the coordinate remapping. You'll have to convert to fasta.");
      ret =  FALSE;
    }

    gen->baseseq->offset = 1;
    gen->baseseq->end  = strlen(gen->baseseq->seq);
  }
      

  if( qstart_str != NULL ) {
    if( is_integer_string(qstart_str,&qstart) == FALSE || qstart < 0) {
      warn("Could not make %s out as query start",qstart);
      ret = FALSE;
    }
  }

  if( qend_str != NULL ) {
    if( is_integer_string(qend_str,&qend) == FALSE || qend < 0) {
      warn("Could not make %s out as query end",qend);
      ret = FALSE;
    }
  }



  if( use_tsm == FALSE ) {
    if( startend != TSM_default && startend != TSM_global && startend != TSM_local && startend != TSM_endbiased) {
      warn("Proteins can only have local/global/endbias startend policies set, not %s",startend_string);
      ret = FALSE;
    }
    if( (pro = read_fasta_file_Protein(protein_file)) == NULL ) {
      ret = FALSE;
      warn("Could not read Protein sequence in %s",protein_file);
    } else {
      if( qstart != -1 || qend != -1 ) {
	if( qstart == -1 )
	  qstart = 0;
	if( qend == -1 ) 
	  qend = pro->baseseq->len;

	pro_temp = truncate_Protein(pro,qstart-1,qend);
	if( pro_temp == NULL ){
	  ret = FALSE;
	} else {
	  free_Protein(pro);
	  pro = pro_temp;
	}
      }
    }
  } else {
    /** using a HMM **/

    /*tsm = read_HMMer_1_7_ascii_file(hmm_file);*/
    /*tsm = Wise2_read_ThreeStateModel_from_hmmer1_file(hmm_file);*/
    tsm = HMMer2_read_ThreeStateModel(hmm_file);


    if( tsm == NULL ) {
      warn("Could not read hmm from %s\n",hmm_file);
      ret = FALSE;
    }  else {

      display_char_in_ThreeStateModel(tsm);
      if( hmm_name != NULL ) {
	if( tsm->name != NULL ) 
	  ckfree(tsm->name);
	tsm->name = stringalloc(hmm_name);
      }

      if( tsm == NULL ) {
	warn("Could not read %s as a hmm",hmm_file);
      }
      
      /** have to set start/end **/
      set_startend_policy_ThreeStateModel(tsm,startend,30,0.2);
      
    }
  } /* end of else tsm != NULL */
    

  if( main_block_str != NULL ) {
    if( is_integer_string(main_block_str,&main_block) == FALSE ) {
      warn("Could not get maximum main_block number %s",main_block_str);
      ret = FALSE;
    }
  }
   
  
  if( potential_file != NULL ) {
    info("Potential gene file is read...\n");
    pg = read_PotentialGene_file(potential_file);
    if( pg == NULL ) {
      warn("Could not build potential gene!");
      return FALSE;
    }
  }

  if( alg_str != NULL ) {
    alg = gwrap_alg_type_from_string(alg_str);
  } else {
    if( use_tsm == TRUE ) {
      alg_str = "623L";
    } else {
      alg_str = "623";
    }
    alg = gwrap_alg_type_from_string(alg_str);
  }


  if( is_double_string(subs_string,&subs_error) == FALSE ) {
    warn("Could not convert %s to a double",subs_error);
    ret = FALSE;
  }

  if( is_double_string(indel_string,&indel_error) == FALSE ) {
    warn("Could not convert %s to a double",indel_error);
    ret = FALSE;
  }

  if( is_double_string(allN_string,&allN) == FALSE ) {
    warn("Could not convert %s to a double",allN_string);
    ret = FALSE;
  }

  
  if( strcmp(cfreq_string,"model") == 0 ) {
    model_codon = TRUE;
  } else if ( strcmp(cfreq_string,"flat") == 0 ) {
    model_codon = FALSE;
  } else {
    warn("Cannot interpret [%s] as a codon modelling parameter\n",cfreq_string);
    ret = FALSE;
  }
  

  if( strcmp(splice_string,"model") == 0 ) {
    model_splice = TRUE;
  } else if ( strcmp(splice_string,"flat") == 0 ) {
    model_splice = FALSE;
  } else {
    warn("Cannot interpret [%s] as a splice modelling parameter\n",splice_string);
    ret = FALSE;
  }

  if( strcmp(null_string,"syn") == 0 ) {
    use_syn = TRUE;
  } else if ( strcmp(null_string,"flat") == 0 ) {
    use_syn = FALSE;
  } else {
    warn("Cannot interpret [%s] as a null model string\n",null_string);
    ret = FALSE;
  }

  if( strcmp(intron_string,"model") == 0 ) {
    use_tied_model = FALSE;
  } else if ( strcmp(intron_string,"tied") == 0 ) {
    use_tied_model = TRUE;
  } else {
    warn("Cannot interpret [%s] as a intron tieing switch\n",intron_string);
    ret = FALSE;
  }



  if( (rm = default_RandomModel()) == NULL) {
    warn("Could not make default random model\n");
    ret = FALSE;
  }

  if( use_new_stats == 0 ) {
    if( (gf = read_GeneFrequency21_file(gene_file)) == NULL) {
      ret = FALSE;
      warn("Could not read a GeneFrequency file in %s",gene_file);
    }
  } else {
    if( (gs = GeneStats_from_GeneModelParam(gmp)) != NULL ){
      ret=FALSE;
      warn("Could not read gene statistics in %s",new_gene_file);
    }
  } /* end of else using new gene stats */


  if( (mat = read_Blast_file_CompMat(matrix_file)) == NULL) {
    if( use_tsm == TRUE ) {
      info("I could not read the Comparison matrix file in %s; however, you are using a HMM so it is not needed. Please set the WISECONFIGDIR or WISEPERSONALDIR variable correctly to prevent this message.",matrix_file);
    } else {
      warn("Could not read Comparison matrix file in %s",matrix_file);
      ret = FALSE;
    }
  }

  if( (ct = read_CodonTable_file(codon_file)) == NULL) {
    ret = FALSE;
    warn("Could not read codon table file in %s",codon_file);
  }

  if( (ofp = openfile(output_file,"W")) ==  NULL) {
    warn("Could not open %s as an output file",output_file);
    ret = FALSE;
  }

  rmd = RandomModelDNA_std();
  return ret;

}

void build_defaults(void)
{
  gene_file = "human.gf";
  new_gene_file = "gene.stat";
  codon_file = "codon.table";
  matrix_file = "blosum62.bla";
  

}

void reverse_target(void)
{
  Genomic * gen_temp;

  gen_temp = reverse_complement_Genomic(gen);

  free_temporary_objects();

  free_Genomic(gen);

  gen = gen_temp;
  
}

void show_short_help(void)
{
  fprintf(stdout,"%s (%s)\n",program_name,VERSION_NUMBER);
  fprintf(stdout,"This program is freely distributed under a GPL. See -version for more info\n");
  fprintf(stdout,"Copyright (c) GRL limited: portions of the code are from separate copyrights\n\n");
  fprintf(stdout,"genewise <protein-file> <dna-file> in fasta format\n");
  fprintf(stdout," Options. In any order, '-' as filename (for any input) means stdin\n");
  fprintf(stdout," Dna      [-u,-v,-trev,-tabs,-fembl,-both]\n Protein  [-s,-t,-g,-e,-m]\n HMM      [-hmmer,-hname]\n");
  fprintf(stdout," Model    [-codon,-gene,-cfreq,-splice,-subs,-indel,-intron,-null]\n Alg      [-kbyte,-alg]\n");
  fprintf(stdout," Output   [-pretty,-genes,-genesf,-para,-sum,-cdna,-trans,-pep,-ace,-embl,-diana]\n");
  fprintf(stdout,"  ..cont  [-gff,-gener,-alb,-pal,-block,-divide]\n");
  fprintf(stdout," Standard [-help,-version,-silent,-quiet,-errorlog]\n");
  fprintf(stdout,"\nFor more help go %s -help.\n",program_name);
  fprintf(stdout,"\nSee WWW help at http://www.sanger.ac.uk/Software/Wise2/\n");
  exit(63);   
}

void show_help(FILE * ofp)
{
  fprintf(ofp,"%s (%s)\n",program_name,VERSION_NUMBER);
  fprintf(ofp,"genewise <protein-file> <dna-file> in fasta format\n");
  /* program specific help */
  fprintf(ofp,"Dna sequence options\n");
  fprintf(ofp,"  -u               start position in dna\n");
  fprintf(ofp,"  -v               end position in dna\n");
  fprintf(ofp,"  -trev            Compare on the reverse strand\n");
  fprintf(ofp,"  -tfor [default]  Compare on the forward strand\n");
  fprintf(ofp,"  -both            Both strands\n");
  fprintf(ofp,"  -tabs            Report positions as absolute to truncated/reverse sequence\n");
  fprintf(ofp,"  -fembl            File is an EMBL file native format\n");
  fprintf(ofp,"Protein comparison options\n");
  fprintf(ofp,"  -s               start position in protein\n");
  fprintf(ofp,"  -t               end   position in protein\n");
  fprintf(ofp,"  -[g]ap    [%3d]  gap penalty\n",gap);
  fprintf(ofp,"  -[e]xt    [%3d]  extension penalty\n",ext);
  fprintf(ofp,"  -[m]atrix [%s]  Comparison matrix\n",matrix_file);
  fprintf(ofp,"HMM options\n");
  fprintf(ofp,"  -hmmer           Protein file is HMMer file (version 2 compatible)\n");
  fprintf(ofp,"  -hname           Use this as the name of the HMM, not filename\n");
  fprintf(ofp,"Gene Model options\n");
  fprintf(ofp,"  -init   [%s]  [default/global/local/wing/endbias] startend policy for the HMM/protein\n",startend_string);
  fprintf(ofp,"  -codon  [%s]  Codon file\n",codon_file);
  fprintf(ofp,"  -gene   [%s]  Gene parameter file\n",gene_file);
  fprintf(ofp,"  -subs   [%2.2g] Substitution error rate\n",subs_error);
  fprintf(ofp,"  -indel  [%2.2g] Insertion/deletion error rate\n",indel_error);
  fprintf(ofp,"  -cfreq  [model/flat] Using codon bias or not?     [default flat]\n");
  fprintf(ofp,"  -splice [model/flat] Using splice model or GT/AG? [default model]\n");
  fprintf(ofp,"  -intron [model/tied] Use tied model for introns   [default tied]\n");
  fprintf(ofp,"  -null   [syn/flat]   Random Model as synchronous or flat [default syn]\n");
  fprintf(ofp,"  -pg     <file>       Potential Gene file (heurestic for speeding alignments)\n");
  fprintf(ofp,"  -alln   [%s]   Probability of matching a NNN codon\n",allN_string);
  fprintf(ofp,"  -insert [model/flat] Use protein insert model     [default flat]\n");
  fprintf(ofp,"Algorithm options\n");
  fprintf(ofp,"  -alg    [623/623L/2193/2193L]  Algorithm used [default 623/623L]\n");
  fprintf(ofp,"Output options [default -pretty -para]\n");
  fprintf(ofp,"  -pretty          show pretty ascii output\n");
  fprintf(ofp,"  -pseudo          Mark genes with frameshifts as pseudogenes\n");
  fprintf(ofp,"  -genes           show gene structure\n");
  fprintf(ofp,"  -genesf          show gene structure with supporting evidence\n");
  fprintf(ofp,"  -embl            show EMBL feature format with CDS key\n");
  fprintf(ofp,"  -diana           show EMBL feature format with misc_feature key (for diana)\n");
  fprintf(ofp,"  -para            show parameters\n");
  fprintf(ofp,"  -sum             show summary output\n");

  /* lets not bring trouble on ourselves ;) */
  /* fprintf(ofp,"  -over            show EMBL overlap (only with EMBL format)\n"); */

  fprintf(ofp,"  -cdna            show cDNA\n");
  fprintf(ofp,"  -trans           show protein translation, breaking at frameshifts\n");
  fprintf(ofp,"  -pep             show protein translation, splicing frameshifts\n");
  fprintf(ofp,"  -ace             ace file gene structure\n");
  fprintf(ofp,"  -gff             Gene Feature Format file\n");
  fprintf(ofp,"  -gener           raw gene structure\n");
  fprintf(ofp,"  -alb             show logical AlnBlock alignment\n");
  fprintf(ofp,"  -pal             show raw matrix alignment\n");
  fprintf(ofp,"  -block  [%s]     Length of main block in pretty output\n",main_block_str);
  fprintf(ofp,"  -divide [%s]     divide string for multiple outputs\n",divide_str);

  show_help_GeneModelParam(ofp);

  show_help_DPRunImpl(ofp);
  show_standard_options(ofp);

  fprintf(ofp,"\nSee WWW help at http://www.sanger.ac.uk/Software/Wise2/\n");

  /*
  fprintf(ofp,"\nPotential Gene file looks like\n");
  fprintf(ofp,"pgene # stands for potential gene\n");
  fprintf(ofp,"ptrans # stands for potential transcript\n");
  fprintf(ofp,"pexon <start-in-dna> <end-in-dna> <start-in-protein> <end-in-protein>\n");
  fprintf(ofp,"pexon <start-in-dna> <end-in-dna> <start-in-protein> <end-in-protein>\n");
  fprintf(ofp,"...\n");
  fprintf(ofp,"endptrans\n");
  fprintf(ofp,"<another ptrans if you like>\n");
  fprintf(ofp,"endpgene\n");
  */

  exit(63);   
}

void show_version(FILE * ofp)
{
  fprintf(ofp,"%s\nVersion: %s\nReleased: %s\nCompiled: %s\n",program_name,VERSION_NUMBER,RELEASE_DAY,COMPILE_DATE);
  fprintf(ofp,"\nThis program is freely distributed under a Gnu Public License\n");
  fprintf(ofp,"The source code is copyright (c) GRL 1998 and others\n");
  fprintf(ofp,"There is no warranty, implied or otherwise on the performance of this program\n");
  fprintf(ofp,"For more information read the GNULICENSE file in the distribution\n\n");
  fprintf(ofp,"Credits: Ewan Birney <birney@sanger.ac.uk> wrote the core code.\n");
  fprintf(ofp,"         Portions of this code was from HMMer2, written by Sean Eddy\n");
  exit(63);   
}


int main(int argc,char ** argv) 
{
  int i;
  char * temp;


  build_defaults();
  
  strip_out_standard_options(&argc,argv,show_help,show_version);

  potential_file = strip_out_assigned_argument(&argc,argv,"pg");

  if( (temp = strip_out_assigned_argument(&argc,argv,"gap")) != NULL )
    gap_str = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"g")) != NULL )
    gap_str = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"ext")) != NULL )
    ext_str = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"e")) != NULL )
    ext_str = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"matrix")) != NULL )
    matrix_file = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"m")) != NULL )
    matrix_file = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"s")) != NULL )
    qstart_str = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"t")) != NULL )
    qend_str = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"u")) != NULL )
    tstart_str = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"v")) != NULL )
    tend_str = temp;

  if( (strip_out_boolean_argument(&argc,argv,"trev")) == TRUE )
    reverse = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"new")) == TRUE )
    use_new_stats = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"tfor")) == TRUE ){
    if( reverse == TRUE ) {
      warn("You have specified both trev and tfor. Treating as both");
      do_both = TRUE;
      reverse = FALSE;
    } else {
      reverse = FALSE;
    }
  }

  if( (temp = strip_out_assigned_argument(&argc,argv,"insert")) != NULL ) {
    if( strcmp(temp,"flat") == 0 ) {
      flat_insert = TRUE;
    } else {
      flat_insert = FALSE;
    }
  }

  if( (strip_out_boolean_argument(&argc,argv,"both")) == TRUE )
    do_both = TRUE;

      
  if( (strip_out_boolean_argument(&argc,argv,"fembl")) == TRUE )
    is_embl = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"tabs")) == TRUE )
    target_abs = TRUE;

  pseudo = strip_out_boolean_argument(&argc,argv,"pseudo");

  if( (temp = strip_out_assigned_argument(&argc,argv,"codon")) != NULL )
    codon_file = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"gene")) != NULL )
    gene_file = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"alg")) != NULL )
    alg_str = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"kbyte")) != NULL )
    kbyte_str = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"subs")) != NULL )
    subs_string = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"indel")) != NULL )
    indel_string = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"cfreq")) != NULL )
    cfreq_string = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"splice")) != NULL )
    splice_string = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"init")) != NULL )
    startend_string = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"null")) != NULL )
    null_string = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"intron")) != NULL )
    intron_string = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"alln")) != NULL )
    allN_string = temp;

  if( (strip_out_boolean_argument(&argc,argv,"hmmer")) == TRUE )
    use_tsm = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"intie")) == TRUE )
    use_tied_model = TRUE;

  if( (temp = strip_out_assigned_argument(&argc,argv,"hname")) != NULL )
    hmm_name = temp;


  if( (strip_out_boolean_argument(&argc,argv,"pretty")) != FALSE )
    show_pretty = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"gff")) != FALSE )
    show_gff = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"diana")) != FALSE )
    show_diana = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"embl")) != FALSE )
    show_embl = TRUE;


  if( (strip_out_boolean_argument(&argc,argv,"genes")) != FALSE )
    show_pretty_gene = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"genesf")) != FALSE )
    show_supp_gene = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"para")) != FALSE )
    show_para = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"trans")) != FALSE )
    show_trans = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"pep")) != FALSE )
    show_pep = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"cdna")) != FALSE )
    show_cdna = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"sum")) != FALSE )
    show_match_sum = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"alb")) != FALSE )
    show_AlnBlock = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"ace")) != FALSE )
    show_ace = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"pal")) != FALSE )
    show_PackAln = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"gener")) != FALSE )
    show_gene_plain = TRUE;

  if( (strip_out_boolean_argument(&argc,argv,"over")) != FALSE )
    show_overlap = TRUE;

  if( (temp = strip_out_assigned_argument(&argc,argv,"divide")) != NULL )
    divide_str = temp;

  if( (temp = strip_out_assigned_argument(&argc,argv,"block")) != NULL )
    main_block_str = temp;

  dpri = new_DPRunImpl_from_argv(&argc,argv);

  gmp  = new_GeneModelParam_from_argv(&argc,argv);

  strip_out_remaining_options_with_warning(&argc,argv);
  

  if( argc !=  3 ) {
    warn("Wrong number of arguments (expect 2)!\n");
    if( argc > 1 ){
      warn("Arg line looked like (after option processing)");
      for(i=1;i<argc;i++) {
	fprintf(stderr,"   %s\n",argv[i]);
      }
    }

    show_short_help();
  }

  if( show_embl == FALSE && show_diana == FALSE && show_gff == FALSE && show_overlap == FALSE && show_pretty_gene == FALSE && show_match_sum == FALSE && show_ace == FALSE && show_gene_plain == FALSE && show_pretty == FALSE && show_AlnBlock == FALSE && show_PackAln == FALSE && show_pep == FALSE ) {
    show_pretty = TRUE;
    show_para = TRUE;
  }
 
  dna_seq_file = argv[2];
  if( use_tsm == FALSE) 
    protein_file = argv[1];
  else 
    hmm_file  = argv[1];

  if( build_objects() == FALSE) 
    fatal("Could not build objects!");

  if( show_para == TRUE) {
    show_parameters();
  }

  if( build_alignment() == FALSE)
    fatal("Could not build alignment!");

  if( show_output() == FALSE)
    fatal("Could not show alignment. Sorry!");

  if( do_both == TRUE) {
    reverse_target();

    if( build_alignment() == FALSE)
      fatal("Could not build alignment!");

    if( show_output() == FALSE)
      fatal("Could not show alignment. Sorry!");
  }

  free_temporary_objects();
  free_io_objects();
  return 0;
}




