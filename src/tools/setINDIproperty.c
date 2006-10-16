/* connect to an INDI server and set one or more device.property.element.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "indiapi.h"
#include "lilxml.h"

static void usage (void);
static void crackSpec (char *spec);
static void openINDIServer(FILE **rfpp, FILE **wfpp);
static void listenINDI (FILE *rfp, FILE *wfp);
static int finished (void);
static void onAlarm (int dummy);
static int readServerChar (FILE *fp);
static void findSet (XMLEle *root, FILE *fp);

/* table of INDI definition elements we can set */
typedef struct {
    char *defType;			/* defXXXVector name */
    char *defOne;			/* defXXX name */
    char *newType;			/* newXXXVector name */
    char *oneType;			/* oneXXX name */
} INDIDef;
static INDIDef defs[] = {
    {"defTextVector",   "defText",   "newTextVector",   "oneText"},
    {"defNumberVector", "defNumber", "newNumberVector", "oneNumber"},
    {"defSwitchVector", "defSwitch", "newSwitchVector", "oneSwitch"},
};
#define	NDEFS	(sizeof(defs)/sizeof(defs[0]))

#define INDIPORT	7624		/* default port */
static char host_def[] = "localhost";	/* default host name */

static char *me;			/* our name for usage message */
static char *host = host_def;		/* working host name */
static int port = INDIPORT;		/* working port number */
static int verbose;			/* report extra info */
static int directfd = -1;		/* direct filedes to server, if >= 0 */
#define TIMEOUT         2               /* default timeout, secs */
static int timeout = TIMEOUT;           /* working timeout, secs */
static LilXML *lillp;			/* XML parser context */

typedef struct {
    char *e, *v;			/* element name and value */
    int ok;				/* set when found */
} SetEV;

typedef struct {
    char *d;				/* device */
    char *p;				/* property */
    SetEV *ev;				/* elements */
    int nev;				/* n elements */
} SetSpec;

static SetSpec *sets;			/* set of properties to set */
static int nsets;

static void scanEV (SetSpec *sp, char e[], char v[]);
static void sendNew (FILE *fp, INDIDef *dp, SetSpec *sp);

int
main (int ac, char *av[])
{
	FILE *rfp, *wfp;

	/* save our name */
	me = av[0];

	/* crack args */
	while (--ac && **++av == '-') {
	    char *s = *av;
	    while (*++s) {
		switch (*s) {
		case 'd':
		    if (ac < 2) {
			fprintf (stderr, "-d requires open fileno\n");
			usage();
		    }
		    directfd = atoi(*++av);
		    ac--;
		    break;
		case 'h':
		    if (directfd >= 0) {
			fprintf (stderr, "Can not combine -d and -h\n");
			usage();
		    }
		    if (ac < 2) {
			fprintf (stderr, "-h requires host name\n");
			usage();
		    }
		    host = *++av;
		    ac--;
		    break;
		case 'p':
		    if (directfd >= 0) {
			fprintf (stderr, "Can not combine -d and -p\n");
			usage();
		    }
		    if (ac < 2) {
			fprintf (stderr, "-p requires tcp port number\n");
			usage();
		    }
		    port = atoi(*++av);
		    ac--;
		    break;
		case 't':
		    if (ac < 2) {
			fprintf (stderr, "-t requires timeout\n");
			usage();
		    }
		    timeout = atoi(*++av);
		    ac--;
		    break;
		case 'v':	/* verbose */
		    verbose++;
		    break;
		default:
		    fprintf (stderr, "Unknown flag: %c\n", *s);
		    usage();
		}
	    }
	}

	/* now ac args starting at av[0] */
	if (ac < 1)
	    usage();

	/* crack each property, add to sets[]  */
	while (ac--)
	    crackSpec(*av++);

	/* open connection */
	if (directfd >= 0) {
	    rfp = fdopen (directfd, "r");
	    wfp = fdopen (directfd, "w");
	    if (!rfp || !wfp) {
		fprintf (stderr, "Direct fd %d: %s\n",directfd,strerror(errno));
		exit(1);
	    }
	    if (verbose)
		fprintf (stderr, "Using direct fd %d\n", directfd);
	} else {
	    openINDIServer(&rfp, &wfp);
	    if (verbose)
		fprintf (stderr, "Connected to %s on port %d\n", host, port);
	}

	/* build a parser context for cracking XML responses */
	lillp = newLilXML();

	/* issue getProperties */
	if (verbose)
	    fprintf (stderr, "Querying for properties\n");
	fprintf(wfp, "<getProperties version='%g'/>\n", INDIV);
	fflush (wfp);

	/* listen for properties, set when see any we recognize */
	listenINDI(rfp, wfp);

	return (0);
}

