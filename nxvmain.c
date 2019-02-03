/**
  * This is the command line application for nxvalidate
	* implemented in C
	*
	* Copyright: GPL
	*
	* Mark Koennecke, mark.koennecke@psi.ch, and the NIAC
	*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <nxvalidate.h>

/*------------------------------------------------------------------*/
typedef struct {
	int warnOpt;
	int debug;
	int warnUndefined;
	int warnBase;
	int neat;
}PrintFilter;
/*------------------------------------------------------------------*/
static void defaultLogPrint(char *key, void *data)
{
	if(strchr((char *)data, ' ') != NULL){
		fprintf(stdout,"%s=\"%s\" ", key, (char *)data);
	} else {
		fprintf(stdout,"%s=%s ", key, (char *)data);
	}
}
/*------------------------------------------------------------------*/
static void defaultneatLogPrint(char *key, void *data)
{

        if(!strcmp(key,"sev") || !strcmp(key,"nxdlPath") ){
		fprintf(stdout,"\n... ");
        }
	if(strchr((char *)data, ' ') != NULL){
		fprintf(stdout,"%s=\"%s\" ", key, (char *)data);
	} else {
		fprintf(stdout,"%s=%s ", key, (char *)data);
	}
}
/*-----------------------------------------------------------------*/
static void FilteringLogger(hash_table *logData, void *userData)
{
	PrintFilter *filt = (PrintFilter *)userData;
	char *sev;

	sev = hash_lookup("sev",logData);
	if(sev != NULL){
		if(strcmp(sev,"debug") == 0 && filt->debug == 0){
			return;
		}
		if(strcmp(sev,"warnopt") == 0 && filt->warnOpt == 0){
			return;
		}
		if(strcmp(sev,"warnbase") == 0 && filt->warnBase == 0){
			return;
		}
		if(strcmp(sev,"warnundef") == 0 && filt->warnUndefined== 0){
			return;
		}
	}

        if (filt->neat ==  1) {
	  hash_enumerate(logData,defaultneatLogPrint);
	  fprintf(stdout,"\n");
        } else {
	  hash_enumerate(logData,defaultLogPrint);
        }
	fprintf(stdout,"\n");
}
/*----------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	pNXVcontext nxvContext = NULL;
  char *defDir = strdup("/usr/local/lib/nexus_definitions");
	char *appDef = NULL, *hdf5Path = NULL;
	int warnOpt = 0, warnBase = 0, warnUndefined = 0;
  char c;
  int status = 0, errCount = 0, warnCount = 0;
	PrintFilter filt;

	/*
		should all be 0 in production
	*/
	filt.warnOpt = 0;
	filt.warnBase = 0;
	filt.warnUndefined = 0;
	filt.debug = 0;
	filt.neat = 0;


	while ((c = getopt (argc, argv, "a:l:p:obdute")) != -1) {
    switch (c)
    {
      case 'e':
        filt.neat = 1;
        break;
      case 'o':
        filt.warnOpt = 1;
        break;
      case 'b':
        filt.warnBase = 1;
        break;
      case 'u':
        filt.warnUndefined = 1;
        break;
      case 'd':
	filt.debug = 1;
	break;
      case 't':
	filt.debug = 1;
	filt.warnUndefined = 1;
	filt.warnBase = 1;
	filt.warnOpt = 1;
	break;
      case 'a':
	appDef = strdup(optarg);
	break;
      case 'p':
	hdf5Path = strdup(optarg);
	break;
      case 'l':
	free(defDir);
	defDir = strdup(optarg);
	break;
      case '?':
        if (optopt == 'a' ||  optopt == 'l' || optopt == 'p')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
    }
	}

	if(argc <= optind){
		fprintf(stderr,"ERROR: no data file to validate specified\n");
		fprintf(stderr,"Usage:\n\tnxvvalidate -a appdef -l appdefdir -p pathtovalidate -o -b -u -d -t datafile\n");
		exit(1);
	}

	nxvContext = NXVinit(defDir);
	if(nxvContext == NULL){
		fprintf(stdout,"ERROR: failed to allocate validation context\n");
		exit(1);
	}
	NXVsetLogger(nxvContext, FilteringLogger, &filt);
  status =  NXVvalidate(nxvContext, argv[optind], appDef, hdf5Path);
	NXVgetCounters(nxvContext, &errCount, &warnCount);
	if(status == 0){
		fprintf(stdout,"%s passed validation\n", argv[optind]);
	} else {
		fprintf(stdout,"%d errors and %d warnings found when validating %s\n",
					errCount, warnCount, argv[optind]);
	}
	NXVkill(nxvContext);
	return(status);
}
