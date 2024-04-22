#define  _POSIX_C_SOURCE 200809L
#define  _XOPEN_SOURCE 500L

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <ftw.h>

#include "aimmsapi.h"

#ifndef AimmsLoggerInitFromConfigfile
#define LOGGER_CONFIG_NOT_SUPPORTED
#warning "this AIMMS version does not support configuring the logger from outside AIMMS" 
#else 
#define LOGGER_CONFIG_SUPPORTED
#endif

/* Call-back to the 'remove()' function called by nftw() */
static int remove_callback(const char *pathname,
                __attribute__((unused)) const struct stat *sbuf,
                __attribute__((unused)) int type,
                __attribute__((unused)) struct FTW *ftwb)
{
    return remove (pathname);
}


int CheckAPIError(int retval, const char *intro)
{
	if (retval != AIMMSAPI_SUCCESS) {
		int errorCode = 0;
		char errorMessage[2048];
		AimmsAPILastErrorA(&errorCode, errorMessage);
        fprintf(stderr, "error: %s: %s\n", intro, errorMessage);
		exit(-1);
	}
	return 0;
}

int show_usage(int retval) {
    fprintf(stderr, "usage: AimmsJobRunner [options] AimmsModel.aimms(pack) [arg1 ... argn]\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "with options:\n");
    fprintf(stderr, "   -l --licfolder   folder where the license-configuration folders can be found, defaults to /usr/local/Aimms\n");
#ifdef LOGGER_CONFIG_SUPPORTED
    fprintf(stderr, "   -c --logconfig   file that specifies the log4cxx logging configuration, defaults to writing to stdout\n");
#endif
    fprintf(stderr, "   -m --maxthreads  maximum number of threads to use, defaults to the number of detected CPUs\n");
    fprintf(stderr, "   -p --procedure   name of the AIMMS procedure to run, defaults to MainExecution\n");
    fprintf(stderr, "   --keeplog        flag that cause some intermediate logfiles to not be deleted, can be useful for debugging\n");
    fprintf(stderr, "   all arguments after the modelfile will be passed on to AIMMS\n");
    return retval;
}

typedef struct aimms_options_t {
    const char* license_folder;
    const char* log_config_name;
    const char* procedure_name;
    const char* max_threads;
    int keep_aimms_logfolder;
} aimms_options;

int run_aimms(aimms_options* opts, int argc, char* argv[]) {
    int index = 0;
    const char* procedure_name = "MainExecution";
    const char* default_args[] = {
        "--as-server", "--hidden", "--end-user", "--ignore-dialogs"
    };
    int n_default_args = sizeof(default_args) / sizeof(default_args[0]);

    char aimms_cmd_line[2048];

    int project_handle = 0;
    int retval = 0;
    int procedure_handle = 0, arg_count = 0, arg_types[AIMMSAPI_MAX_NUMB_OF_PROC_ARG];
    int procedure_return = 0;
    char tmp_dir_template[] = "/tmp/aimms.XXXXXX";
    char* tmp_dirname = 0;


    // create a temporary folder for AIMMS internal logs
    tmp_dirname = mkdtemp (tmp_dir_template);
    if (tmp_dirname == 0)
    {
       perror ("tempdir: error: Could not create tmp directory");
       exit (EXIT_FAILURE);
    }

#ifdef LOGGER_CONFIG_SUPPORTED    
    // configure log4cxx
    if (opts->log_config_name) {
        AimmsLoggerInitFromConfigfileA(opts->log_config_name);
    }
    else {
        AimmsLoggerInitWriteToConsole();
    }
#endif

    // construct cmdline to pass to AIMMS
    aimms_cmd_line[0] = 0;
    for (index = 0; index < n_default_args; index++) {
        strcat(aimms_cmd_line, " ");
        strcat(aimms_cmd_line, default_args[index]);
    }
    // add temporary logging folder
    strcat(aimms_cmd_line, " --log-dir ");
    strcat(aimms_cmd_line, tmp_dirname);
    
    // add maximum threads
    if (opts->max_threads) {
        strcat(aimms_cmd_line, " --max-threads ");
        strcat(aimms_cmd_line, opts->max_threads);
    }
    
    // add aimms-root
    if (opts->license_folder){
        strcat(aimms_cmd_line, " --aimms-root ");
        strcat(aimms_cmd_line, opts->license_folder);
    }
    
    // add remainder of the arguments (including project name)
    for (index = optind; index < argc; index++) {
        strcat(aimms_cmd_line, " \"");
        strcat(aimms_cmd_line, argv[index]);
        strcat(aimms_cmd_line, "\"");
    }

    // open the AIMMS project
    fprintf(stdout, "Opening AIMMS project: %s\n", aimms_cmd_line);
    retval = AimmsProjectOpenA(aimms_cmd_line, &project_handle);
    CheckAPIError(retval, "Could not open AIMMS project");

    // open the AIMMS procedure for running
    if (opts->procedure_name) {
        procedure_name = opts->procedure_name;
    }
    retval = AimmsProcedureHandleCreateA((char*)procedure_name, &procedure_handle, &arg_count, arg_types);
    CheckAPIError(retval, "Cannot open procedure in AIMMS project");

    // run the procedure
    fprintf(stdout, "Running procedure: %s\n", procedure_name);
    retval = AimmsProcedureRunA(procedure_handle, 0, 0, &procedure_return);
    CheckAPIError(retval, "Running AIMMS project procedure failed");
    fprintf(stdout, "Procedure finished with return value 0x%08x\n", procedure_return);

    // free up the handle
    retval = AimmsProcedureHandleDelete(procedure_handle);
    CheckAPIError(retval, "Closing AIMMS project procedure failed");

    // free up the project_handle
    retval = AimmsProjectClose(project_handle, 0);
    CheckAPIError(retval, "Closing AIMMS project failed");
    
    if (opts->keep_aimms_logfolder == 0){
        if (nftw (tmp_dirname, remove_callback, FOPEN_MAX, FTW_DEPTH | FTW_MOUNT | FTW_PHYS) == -1)
        {
            perror("tempdir: error: ");
            exit(EXIT_FAILURE);
        }
    }

    return procedure_return;
}
static int keep_aimms_logfolder = 0;

int main(int argc, char* argv[])
{
    int c = 0;
    aimms_options opts;

    // set the defaults
    opts.keep_aimms_logfolder = 0;
    opts.license_folder = 0;
    opts.log_config_name = 0;
    opts.max_threads = 0;
    opts.procedure_name = 0;
    static struct option long_options[] =
    {
        {"keeplog",     no_argument,       &keep_aimms_logfolder, 1},
        {"licfolder",   required_argument, 0, 'l'},
#ifdef LOGGER_CONFIG_SUPPORTED
        {"logconfig",   required_argument, 0, 'c'},
#endif
        {"maxthreads",  required_argument, 0, 'm'},
        {"procedure",   required_argument, 0, 'p'},
        {0, 0, 0, 0}
    };
#ifdef LOGGER_CONFIG_SUPPORTED
	const char* short_options = "l:c:m:p:";
#else
	const char* short_options = "l:m:p:";
#endif

    while (1)
    {
        int option_index = 0;

        c = getopt_long (argc, argv, short_options, long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            if (long_options[option_index].flag != 0)
                break;
            break;

        case 'l':
            opts.license_folder = optarg;
            break;

        case 'c':
            opts.log_config_name = optarg;
            break;

        case 'm':
            opts.max_threads = optarg;
            break;

        case 'p':
            opts.procedure_name = optarg;
            break;

        case '?':
            return show_usage(1);
            break;

        default:
            return show_usage(1);
            break;
        }
    }
    opts.keep_aimms_logfolder = keep_aimms_logfolder;
	
	// we need at least the main AIMMS model
	if (optind >= argc){
		return show_usage(1);
	}
	
    return run_aimms(&opts,argc,argv);
}

