/*----------------------------------------------------------------------------
 *
 * Description:
 *      QMP intialize code for single node build
 *
 * Author:  
 *      James C. Osborn
 *
 * Revision History:
 *   $Log: not supported by cvs2svn $
 *   Revision 1.4  2005/06/29 19:44:32  edwards
 *   Removed ANSI-99-isms. Now compiles under c89.
 *
 *   Revision 1.3  2005/06/21 20:18:39  osborn
 *   Added -qmp-geom command line argument to force grid-like behavior.
 *
 *   Revision 1.2  2005/06/20 22:20:59  osborn
 *   Fixed inclusion of profiling header.
 *
 *   Revision 1.1  2004/10/08 04:49:34  osborn
 *   Split src directory into include and lib.
 *
 *   Revision 1.1  2004/06/14 20:36:31  osborn
 *   Updated to API version 2 and added single node target
 *
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define __USE_UNIX98 /* needed to get gethostname from GNU unistd.h */
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

#include "QMP_P_SINGLE.h"

/**
 * This machine information
 */
static struct QMP_machine par_machine = {.inited=QMP_FALSE};
QMP_machine_t QMP_global_m = &par_machine;

static struct QMP_logical_topology par_logical_topology;
QMP_logical_topology_t QMP_topo = &par_logical_topology;


/**
 * Populate this machine information.
 */
static void
QMP_init_machine_i(int* argc, char*** argv)
{
  /* get host name of this machine */
  gethostname (QMP_global_m->host, sizeof (QMP_global_m->host));

  int first=-1, last=-1;
  int i, nd, n;
  for(i=0; i<*argc; i++) {
    if(strcmp((*argv)[i], "-qmp-geom")==0) {
      first = i;
      while( (++i<*argc) && (isdigit((*argv)[i][0])) );
      last = i-1;
    }
  }

  nd = last - first;
  if(nd<=0) {
    QMP_global_m->ic_type = QMP_SWITCH;
    QMP_global_m->ndim = 0;
    QMP_global_m->geom = NULL;
    QMP_global_m->coord = NULL;
  } else { /* act like a mesh */
    QMP_global_m->ic_type = QMP_MESH;
    QMP_global_m->ndim = nd;
    QMP_global_m->geom = (int *) malloc(nd*sizeof(int));
    QMP_global_m->coord = (int *) malloc(nd*sizeof(int));
    n = QMP_global_m->nodeid;
    for(i=0; i<nd; i++) {
      QMP_global_m->geom[i] = atoi((*argv)[i+first+1]);
      QMP_global_m->coord[i] = n % QMP_global_m->geom[i];
      n /= QMP_global_m->geom[i];
    }
  }
  if(first>=0) {  /* remove from arguments */
    for(i=last+1; i<*argc; i++) (*argv)[i-nd-1] = (*argv)[i];
    *argc -= nd + 1;
  }
}

/* Initialize QMP */
QMP_status_t
QMP_init_msg_passing (int* argc, char*** argv, QMP_thread_level_t required,
                      QMP_thread_level_t *provided)
{
  QMP_global_m->num_nodes = 1;
  QMP_global_m->nodeid = 0;
  QMP_global_m->verbose = 0;
  QMP_global_m->proflevel = 0;
  QMP_global_m->inited = QMP_TRUE;
  QMP_global_m->err_code = QMP_SUCCESS;

  QMP_topo->topology_declared = QMP_FALSE;

  QMP_init_machine_i(argc, argv);

  return QMP_global_m->err_code;
}

/* Shutdown the machine */
void
QMP_finalize_msg_passing(void)
{
  if(!QMP_global_m->inited) {
    QMP_FATAL("QMP_finalize_msg_passing called but QMP is not initialized!");
  }
  QMP_global_m->inited = QMP_FALSE;
}

/* Abort the program */
void 
QMP_abort(int error_code)
{
  fprintf(stderr, "node 0/1: QMP Aborted!\n");
  exit(error_code);
}

/* Print string and abort the program */
void
QMP_abort_string(int error_code, char *message)
{
  fprintf(stderr, message);
  fprintf(stderr, "\n");
  fprintf(stderr, "node 0/1: QMP Aborted!\n");
  exit(error_code);
}
