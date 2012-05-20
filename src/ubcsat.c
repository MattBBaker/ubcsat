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

#include "ubcsat.h"

#ifdef __cplusplus 
namespace ubcsat {
#endif

int ubcsatmain(int argc, char *argv[]) {

  StartAbsoluteClock();
  
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

  RunProcedures(FinalCalculations);

  RunProcedures(FinalReports);

  CleanExit();

  return(0);
  
}

char *myargv[100];
int myargc = 0;

int irotsmain(int argc, char *argv[]) {

  if (argc != 2) {
    printf("ERROR Competition build requires 1 (and only 1) parameter:\n   filename.cnf or filename.wcnf\n");
    exit(0);
  }

  myargv[myargc++] = argv[0];

  myargv[myargc++] = "-i";
  myargv[myargc++] = argv[1];

  myargv[myargc++] = "-q";

  myargv[myargc++] = "-r";
  myargv[myargc++] = "maxsatcomp";
  myargv[myargc++] = "stdout";
  myargv[myargc++] = "240";

  myargv[myargc++] = "-cutoff";
  myargv[myargc++] = "max";

  myargv[myargc++] = "-abstime";
  myargv[myargc++] = "-systime";
  myargv[myargc++] = "-gtimeout";
  myargv[myargc++] = "295";

  myargv[myargc++] = "-alg";
  myargv[myargc++] = "irots";
  myargv[myargc++] = "-w";

  return(ubcsatmain(myargc,myargv));
}

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

int main(int argc, char *argv[]) {
  return(ubcsat::irotsmain(argc,argv));
}

#else

int main(int argc, char *argv[]) {
  return(irotsmain(argc,argv));
}

#endif