static void
usage()
{
	fprintf(stderr, "Purpose: set one or more writable INDI properties\n");
	fprintf(stderr, "%s\n", "$Revision: 1.2 $");
	fprintf(stderr, "Usage: %s [options] device.property.e1[;e2...]=v1[;v2...] ...\n",
									    me);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -d f  : use file descriptor f already open to server\n");
	fprintf(stderr, "  -h h  : alternate host, default is %s\n", host_def);
	fprintf(stderr, "  -p p  : alternate port, default is %d\n", INDIPORT);
	fprintf(stderr, "  -t t  : max time to wait, default is %d secs\n",TIMEOUT);
	fprintf(stderr, "  -v    : verbose (cumulative)\n");
	fprintf(stderr, "Exit status:\n");
	fprintf(stderr, "  0: all settings successful\n");
	fprintf(stderr, "  1: at least one setting was invalid\n");
	fprintf(stderr, "  2: real trouble, try repeating with -v\n");

	exit (2);
}

/* crack property set spec, add to sets [] */
static void
crackSpec (char *spec)
{
	char d[1024], p[1024], e[2048], v[2048];

	/* scan */
	if (sscanf (spec, "%[^.].%[^.].%[^.=]=%s", d, p, e, v) != 4) {
	    fprintf (stderr, "Bad format: %s\n", spec);
	    usage();
	}

	/* add to list */
	if (!sets)
	    sets = (SetSpec *) malloc (1);		/* seed realloc */
	sets = (SetSpec *) realloc (sets, (nsets+1)*sizeof(SetSpec));
	sets[nsets].d = strcpy (malloc(strlen(d)+1), d);
	sets[nsets].p = strcpy (malloc(strlen(p)+1), p);
	sets[nsets].ev = (SetEV *) malloc (1);		/* seed realloc */
	sets[nsets].nev = 0;
	scanEV (&sets[nsets++], e, v);
}

/* open a read and write connection to host and port or die.
 * exit if trouble.
 */
