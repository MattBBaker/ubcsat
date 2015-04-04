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

#ifdef __cplusplus 
namespace ubcsat {
#endif

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

  printf("My id %d Seed %d\n", my_pe(), iSeed);
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
