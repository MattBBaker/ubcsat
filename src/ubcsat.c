/*

      ##  ##  #####    #####   $$$$$   $$$$   $$$$$$    
      ##  ##  ##  ##  ##      $$      $$  $$    $$      
      ##  ##  #####   ##       $$$$   $$$$$$    $$      
      ##  ##  ##  ##  ##          $$  $$  $$    $$      
       ####   #####    #####  $$$$$   $$  $$    $$      
  ======================================================
  SLS SAT Solver from The University of British Columbia
  ======================================================
  ...Developed by Dave Tompkins (davet [@] cs.ubc.ca)...
  ------------------------------------------------------
  .......consult legal.txt for legal information........
  ......consult revisions.txt for revision history......
  ------------------------------------------------------
  ... project website: http://www.satlib.org/ubcsat ....
  ------------------------------------------------------
  .....e-mail ubcsat-help [@] cs.ubc.ca for support.....
  ------------------------------------------------------

*/

#include "mpp/shmem.h"
#include "ubcsat.h"

/* SHMEM globals */
BOOL *rem_aVarValue;
UINT32 *rem_iNumFalse;

#define get_rem_avar(pe,i) (rem_aVarValue + iNumVars*pe +i)
#define get_rem_inum(pe) (rem_iNumFalse + pe)

#ifdef __cplusplus 
namespace ubcsat {
#endif

static push_my_data(void) {
    int i;
    /* Bcast the data */
    for (i = 0; i < num_pes(); i++) {
        /* Push iNumFalse */
        shmem_putmem_nb(get_rem_avar(my_pe(), 0), rem_aVarValue, iNumVars*sizeof(BOOL), i, NULL);
        /* Push aVar*/
        shmem_putmem_nb(get_rem_inum(my_pe()), rem_iNumFalse, sizeof(UINT32), i, NULL);
    }
}

int ubcsatmain(int argc, char *argv[]) {
  
  InitSeed();

  SetupUBCSAT();

  AddAlgorithms();
  AddParameters();
  AddReports();
  AddDataTriggers();
  AddReportTriggers();

  AddLocal();
  
  ParseAllParameters(argc,argv);

  ActivateAlgorithmTriggers();
  ActivateReportTriggers();

  RandomSeed(iSeed);

  RunProcedures(PostParameters);

  RunProcedures(ReadInInstance);

  RunProcedures(PostRead);

  RunProcedures(CreateData);
  RunProcedures(CreateStateInfo);

  iRun = 0;
  iNumSolutionsFound = 0;
  bTerminateAllRuns = 0;

  RunProcedures(PreStart);

  /* SHMEM allocate memory for remote access */
  printf("My id %d Seed %d\n", my_pe(), iSeed);
  /* Vertor of values */
  rem_aVarValue = shmalloc(iNumVars * sizeof(BOOL) * num_pes());
  if (rem_aVarValue == NULL) {
      fprintf(stderr, "Failed to allocate memory for rem_aVarValue\n"); fflush(stderr);
      shmem_global_exit(1);
  }
  /* Cost of the current solution */
  rem_iNumFalse = shmalloc(sizeof(UINT32) * num_pes());
  if (rem_iNumFalse == NULL) {
      fprintf(stderr, "Failed to allocate memory for rem_iNumFalse\n"); fflush(stderr);
      shmem_global_exit(1);
  }
  /* SHMEM sync */
  shmem_barrier_all();

  StartTotalClock();

  while ((iRun < iNumRuns) && (! bTerminateAllRuns)) {

    iRun++;

    iStep = 0;
    bSolutionFound = 0;
    bTerminateRun = 0;
    bRestart = 1;

    RunProcedures(PreRun);

    StartRunClock();
    
    while ((iStep < iCutoff) && (! bSolutionFound) && (! bTerminateRun)) {

      iStep++;
      iFlipCandidate = 0;

      RunProcedures(PreStep);
      RunProcedures(CheckRestart);

      if (bRestart) {
        RunProcedures(PreInit);
        RunProcedures(InitData);
        RunProcedures(InitStateInfo);
        RunProcedures(PostInit);
        bRestart = 0;
      } else {
        RunProcedures(ChooseCandidate);
        RunProcedures(PreFlip);
        RunProcedures(FlipCandidate);
        RunProcedures(UpdateStateInfo);
        RunProcedures(PostFlip);
      }
      
      RunProcedures(PostStep);

      RunProcedures(StepCalculations);

      RunProcedures(CheckTerminate);
    }

    StopRunClock();

    RunProcedures(RunCalculations);
    
    RunProcedures(PostRun);

    if (bSolutionFound) {
      iNumSolutionsFound++;
      if (iNumSolutionsFound == iFind) {
        bTerminateAllRuns = 1;
      }
    }
  }

  StopTotalClock();
  /* SHMEM exit */
  printf("====> Solved by PE %d found %d\n", my_pe(), iNumSolutionsFound);fflush(stdout);

  RunProcedures(FinalCalculations);

  RunProcedures(FinalReports);

  /* SHMEM exit */
  shmem_global_exit(iNumSolutionsFound > 0 ? 0:1);
  CleanExit();

  return(0);
  
}

char *myargv[100];
int myargc = 0;

int sparrowmain(int argc, char *argv[]) {

  int seed;
  char b_seed[128];

  if (argc != 3) {
    printf("ERROR Competition build requires 2 (and only 2) parameters: filename.cnf and seed\n");
    exit(0);
  }

  myargv[myargc++] = argv[0];

  myargv[myargc++] = "-i";
  myargv[myargc++] = argv[1];

  myargv[myargc++] = "-seed";
  seed = atoi(argv[2]) * (1+my_pe());
  sprintf(b_seed, "%d", seed);
  myargv[myargc++] = b_seed;

  myargv[myargc++] = "-q";

  myargv[myargc++] = "-r";
  myargv[myargc++] = "satcomp";

  myargv[myargc++] = "-cutoff";
  myargv[myargc++] = "max";

  myargv[myargc++] = "-alg";
  myargv[myargc++] = "sparrow";

  myargv[myargc++] = "-v";
  myargv[myargc++] = "sat11";

  return(ubcsatmain(myargc,myargv));
}

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

int main(int argc, char *argv[]) {
  return(ubcsat::sparrowmain(argc,argv));
}

#else

int main(int argc, char *argv[]) {
  /* SHMEM bootstrap */
  start_pes(0);
  return(sparrowmain(argc,argv));
}

#endif
