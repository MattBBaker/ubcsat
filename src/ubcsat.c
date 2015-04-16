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
#include <assert.h>

/* SHMEM global exit */
#if defined(SGI) || defined(CRAY_704)
#include <mpi.h>
#define UBCSAT_GLOBAL_INIT() MPI_Init(NULL, NULL); //start_pes(0)
#define UBCSAT_GLOBAL_EXIT(r) MPI_Abort(MPI_COMM_WORLD, r)
#else
#define UBCSAT_GLOBAL_INIT()  start_pes(0)
#define UBCSAT_GLOBAL_EXIT(r) shmem_global_exit(r)
#endif

#define DEBUG 0

/* SHMEM globals */
BOOL *rem_aVarValue;
UINT32 *rem_iNumFalse;
long long *rem_counter;
long long *cached_counter;
BOOL      *flags_counter;
double *NormW;
double *Prop;

#define get_rem_avar(pe,i) (rem_aVarValue + iNumVars*pe +i)
#define get_rem_inum(pe) (rem_iNumFalse + pe)

#ifdef __cplusplus 
namespace ubcsat {
#endif

static int calc_NormW() 
{
    int num_pes = shmem_n_pes();
    double magnitude = 0;
    double NormWsum = 0;
    double VarSum = 0;
    double Var = 0;
    int num_updates = 0;
    int i, v;

    for (i = 0; i < num_pes; i++) {
#if DEBUG 
        fprintf(stdout,"[%d] Counters %lld %lld rem_iNumFalse %d\n",shmem_my_pe(), cached_counter[i],rem_counter[i], rem_iNumFalse[i]); fflush(stdout);
#endif
        if (cached_counter[i] < rem_counter[i]) {
            /* cache it */
            /* I kinda assume here that we don't have double updates
             * It has to be converted to critical section later on */
            //cached_counter[i] = rem_counter[i];
            flags_counter[i] = 1;
            ++num_updates;
#if DEBUG
            fprintf(stdout,"[%d][%d] > rem_iNumFalse %d\n",
                    shmem_my_pe(), i, rem_iNumFalse[i]); fflush(stdout);
#endif
            magnitude += pow(rem_iNumFalse[i], 2);
        } else {
            flags_counter[i] = 0;
        }
    }

    magnitude = sqrt(magnitude);

    if (num_updates < 2) {
        return 0;
    }

    
    /* Check if we can read the value */
    for (i = 0; i < num_pes; i++) {
        if (flags_counter[i]) {
            cached_counter[i] = rem_counter[i];
            NormW[i] = (double)(magnitude - rem_iNumFalse[i])/magnitude;
#if DEBUG
            fprintf(stdout,"[%d][%d] >> mag %f norm %f rem_iNumFalse %d\n",
                    shmem_my_pe(), i, magnitude, NormW[i], rem_iNumFalse[i]); fflush(stdout);
#endif
        }
    }

    for (i = 0; i < num_pes; i++) {
        if (flags_counter[i]) {
            NormWsum += NormW[i];
        }
    }

#if 1
    fprintf(stdout,"[%d] Number of updates %d mag %f norm %f\n",shmem_my_pe(), num_updates, magnitude, NormWsum); fflush(stdout);
#endif

    for (v = 0; v < iNumVars; v++) {
        Var = 0;
        for (i = 0; i < num_pes; i++) {
            if (flags_counter[i]) {
                Var += (*get_rem_avar(i, v)) * NormW[i];
            }
        }
        Var = Var / NormWsum;
        assert(Var >= 0 && Var <= 1);
        if (Prop[v] != Var) {
            Prop[v] = Var;
        }
    }

    return 1;
}

static push_my_data(void) {
    int i;
#if DEBUG
    fprintf(stdout, "push_my_data\n"); fflush(stdout);
#endif
    /* Bcast the data */
    for (i = 0; i < shmem_n_pes(); i++) {
        /* Push iNumFalse */
#if defined(SGI) || defined(CRAY_704)
        shmem_putmem(get_rem_avar(shmem_my_pe(), 0), aVarValue, iNumVars*sizeof(BOOL), i);
#if DEBUG
        fprintf(stdout, "from %d to %d Data %d\n", shmem_my_pe(),i, aVarValue[0]); fflush(stdout);
#endif
        shmem_putmem(get_rem_inum(shmem_my_pe()), &iNumFalse, sizeof(UINT32), i);
#if DEBUG
        fprintf(stdout, "from %d to %d False %d\n", shmem_my_pe(),i, iNumFalse); fflush(stdout);
#endif
#else
        shmem_putmem_nb(get_rem_avar(shmem_my_pe(), 0), aVarValue, iNumVars*sizeof(BOOL), i, NULL);
#if DEBUG
        fprintf(stdout, "%d Data %d\n", i, aVarValue); fflush(stdout);
#endif
        shmem_putmem_nb(get_rem_inum(shmem_my_pe()), &iNumFalse, sizeof(UINT32), i, NULL);
#endif
        /* Anounce update */
        shmem_fence();
        shmem_longlong_inc(&rem_counter[shmem_my_pe()], i);
    }
}

/* Sync the data */
static sync_data(void) {
    shmem_quiet();
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

  /* Init Norm/Prob vectors */
  NormW = calloc(sizeof(double), shmem_n_pes());
  if (NormW == NULL) {
      fprintf(stderr, "Failed to allocate memory for NormW\n"); fflush(stderr);
      UBCSAT_GLOBAL_EXIT(1);
  }

  Prop = calloc(sizeof(double), iNumVars);
  if (Prop == NULL) {
      fprintf(stderr, "Failed to allocate memory for Prop\n"); fflush(stderr);
      UBCSAT_GLOBAL_EXIT(1);
  }

  cached_counter = calloc(sizeof(long long), shmem_n_pes());
  if (cached_counter == NULL) {
      fprintf(stderr, "Failed to allocate memory for cached_counter\n"); fflush(stderr);
      UBCSAT_GLOBAL_EXIT(1);
  }

  flags_counter = calloc(sizeof(BOOL), iNumVars);
  if (flags_counter == NULL) {
      fprintf(stderr, "Failed to allocate memory for flags_counter\n"); fflush(stderr);
      UBCSAT_GLOBAL_EXIT(1);
  }

  /* SHMEM allocate memory for remote access */
  //printf("My id %d Seed %d\n", shmem_my_pe(), iSeed);

  /* Vertor of values */
  rem_aVarValue = shmalloc(iNumVars * sizeof(BOOL) * shmem_n_pes());
  if (rem_aVarValue == NULL) {
      fprintf(stderr, "Failed to allocate memory for rem_aVarValue\n"); fflush(stderr);
      UBCSAT_GLOBAL_EXIT(1);
  }
  memset(rem_aVarValue, 0, sizeof(BOOL) * shmem_n_pes());

  /* Cost of the current solution */
  rem_iNumFalse = shmalloc(sizeof(UINT32) * shmem_n_pes());
  if (rem_iNumFalse == NULL) {
      fprintf(stderr, "Failed to allocate memory for rem_iNumFalse\n"); fflush(stderr);
      UBCSAT_GLOBAL_EXIT(1);
  }
  memset(rem_iNumFalse, 0, sizeof(UINT32) * shmem_n_pes());

  /* Signal that data is there */
  rem_counter = shmalloc(sizeof(long long) * shmem_n_pes());
  if (rem_counter == NULL) {
      fprintf(stderr, "Failed to allocate memory for rem_counter\n"); fflush(stderr);
      UBCSAT_GLOBAL_EXIT(1);
  }
  memset(rem_counter, 0, sizeof(long long) * shmem_n_pes());
  /* SHMEM sync */
  shmem_barrier_all();

  StartTotalClock();

#if SHMEM_PROP
  iNumRuns = iCutoff;
  iFind = 1;
#endif

  while ((iRun < iNumRuns) && (! bTerminateAllRuns)) {

    iRun++;

    iStep = 0;
    bSolutionFound = 0;
    bTerminateRun = 0;
    bRestart = 1;

    RunProcedures(PreRun);

#if SHMEM_PROP 
    // worked well iCutoff = 10000000;
    //iCutoff = 1000000;
    //iCutoff = 500000;
    iCutoff = 400000;
    //iCutoff = 400000;
    //iCutoff = 4000000;
#endif
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
      
#if SHMEM_PROP 
      // fprintf(stdout, "bbb %d %d\n", iStep, (int)(0.7*iCutoff));
      if (iStep == (int)(0.7*iCutoff)) {
          //fprintf(stdout, "AAAAA %d %d\n", iStep, (int)(0.7*iCutoff));
          push_my_data();
      }
#endif
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
#if SHMEM_PROP
    sync_data();
#if DEBUG
    fprintf(stdout,"Step %d bSolutionFound %d bTerminateRun %d iFind %d\n", iStep, bSolutionFound, bTerminateRun, iFind); fflush(stdout);
#endif
    if (calc_NormW()) {
        bRestart = 1;
    } else {
        bRestart = 0;
    }
#endif
  }

  StopTotalClock();
  /* SHMEM exit */
  printf("====> Solved by PE %d found %d\n", shmem_my_pe(), iNumSolutionsFound);fflush(stdout);

  RunProcedures(FinalCalculations);

  RunProcedures(FinalReports);

  /* SHMEM exit */
  UBCSAT_GLOBAL_EXIT(iNumSolutionsFound > 0 ? 0:1);
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
  seed = atoi(argv[2]) * (1+shmem_my_pe());
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
  UBCSAT_GLOBAL_INIT();
  return(sparrowmain(argc,argv));
}

#endif