static void
openINDIServer (FILE **rfpp, FILE **wfpp)
{
	struct sockaddr_in serv_addr;
	struct hostent *hp;
	int sockfd;

	/* lookup host address */
	hp = gethostbyname (host);
	if (!hp) {
	    perror ("gethostbyname");
	    exit (2);
	}

	/* create a socket to the INDI server */
	(void) memset ((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr =
			    ((struct in_addr *)(hp->h_addr_list[0]))->s_addr;
	serv_addr.sin_port = htons(port);
	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
	    perror ("socket");
	    exit(2);
	}

	/* connect */
	if (connect (sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
	    perror ("connect");
	    exit(2);
	}

	/* prepare for line-oriented i/o to client */
	*rfpp = fdopen (sockfd, "r");
	*wfpp = fdopen (sockfd, "w");
}

/* listen for property reports, send new sets if match */
static void
listenINDI (FILE *rfp, FILE *wfp)
{
	char msg[1024];

	/* arrange to call onAlarm() if not seeing any more defXXX */
	signal (SIGALRM, onAlarm);
	alarm (timeout);

	/* read from server, exit if find all properties */
	while (1) {
	    XMLEle *root = readXMLEle (lillp, readServerChar(rfp), msg);
	    if (root) {
		/* found a complete XML element */
		if (verbose > 1)
		    prXMLEle (stderr, root, 0);
		findSet (root, wfp);
		if (finished() == 0) {
		    shutdown (fileno(wfp), SHUT_WR);	/* insure flush */
		    exit (0);		/* found all we want */
		}
		delXMLEle (root);	/* not yet, delete and continue */
	    } else if (msg[0]) {
		fprintf (stderr, "Bad XML from %s/%d: %s\n", host, port, msg);
		exit(2);
	    }
	}
}

/* return 0 if we are sure we set everything we wanted to, else -1 */
static int
finished ()
{
	int i, j;

	for (i = 0; i < nsets; i++)
	    for (j = 0; j < sets[i].nev; j++)
		if (!sets[i].ev[j].ok)
		    return (-1);
	return (0);
}

/* called after timeout seconds because we did not find something we trying
 * to set.
 */
static void
onAlarm (int dummy)
{
	int i, j;

	for (i = 0; i < nsets; i++)
	    for (j = 0; j < sets[i].nev; j++)
		if (!sets[i].ev[j].ok)
		    fprintf (stderr, "No %s.%s.%s from %s:%d\n", sets[i].d,
				    sets[i].p, sets[i].ev[j].e, host, port);

	exit (1);
}

static int
readServerChar (FILE *fp)
{
	int c = fgetc (fp);

	if (c == EOF) {
	    if (ferror(fp))
		perror ("read");
	    else
		fprintf (stderr,"INDI server %s:%d disconnected\n", host, port);
	    exit (2);
	}

	if (verbose > 2)
	    fprintf (stderr, "Read %c\n", c);

	return (c);
}

/* issue a set command if it matches the given property */
static void
findSet (XMLEle *root, FILE *fp)
{
	char *rtype, *rdev, *rprop;
	XMLEle *ep;
	int t, s, i;

	/* check type */
	rtype = tagXMLEle (root);
	for (t = 0; t < NDEFS; t++)
	    if (strcmp (rtype, defs[t].defType) == 0)
		break;
	if (t == NDEFS)
	    return;
	alarm (timeout);	/* reset timeout */

	/* check each set for matching device and property name, send if ok */
	rdev  = findXMLAttValu (root, "device");
	rprop = findXMLAttValu (root, "name");
	if (verbose > 1)
	    fprintf (stderr, "Read definition for %s.%s\n", rdev, rprop);
	for (s = 0; s < nsets; s++) {
	    if (!strcmp (rdev, sets[s].d) && !strcmp (rprop, sets[s].p)) {
		/* found device and name,  check perm */
		if (!strchr (findXMLAttValu (root, "perm"), 'w')) {
		    if (verbose)
			fprintf (stderr, "%s.%s is read-only\n", rdev, rprop);
		    exit (1);
		}
		/* check matching elements */
		for (i = 0; i < sets[s].nev; i++) {
		    for (ep = nextXMLEle(root,1); ep; ep = nextXMLEle(root,0)) {
			if (!strcmp(findXMLAttValu (ep,"name"),sets[s].ev[i].e)
				    && !strcmp(tagXMLEle(ep), defs[t].defOne)) {
			    sets[s].ev[i].ok = 1;
			    break;
			}
		    }
		    if (!ep)
			return;		/* not in this msg, maybe later */
		}
		/* all element names found, send new values */
		sendNew (fp, &defs[t], &sets[s]);
	    }
	}
}

/* send the given set specification of the given INDI type to channel on fp */
static void
sendNew (FILE *fp, INDIDef *dp, SetSpec *sp)
{
	int i;

	fprintf (fp, "<%s device='%s' name='%s'>\n", dp->newType, sp->d, sp->p);
	for (i = 0; i < sp->nev; i++) {
	    if (verbose)
		fprintf (stderr, "  %s.%s.%s <- %s\n", sp->d, sp->p,
						    sp->ev[i].e, sp->ev[i].v);
	    fprintf (fp, "  <%s name='%s'>%s</%s>\n", dp->oneType, sp->ev[i].e,
						    sp->ev[i].v, dp->oneType);
	}
	fprintf (fp, "</%s>\n", dp->newType);
	fflush (fp);
	if (feof(fp) || ferror(fp)) {
	    fprintf (stderr, "Send error\n");
	    exit(2);
	}
}

/* scan e1[;e2...] v1[;v2,...] from e[] and v[] and add to spec sp.
 * exit if trouble.
 */
static void
scanEV (SetSpec *sp, char e[], char v[])
{
	static char sep[] = ";";
	char *ep, *vp;

	if (verbose > 1)
	    fprintf (stderr, "Scanning %s = %s\n", e, v);

	while (1) {
	    char *e0 = strtok_r (e, sep, &ep);
	    char *v0 = strtok_r (v, sep, &vp);

	    if (!e0 && !v0)
		break;
	    if (!e0) {
		fprintf (stderr, "More values than elements for %s.%s\n",
								sp->d, sp->p);
		exit(2);
	    }
	    if (!v0) {
		fprintf (stderr, "More elements than values for %s.%s\n",
								sp->d, sp->p);
		exit(2);
	    }

	    sp->ev = (SetEV *) realloc (sp->ev, (sp->nev+1)*sizeof(SetEV));
	    sp->ev[sp->nev].e = strcpy (malloc(strlen(e0)+1), e0);
	    sp->ev[sp->nev].v = strcpy (malloc(strlen(v0)+1), v0);
	    sp->nev++;

	    e = NULL;
	    v = NULL;
	}
}

/* For RCS Only -- Do Not Edit */
static char *rcsid[2] = {(char *)rcsid, "@(#) $RCSfile: setINDI.c,v $ $Date: 2006/09/12 19:55:51 $ $Revision: 1.2 $ $Name:  $"};
