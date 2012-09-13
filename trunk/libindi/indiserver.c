/* INDI Server for protocol version 1.7.
 * Copyright (C) 2007 Elwood C. Downey ecdowney@clearskyinstitute.com

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

 * argv lists names of Driver programs to run or sockets to connect for Devices.
 * Drivers are restarted if they exit or connection closes.
 * Each local Driver's stdin/out are assumed to provide INDI traffic and are
 *   connected here via pipes. Local Drivers' stderr are connected to our
 *   stderr with date stamp and driver name prepended.
 * We only support Drivers that advertise support for one Device. The problem
 *   with multiple Devices in one Driver is without a way to know what they
 *   _all_ are there is no way to avoid sending all messages to all Drivers.
 * Outbound messages are limited to Devices and Properties seen inbound.
 *   Messages to Devices on sockets always include Device so the chained
 *   indiserver will only pass back info from that Device.
 * All newXXX() received from one Client are echoed to all other Clients who
 *   have shown an interest in the same Device and property.
 *
 * Implementation notes:
 *
 * We fork each driver and open a server socket listening for INDI clients.
 * Then forever we listen for new clients and pass traffic between clients and
 * drivers, subject to optimizations based on sniffing messages for matching
 * Devices and Properties. Since one message might be destined to more than
 * one client or device, they are queued and only removed after the last
 * consumer is finished. XMLEle are converted to linear strings before being
 * sent to optimize write system calls and avoid blocking to slow clients.
 * Clients that get more than maxqsiz bytes behind are shut down.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "lilxml.h"
#include "indiapi.h"
#include "fq.h"

#define INDIPORT        7624            /* default TCP/IP port to listen */
#define	REMOTEDVR	(-1234)		/* invalid PID to flag remote drivers */
#define	MAXRBUF		4096		/* max read buffering here */
#define	MAXWSIZ		4096		/* max bytes/write */
#define	DEFMAXQSIZ	10		/* default max q behind, MB */

/* associate a usage count with queuded client or device message */
typedef struct {
    int count;				/* number of consumers left */
    unsigned int cl;			/* content length */
    char *cp;				/* content: buf or malloced */
    char buf[MAXWSIZ];		/* local buf for most messages */
} Msg;

/* BLOB handling, NEVER is the default */
typedef enum {B_NEVER=0, B_ALSO, B_ONLY} BLOBHandling;

/* device + property name */
typedef struct {
    char dev[MAXINDIDEVICE];
    char name[MAXINDINAME];
    BLOBHandling blob;			/* when to snoop BLOBs */
} Property;


/* record of each snooped property
typedef struct {
    Property prop;
    BLOBHandling blob;
} Property;
*/

struct
{
    const char *name;                      /* Path to FIFO for dynamic startups & shutdowns of drivers */
    int fd;
    FILE *fs;
} fifo;

/* info for each connected client */
typedef struct {
    int active;				/* 1 when this record is in use */
    Property *props;			/* malloced array of props we want */
    int nprops;				/* n entries in props[] */
    int allprops;			/* saw getProperties w/o device */
    BLOBHandling blob;			/* when to send setBLOBs */
    int s;				/* socket for this client */
    LilXML *lp;				/* XML parsing context */
    FQ *msgq;				/* Msg queue */
    unsigned int nsent;				/* bytes of current Msg sent so far */
} ClInfo;
static ClInfo *clinfo;			/*  malloced pool of clients */
static int nclinfo;			/* n total (not active) */

/* info for each connected driver */
typedef struct {
    char name[MAXINDIDEVICE];		/* persistent name */
    char dev[MAXINDIDEVICE];		/* device served by this driver */
    int active;				/* 1 when this record is in use */
    Property *sprops;			/* malloced array of props we snoop */
    int nsprops;			/* n entries in sprops[] */
    int pid;				/* process id or REMOTEDVR if remote */
    int rfd;				/* read pipe fd */
    int wfd;				/* write pipe fd */
    int efd;				/* stderr from driver, if local */
    int restarts;			/* times process has been restarted */
    LilXML *lp;				/* XML parsing context */
    FQ *msgq;				/* Msg queue */
    unsigned int nsent;			/* bytes of current Msg sent so far */
} DvrInfo;
static DvrInfo *dvrinfo;		/* malloced array of drivers */
static int ndvrinfo;			/* n total */

static char *me;			/* our name */
static int port = INDIPORT;		/* public INDI port */
static int verbose;			/* chattiness */
static int lsocket;			/* listen socket */
static char *ldir;			/* where to log driver messages */
static int maxqsiz = (DEFMAXQSIZ*1024*1024); /* kill if these bytes behind */

static void logStartup(int ac, char *av[]);
static void usage (void);
static void noZombies (void);
static void noSIGPIPE (void);
static void indiFIFO(void);
static void indiRun (void);
static void indiListen (void);
static void newFIFO(void);
static void newClient (void);
static int newClSocket (void);
static void shutdownClient (ClInfo *cp);
static int readFromClient (ClInfo *cp);
static void startDvr (DvrInfo *dp);
static void startLocalDvr (DvrInfo *dp);
static void startRemoteDvr (DvrInfo *dp);
static int openINDIServer (char host[], int indi_port);
static void shutdownDvr (DvrInfo *dp, int restart);
static void q2RDrivers (const char *dev, Msg *mp, XMLEle *root);
static void q2SDrivers (int isblob, const char *dev, const char *name, Msg *mp,
    XMLEle *root);
static int q2Clients (ClInfo *notme, int isblob, const char *dev, const char *name,
    Msg *mp, XMLEle *root);
static void addSDevice (DvrInfo *dp, const char *dev, const char *name);
static Property *findSDevice (DvrInfo *dp, const char *dev, const char *name);
static void addClDevice (ClInfo *cp, const char *dev, const char *name, int isblob);
static int findClDevice (ClInfo *cp, const char *dev, const char *name);
static int readFromDriver (DvrInfo *dp);
static int stderrFromDriver (DvrInfo *dp);
static int msgQSize (FQ *q);
static void setMsgXMLEle (Msg *mp, XMLEle *root);
static void setMsgStr (Msg *mp, char *str);
static void freeMsg (Msg *mp);
static Msg *newMsg (void);
static int sendClientMsg (ClInfo *cp);
static int sendDriverMsg (DvrInfo *cp);
static void crackBLOB (const char *enableBLOB, BLOBHandling *bp);
static void crackBLOBHandling(const char *dev, const char *name, const char *enableBLOB, ClInfo *cp);
static void traceMsg (XMLEle *root);
static char *indi_tstamp (char *s);
static void logDMsg (XMLEle *root, const char *dev);
static void Bye(void);

int
main (int ac, char *av[])
{
	/* log startup */
	logStartup(ac, av);

	/* save our name */
	me = av[0];

	/* crack args */
	while ((--ac > 0) && ((*++av)[0] == '-')) {
	    char *s;
	    for (s = av[0]+1; *s != '\0'; s++)
		switch (*s) {
		case 'l':
		    if (ac < 2) {
			fprintf (stderr, "-l requires log directory\n");
			usage();
		    }
		    ldir = *++av;
		    ac--;
		    break;
		case 'm':
		    if (ac < 2) {
			fprintf (stderr, "-m requires max MB behind\n");
			usage();
		    }
		    maxqsiz = 1024*1024*atoi(*++av);
		    ac--;
		    break;
		case 'p':
		    if (ac < 2) {
			fprintf (stderr, "-p requires port value\n");
			usage();
		    }
		    port = atoi(*++av);
		    ac--;
		    break;
                case 'f':
                    if (ac < 2) {
                        fprintf (stderr, "-f requires fifo node\n");
                        usage();
                    }
                    fifo.name = *++av;
                    ac--;
                    break;
		case 'v':
		    verbose++;
		    break;
		default:
		    usage();
		}
	}

	/* at this point there are ac args in av[] to name our drivers */
        if (ac == 0 && !fifo.name)
            usage();

	/* take care of some unixisms */
	noZombies();
	noSIGPIPE();

	/* realloc seed for client pool */
	clinfo = (ClInfo *) malloc (1);
	nclinfo = 0;

	/* create driver info array all at once since size never changes */
        ndvrinfo = ac;
	dvrinfo = (DvrInfo *) calloc (ndvrinfo, sizeof(DvrInfo));

	/* start each driver */
        while (ac-- > 0) {
            strcpy(dvrinfo[ac].name, *av++);
	    startDvr (&dvrinfo[ac]);
        }

	/* announce we are online */
	indiListen();

        /* Load up FIFO, if available */
        indiFIFO();

	/* handle new clients and all io */
	while (1)
	    indiRun();

	/* whoa! */
	fprintf (stderr, "unexpected return from main\n");
	return (1);
}

/* record we have started and our args */
static void
logStartup(int ac, char *av[])
{
	int i;

	fprintf (stderr, "%s: startup: ", indi_tstamp(NULL));
	for (i = 0; i < ac; i++)
	    fprintf (stderr, "%s ", av[i]);
	fprintf (stderr, "\n");
}

/* print usage message and exit (2) */
static void
usage(void)
{
	fprintf (stderr, "Usage: %s [options] driver [driver ...]\n", me);
	fprintf (stderr, "Purpose: server for local and remote INDI drivers\n");
	fprintf (stderr, "Code %s. Protocol %g.\n", "$Revision: 726523 $", INDIV);
	fprintf (stderr, "Options:\n");
        fprintf (stderr, " -l d     : log driver messages to <d>/YYYY-MM-DD.islog\n");
        fprintf (stderr, " -m m     : kill client if gets more than this many MB behind, default %d\n", DEFMAXQSIZ);
        fprintf (stderr, " -p p     : alternate IP port, default %d\n", INDIPORT);
        fprintf (stderr, " -f path  : Path to fifo for dynamic startup and shutdown of drivers.\n");
        fprintf (stderr, " -v       : show key events, no traffic\n");
        fprintf (stderr, " -vv      : -v + key message content\n");
        fprintf (stderr, " -vvv     : -vv + complete xml\n");
        fprintf (stderr, "driver    : executable or device@host[:port]\n");

	exit (2);
}

/* arrange for no zombies if drivers die */
static void
noZombies()
{
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
#ifdef SA_NOCLDWAIT
	sa.sa_flags = SA_NOCLDWAIT;
#else
	sa.sa_flags = 0;
#endif
	(void)sigaction(SIGCHLD, &sa, NULL);
}

/* turn off SIGPIPE on bad write so we can handle it inline */
static void
noSIGPIPE()
{
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	(void)sigaction(SIGPIPE, &sa, NULL);
}

static DvrInfo * allocDvr ()
{
    DvrInfo *dp = NULL;
    int dvi;

/* try to reuse a drivber slot, else add one */
for (dvi = 0; dvi < ndvrinfo; dvi++)
    if (!(dp = &dvrinfo[dvi])->active)
        break;
if (dvi == ndvrinfo)
{
    /* grow dvrinfo */
    dvrinfo = (DvrInfo *) realloc (dvrinfo, (ndvrinfo+1)*sizeof(DvrInfo));
    if (!dvrinfo) {
        fprintf (stderr, "no memory for new drivers\n");
        Bye();
    }
    dp = &dvrinfo[ndvrinfo++];
}

/* rig up new dvrinfo entry */
memset (dp, 0, sizeof(*dp));
dp->active = 1;

return dp;

}

/* start the given INDI driver process or connection.
 * exit if trouble.
 */
static void
startDvr (DvrInfo *dp)
{
	if (strchr (dp->name, '@'))
	    startRemoteDvr (dp);
	else
	    startLocalDvr (dp);
}

/* start the given local INDI driver process.
 * exit if trouble.
 */
static void
startLocalDvr (DvrInfo *dp)
{
	Msg *mp;
	char buf[1024];
	int rp[2], wp[2], ep[2];
	int pid;

	/* build three pipes: r, w and error*/
	if (pipe (rp) < 0) {
	    fprintf (stderr, "%s: read pipe: %s\n", indi_tstamp(NULL),
							    strerror(errno));
	    Bye();
	}
	if (pipe (wp) < 0) {
	    fprintf (stderr, "%s: write pipe: %s\n", indi_tstamp(NULL),
							    strerror(errno));
	    Bye();
	}
	if (pipe (ep) < 0) {
	    fprintf (stderr, "%s: stderr pipe: %s\n", indi_tstamp(NULL),
							    strerror(errno));
	    Bye();
	}

	/* fork&exec new process */
	pid = fork();
	if (pid < 0) {
	    fprintf (stderr, "%s: fork: %s\n", indi_tstamp(NULL), strerror(errno));
	    Bye();
	}
	if (pid == 0) {
	    /* child: exec name */
	    int fd;

	    /* rig up pipes */
	    dup2 (wp[0], 0);	/* driver stdin reads from wp[0] */
	    dup2 (rp[1], 1);	/* driver stdout writes to rp[1] */
	    dup2 (ep[1], 2);	/* driver stderr writes to e[]1] */
	    for (fd = 3; fd < 100; fd++)
		(void) close (fd);

	    /* go -- should never return */
	    execlp (dp->name, dp->name, NULL);
	    fprintf (stderr, "%s: Driver %s: execlp: %s\n", indi_tstamp(NULL),
						dp->name, strerror(errno));
	    _exit (1);	/* parent will notice EOF shortly */
	}

	/* don't need child's side of pipes */
	close (wp[0]);
	close (rp[1]);
	close (ep[1]);

	/* record pid, io channels, init lp and snoop list */
	dp->pid = pid;
	dp->rfd = rp[0];
	dp->wfd = wp[1];
	dp->efd = ep[0];
	dp->lp = newLilXML();
        dp->msgq = newFQ(1);
        dp->sprops = (Property*) malloc (1);	/* seed for realloc */
	dp->nsprops = 0;
	dp->nsent = 0;
        dp->active = 1;

	/* first message primes driver to report its properties -- dev known
	 * if restarting
	 */
	mp = newMsg();
	pushFQ (dp->msgq, mp);
	if (dp->dev[0])
	    sprintf (buf, "<getProperties device='%s' version='%g'/>\n",
							    dp->dev, INDIV);
	else
	    sprintf (buf, "<getProperties version='%g'/>\n", INDIV);
	setMsgStr (mp, buf);
	mp->count++;

	if (verbose > 0)
	    fprintf (stderr, "%s: Driver %s: pid=%d rfd=%d wfd=%d efd=%d\n",
		    indi_tstamp(NULL), dp->name, dp->pid, dp->rfd, dp->wfd, dp->efd);
}

/* start the given remote INDI driver connection.
 * exit if trouble.
 */
static void
startRemoteDvr (DvrInfo *dp)
{
	Msg *mp;
	char dev[1024];
	char host[1024];
	char buf[1024];
	int indi_port, sockfd;

	/* extract host and port */
	indi_port = INDIPORT;
	if (sscanf (dp->name, "%[^@]@%[^:]:%d", dev, host, &indi_port) < 2) {
	    fprintf (stderr, "Bad remote device syntax: %s\n", dp->name);
	    Bye();
	}

	/* connect */
	sockfd = openINDIServer (host, indi_port);

	/* record flag pid, io channels, init lp and snoop list */
	dp->pid = REMOTEDVR;
	dp->rfd = sockfd;
	dp->wfd = sockfd;
	dp->lp = newLilXML();
	dp->msgq = newFQ(1);
        dp->sprops = (Property*) malloc (1);	/* seed for realloc */
	dp->nsprops = 0;
	dp->nsent = 0;
        dp->active = 1;

	/* N.B. storing name now is key to limiting outbound traffic to this
	 * dev.
	 */
	strncpy (dp->dev, dev, MAXINDIDEVICE-1);
	dp->dev[MAXINDIDEVICE-1] = '\0';

	/* Sending getProperties with device lets remote server limit its
	 * outbound (and our inbound) traffic on this socket to this device.
	 */
	mp = newMsg();
	pushFQ (dp->msgq, mp);
	sprintf (buf, "<getProperties device='%s' version='%g'/>\n",
							    dp->dev, INDIV);
	setMsgStr (mp, buf);
	mp->count++;

	if (verbose > 0)
	    fprintf (stderr, "%s: Driver %s: socket=%d\n", indi_tstamp(NULL),
							    dp->name, sockfd);
}

/* open a connection to the given host and port or die.
 * return socket fd.
 */
static int
openINDIServer (char host[], int indi_port)
{
	struct sockaddr_in serv_addr;
	struct hostent *hp;
	int sockfd;

	/* lookup host address */
	hp = gethostbyname (host);
	if (!hp) {
	    fprintf (stderr, "gethostbyname(%s): %s\n", host, strerror(errno));
	    Bye();
	}

	/* create a socket to the INDI server */
	(void) memset ((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr =
			    ((struct in_addr *)(hp->h_addr_list[0]))->s_addr;
	serv_addr.sin_port = htons(indi_port);
	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
	    fprintf (stderr, "socket(%s,%d): %s\n", host, indi_port,strerror(errno));
	    Bye();
	}

	/* connect */
	if (connect (sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
	    fprintf (stderr, "connect(%s,%d): %s\n", host,indi_port,strerror(errno));
	    Bye();
	}

	/* ok */
	return (sockfd);
}

/* create the public INDI Driver endpoint lsocket on port.
 * return server socket else exit.
 */
static void
indiListen ()
{
	struct sockaddr_in serv_socket;
	int sfd;
	int reuse = 1;

	/* make socket endpoint */
	if ((sfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
	    fprintf (stderr, "%s: socket: %s\n", indi_tstamp(NULL), strerror(errno));
	    Bye();
	}

	/* bind to given port for any IP address */
	memset (&serv_socket, 0, sizeof(serv_socket));
	serv_socket.sin_family = AF_INET;
	#ifdef SSH_TUNNEL
	serv_socket.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
	#else
	serv_socket.sin_addr.s_addr = htonl (INADDR_ANY);
	#endif
	serv_socket.sin_port = htons ((unsigned short)port);
	if (setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse)) < 0){
	    fprintf (stderr, "%s: setsockopt: %s\n", indi_tstamp(NULL),
							    strerror(errno));
	    Bye();
	}
	if (bind(sfd,(struct sockaddr*)&serv_socket,sizeof(serv_socket)) < 0){
	    fprintf (stderr, "%s: bind: %s\n", indi_tstamp(NULL), strerror(errno));
	    Bye();
	}

	/* willing to accept connections with a backlog of 5 pending */
	if (listen (sfd, 5) < 0) {
	    fprintf (stderr, "%s: listen: %s\n", indi_tstamp(NULL), strerror(errno));
	    Bye();
	}

	/* ok */
	lsocket = sfd;
	if (verbose > 0)
	    fprintf (stderr, "%s: listening to port %d on fd %d\n",
	    					indi_tstamp(NULL), port, sfd);
}

/* Attempt to open up FIFO */
static void indiFIFO(void)
{
    /* Open up FIFO, if available */
    if (fifo.name)
    {
        fifo.fd = open(fifo.name, O_RDONLY);
                  if (fifo.fd < 0)
                  {
                       fprintf(stderr, "%s: open(%s): %s.\n", indi_tstamp(NULL), fifo.name, strerror(errno));
                        Bye();
                   }

        fifo.fs = fdopen(fifo.fd, "r");
    }
}

/* service traffic from clients and drivers */
static void
indiRun(void)
{
	fd_set rs, ws;
        int maxfd=0;
	int i, s;

	/* init with no writers or readers */
	FD_ZERO(&ws);
	FD_ZERO(&rs);

        if (fifo.name && fifo.fd > 0)
        {
           FD_SET(fifo.fd, &rs);
           maxfd = fifo.fd;
        }

	/* always listen for new clients */
	FD_SET(lsocket, &rs);
        if (lsocket > maxfd)
                maxfd = lsocket;

	/* add all client readers and client writers with work to send */
	for (i = 0; i < nclinfo; i++) {
	    ClInfo *cp = &clinfo[i];
	    if (cp->active) {
		FD_SET(cp->s, &rs);
		if (nFQ(cp->msgq) > 0)
		    FD_SET(cp->s, &ws);
		if (cp->s > maxfd)
		    maxfd = cp->s;
	    }
	}

	/* add all driver readers and driver writers with work to send */
        for (i = 0; i < ndvrinfo; i++)
        {
	    DvrInfo *dp = &dvrinfo[i];
            if (dp->active)
            {
                FD_SET(dp->rfd, &rs);
                if (dp->rfd > maxfd)
                   maxfd = dp->rfd;
                if (dp->pid != REMOTEDVR)
                {
                   FD_SET(dp->efd, &rs);
                   if (dp->efd > maxfd)
                      maxfd = dp->efd;
                }
                if (nFQ(dp->msgq) > 0)
                {
                   FD_SET(dp->wfd, &ws);
                   if (dp->wfd > maxfd)
                       maxfd = dp->wfd;
                }
            }
	}

	/* wait for action */
	s = select (maxfd+1, &rs, &ws, NULL, NULL);
	if (s < 0) {
	    fprintf (stderr, "%s: select(%d): %s\n", indi_tstamp(NULL), maxfd+1,
							    strerror(errno));
	    Bye();
	}


        /* new command from FIFO? */
        if (s > 0 && FD_ISSET(fifo.fd, &rs)) {
            newFIFO();
            s--;
        }

	/* new client? */
	if (s > 0 && FD_ISSET(lsocket, &rs)) {
	    newClient();
	    s--;
	}

	/* message to/from client? */
	for (i = 0; s > 0 && i < nclinfo; i++) {
	    ClInfo *cp = &clinfo[i];
	    if (cp->active) {
		if (FD_ISSET(cp->s, &rs)) {
		    if (readFromClient(cp) < 0)
			return;	/* fds effected */
		    s--;
		}
		if (s > 0 && FD_ISSET(cp->s, &ws)) {
		    if (sendClientMsg(cp) < 0)
			return;	/* fds effected */
		    s--;
		}
	    }
	}

	/* message to/from driver? */
	for (i = 0; s > 0 && i < ndvrinfo; i++) {
	    DvrInfo *dp = &dvrinfo[i];
	    if (dp->pid != REMOTEDVR && FD_ISSET(dp->efd, &rs)) {
		if (stderrFromDriver(dp) < 0)
		    return;	/* fds effected */
		s--;
	    }
	    if (s > 0 && FD_ISSET(dp->rfd, &rs)) {
		if (readFromDriver(dp) < 0)
		    return;	/* fds effected */
		s--;
	    }
	    if (s > 0 && FD_ISSET(dp->wfd, &ws) && nFQ(dp->msgq) > 0) {
		if (sendDriverMsg(dp) < 0)
		    return;	/* fds effected */
		s--;
	    }
	}
}

/* Read commands from FIFO and process them. Start/stop drivers accordingly */
static void newFIFO(void)
{
    char line[MAXRBUF], tDriver[MAXRBUF], tConfig[MAXRBUF], tDev[MAXRBUF];
    static char *envDev=NULL, *envConfig=NULL;
    const char *delm = " ";
    char *token, *cp, *tp;
    DvrInfo *dp = NULL;
    int startCmd=0;

    if (envDev == NULL)
        envDev = (char *) malloc(MAXRBUF* sizeof(char));
    if (envConfig == NULL)
        envConfig = (char *) malloc(MAXRBUF * sizeof(char));

    while ( fgets (line, MAXRBUF, fifo.fs) != NULL)
    {
        if (verbose)
            fprintf(stderr, "FIFO: %s\n", line);

        tDev[0] = '\0', tDriver[0] = '\0', tConfig[0] = '\0', envDev[0] = '\0', envConfig[0] = '\0';
        cp = strdup(line);

        token = strsep(&cp, delm);

        if (!strcmp(token, "start") || !strcmp(token, "stop"))
        {
            //fprintf(stderr, "TOKEN: %s\n", token);

                if (!strcmp(token, "start"))
                    startCmd = 1;

                token = strsep(&cp, delm);

                if (!token)
                    continue;

               strncpy(tDriver, token, MAXRBUF);
               if (tp = strchr(tDriver, '\n'))
                   *tp = '\0';

               if (verbose)
                fprintf(stderr, "FIFO: Request for %s driver: %s\n", (startCmd == 1) ? "starting" : "stopping", tDriver);

               /* Find more name + config */
               token = strsep(&cp, delm);

               if (token)
               {
                 /* If config file detected, copy it */
                if (strstr(token, ".xml"))                 
                    strncpy(tConfig, token, MAXRBUF);
                /* Get rid of quotes */
                else if (strstr(token, "\"") || strstr(token, "'"))
                {
                    strncat(tDev, ++token, sizeof(tDev)-strlen(tDev)-1);

                    if ( tDev[strlen(tDev)-2] == '\"' || tDev[strlen(tDev)-2] == '\'')
                        tDev[strlen(tDev)-2] = '\0';
                   else while (token = strsep(&cp, delm) )
                   {
                     strcat(tDev, " ");
                     strncat(tDev, token, sizeof(tDev)-strlen(tDev)-1);
                     if ( (tp=strchr(tDev, '\"')) || (tp=strchr(tDev, '\'')))
                     {
                         tDev[strlen(tDev)-1] = '\0';
                         *tp='\0';
                         break;
                     }
                    }
                 }
                 else
                {
                     strncpy(tDev, token, MAXRBUF);
                     if (tp = strchr(tDev, '\n'))
                         *tp = '\0';
                 }

                  /* Find config, if there is any */
                  token = strsep(&cp, delm);
                  if (token)
                  {
                      /* Get rid of quotes */
                      if (strstr(token, "\"") || strstr(token, "'"))
                      {
                       strncat(tConfig, ++token, sizeof(tConfig)-strlen(tDev)-1);

                       if ( tConfig[strlen(tConfig)-2] == '\"' || tConfig[strlen(tConfig)-2] == '\'')
                           tConfig[strlen(tConfig)-2] = '\0';

                       while (token = strsep(&cp, delm) )
                       {
                          strcat(tConfig, " ");
                          strncat(tConfig, token, sizeof(tConfig)-strlen(tDev)-1);

                          if ( (tp=strchr(tConfig, '\"')) || (tp=strchr(tConfig, '\'')))
                          {
                               tConfig[strlen(tConfig)-1] = '\0';
                              *tp = '\0';
                              break;
                          }
                       }
                      }
                       else
                      {
                           strncpy(tConfig, token, MAXRBUF);
                           if (tp = strchr(tConfig, '\n'))
                               *tp = '\0';
                       }
                  }
              }

               /* Did we find device name? */
               if (tDev[0])
               {
                 snprintf(envDev, MAXRBUF, "INDIDEV=%s", tDev);

                 if (verbose)
                    fprintf(stderr, "With name: %s\n", envDev);

                 putenv(envDev);

                 //fprintf(stderr, "envionment check INDIDEV: %s\n", getenv("INDIDEV"));
               }

               /* Did we find config file */
               if (tConfig[0])
               {
                   snprintf(envConfig, MAXRBUF, "INDICONFIG=%s", tConfig);

                   if (verbose)
                    fprintf(stderr, "With config: %s\n", envConfig);

                   putenv(envConfig);
               }

               if (startCmd)
               {
                   if (verbose)
                        fprintf(stderr, "FIFO: Starting driver %s\n", tDriver);
                    dp = allocDvr();
                    strncpy(dp->name, tDriver, MAXINDIDEVICE);
                    strncpy(dp->dev, tDev, MAXINDIDEVICE);
                    startDvr (dp);
              }
               else
               {
                  for (dp = dvrinfo; dp < &dvrinfo[ndvrinfo]; dp++)
                   {
                       if (!strcmp(dp->name, tDriver) && dp->active==1)
                       {
                           /* If device name is given, check against it before shutting down */
                           if (tDev[0] && strcmp(dp->dev, tDev))
                               continue;

                           if (verbose)
                               fprintf(stderr, "FIFO: Shutting down driver: %s\n", tDriver);

                           shutdownDvr(dp, 0);

                               /* Inform clients that this driver is dead */
                               XMLEle *root = addXMLEle (NULL, "delProperty");
                               addXMLAtt(root, "device", dp->dev);
                               Msg * mp = newMsg();

                               q2Clients(NULL, 0, dp->dev, NULL, mp, root);
                               if (mp->count > 0)
                                   setMsgXMLEle (mp, root);
                               else
                                   freeMsg (mp);
                               delXMLEle (root);

                           break;

                       }
                   }
               }
         }
    }
}

/* prepare for new client arriving on lsocket.
 * exit if trouble.
 */
static void
newClient()
{
	ClInfo *cp = NULL;
	int s, cli;

	/* assign new socket */
	s = newClSocket ();

	/* try to reuse a clinfo slot, else add one */
	for (cli = 0; cli < nclinfo; cli++)
	    if (!(cp = &clinfo[cli])->active)
		break;
	if (cli == nclinfo) {
	    /* grow clinfo */
	    clinfo = (ClInfo *) realloc (clinfo, (nclinfo+1)*sizeof(ClInfo));
	    if (!clinfo) {
		fprintf (stderr, "no memory for new client\n");
		Bye();
	    }
	    cp = &clinfo[nclinfo++];
	}

	/* rig up new clinfo entry */
	memset (cp, 0, sizeof(*cp));
	cp->active = 1;
	cp->s = s;
	cp->lp = newLilXML();
	cp->msgq = newFQ(1);
	cp->props = malloc (1);
	cp->nsent = 0;

	if (verbose > 0) {
	    struct sockaddr_in addr;
	    socklen_t len = sizeof(addr);
	    getpeername(s, (struct sockaddr*)&addr, &len);
	    fprintf(stderr,"%s: Client %d: new arrival from %s:%d - welcome!\n",
			    indi_tstamp(NULL), cp->s, inet_ntoa(addr.sin_addr),
							ntohs(addr.sin_port));
	}
}

/* read more from the given client, send to each appropriate driver when see
 * xml closure. also send all newXXX() to all other interested clients.
 * return -1 if had to shut down anything, else 0.
 */
static int
readFromClient (ClInfo *cp)
{
	char buf[MAXRBUF];
	int shutany = 0;
	int i, nr;

	/* read client */
	nr = read (cp->s, buf, sizeof(buf));
	if (nr <= 0) {
	    if (nr < 0)
		fprintf (stderr, "%s: Client %d: read: %s\n", indi_tstamp(NULL),
							cp->s, strerror(errno));
	    else if (verbose > 0)
		fprintf (stderr, "%s: Client %d: read EOF\n", indi_tstamp(NULL),
									cp->s);
	    shutdownClient (cp);
	    return (-1);
	}

	/* process XML, sending when find closure */
	for (i = 0; i < nr; i++) {
	    char err[1024];
	    XMLEle *root = readXMLEle (cp->lp, buf[i], err);
	    if (root) {
		char *roottag = tagXMLEle(root);
		const char *dev = findXMLAttValu (root, "device");
		const char *name = findXMLAttValu (root, "name");
		int isblob = !strcmp (tagXMLEle(root), "setBLOBVector");
		Msg *mp;

		if (verbose > 2) {
		    fprintf (stderr, "%s: Client %d: read ",indi_tstamp(NULL),cp->s);
		    traceMsg (root);
		} else if (verbose > 1) {
		    fprintf (stderr, "%s: Client %d: read <%s device='%s' name='%s'>\n",
				    indi_tstamp(NULL), cp->s, tagXMLEle(root),
				    findXMLAttValu (root, "device"),
				    findXMLAttValu (root, "name"));
		}

		/* snag interested properties.
		 * N.B. don't open to alldevs if seen specific dev already, else
		 *   remote client connections start returning too much.
		 */
		if (dev[0])
                    addClDevice (cp, dev, name, isblob);
		else if (!strcmp (roottag, "getProperties") && !cp->nprops)
		    cp->allprops = 1;

		/* snag enableBLOB -- send to remote drivers too */
		if (!strcmp (roottag, "enableBLOB"))
                   // crackBLOB (pcdataXMLEle(root), &cp->blob);
                     crackBLOBHandling (dev, name, pcdataXMLEle(root), cp);

		/* build a new message -- set content iff anyone cares */
		mp = newMsg();

		/* send message to driver(s) responsible for dev */
		q2RDrivers (dev, mp, root);

		/* echo new* commands back to other clients */
		if (!strncmp (roottag, "new", 3)) {
                    if (q2Clients (cp, isblob, dev, name, mp, root) < 0)
			shutany++;
		}

		/* set message content if anyone cares else forget it */
		if (mp->count > 0)
		    setMsgXMLEle (mp, root);
		else
		    freeMsg (mp);
		delXMLEle (root);

	    } else if (err[0]) {
		char *ts = indi_tstamp(NULL);
		fprintf (stderr, "%s: Client %d: XML error: %s\n", ts,
								cp->s, err);
		fprintf (stderr, "%s: Client %d: XML read: %.*s\n", ts,
							    cp->s, nr, buf);
		shutdownClient (cp);
		return (-1);
	    }
	}

	return (shutany ? -1 : 0);
}

/* read more from the given driver, send to each interested client when see
 * xml closure. if driver dies, try restarting.
 * return 0 if ok else -1 if had to shut down anything.
 */
static int
readFromDriver (DvrInfo *dp)
{
	char buf[MAXRBUF];
	int shutany = 0;
	int i, nr;

	/* read driver */
	nr = read (dp->rfd, buf, sizeof(buf));
	if (nr <= 0) {
	    if (nr < 0)
		fprintf (stderr, "%s: Driver %s: stdin %s\n", indi_tstamp(NULL),
						    dp->name, strerror(errno));
	    else
		fprintf (stderr, "%s: Driver %s: stdin EOF\n",
							indi_tstamp(NULL), dp->name);
            shutdownDvr (dp, 1);
	    return (-1);
	}

	/* process XML, sending when find closure */
	for (i = 0; i < nr; i++) {
	    char err[1024];
	    XMLEle *root = readXMLEle (dp->lp, buf[i], err);
	    if (root) {
		char *roottag = tagXMLEle(root);
		const char *dev = findXMLAttValu (root, "device");
		const char *name = findXMLAttValu (root, "name");
		int isblob = !strcmp (tagXMLEle(root), "setBLOBVector");
		Msg *mp;

		if (verbose > 2) {
		    fprintf(stderr, "%s: Driver %s: read ", indi_tstamp(0),dp->name);
		    traceMsg (root);
		} else if (verbose > 1) {
		    fprintf (stderr, "%s: Driver %s: read <%s device='%s' name='%s'>\n",
				    indi_tstamp(NULL), dp->name, tagXMLEle(root),
				    findXMLAttValu (root, "device"),
				    findXMLAttValu (root, "name"));
		}

		/* that's all if driver is just registering a snoop */
		if (!strcmp (roottag, "getProperties")) {
		    addSDevice (dp, dev, name);
		    delXMLEle (root);
		    continue;
		}

		/* that's all if driver is just registering a BLOB mode */
		if (!strcmp (roottag, "enableBLOB")) {
                    Property *sp = findSDevice (dp, dev, name);
		    if (sp)
			crackBLOB (pcdataXMLEle (root), &sp->blob);
		    delXMLEle (root);
		    continue;
		}

		/* snag device name if not known yet */
		if (!dp->dev[0] && dev[0]) {
		    strncpy (dp->dev, dev, MAXINDIDEVICE-1);
		    dp->dev[MAXINDIDEVICE-1] = '\0';
		}

		/* log messages if any and wanted */
		if (ldir)
		    logDMsg (root, dev);

		/* build a new message -- set content iff anyone cares */
		mp = newMsg();

		/* send to interested clients */
                if (q2Clients (NULL, isblob, dev, name, mp, root) < 0)
		    shutany++;

		/* send to snooping drivers */
		q2SDrivers (isblob, dev, name, mp, root);

		/* set message content if anyone cares else forget it */
		if (mp->count > 0)
		    setMsgXMLEle (mp, root);
		else
		    freeMsg (mp);
		delXMLEle (root);

	    } else if (err[0]) {
		char *ts = indi_tstamp(NULL);
		fprintf (stderr, "%s: Driver %s: XML error: %s\n", ts,
								dp->name, err);
		fprintf (stderr, "%s: Driver %s: XML read: %.*s\n", ts,
							    dp->name, nr, buf);
                shutdownDvr (dp, 1);
		return (-1);
	    }
	}

	return (shutany ? -1 : 0);
}

/* read more from the given driver stderr, add prefix and send to our stderr.
 * return 0 if ok else -1 if had to restart.
 */
static int
stderrFromDriver (DvrInfo *dp)
{
	static char exbuf[MAXRBUF];
	static int nexbuf;
	int i, nr;

	/* read more */
	nr = read (dp->efd, exbuf+nexbuf, sizeof(exbuf)-nexbuf);
	if (nr <= 0) {
	    if (nr < 0)
		fprintf (stderr, "%s: Driver %s: stderr %s\n", indi_tstamp(NULL),
						    dp->name, strerror(errno));
	    else
		fprintf (stderr, "%s: Driver %s: stderr EOF\n",
							indi_tstamp(NULL), dp->name);
            shutdownDvr (dp, 1);
	    return (-1);
	}
	nexbuf += nr;

	/* prefix each whole line to our stderr, save extra for next time */
	for (i = 0; i < nexbuf; i++) {
	    if (exbuf[i] == '\n') {
		fprintf (stderr, "%s: Driver %s: %.*s\n", indi_tstamp(NULL),
							    dp->name, i, exbuf);
		i++;				  /* count including nl */
		nexbuf -= i;			  /* remove from nexbuf */
		memmove (exbuf, exbuf+i, nexbuf); /* slide remaining to front */
		i = -1;				  /* restart for loop scan */
	    }
	}

	return (0);
}

/* close down the given client */
static void
shutdownClient (ClInfo *cp)
{
	Msg *mp;

	/* close connection */
	shutdown (cp->s, SHUT_RDWR);
	close (cp->s);

	/* free memory */
	delLilXML (cp->lp);
	free (cp->props);

	/* decrement and possibly free any unsent messages for this client */
	while ((mp = (Msg*) popFQ(cp->msgq)) != NULL)
	    if (--mp->count == 0)
		freeMsg (mp);
	delFQ (cp->msgq);

	/* ok now to recycle */
	cp->active = 0;

	if (verbose > 0)
	    fprintf (stderr, "%s: Client %d: shut down complete - bye!\n",
							indi_tstamp(NULL), cp->s);
}

/* close down the given driver and restart */
static void
shutdownDvr (DvrInfo *dp, int restart)
{
	Msg *mp;

	/* make sure it's dead, reclaim resources */
	if (dp->pid == REMOTEDVR) {
	    /* socket connection */
	    shutdown (dp->wfd, SHUT_RDWR);
	    close (dp->wfd);	/* same as rfd */
	} else {
	    /* local pipe connection */
            kill (dp->pid, SIGKILL);	/* we've insured there are no zombies */
	    close (dp->wfd);
	    close (dp->rfd);
	    close (dp->efd);
	}

	/* free memory */
	free (dp->sprops);
	delLilXML (dp->lp);

        /* ok now to recycle */
        dp->active = 0;

	/* decrement and possibly free any unsent messages for this client */
	while ((mp = (Msg*) popFQ(dp->msgq)) != NULL)
	    if (--mp->count == 0)
		freeMsg (mp);
	delFQ (dp->msgq);

        if (restart)
        {
            fprintf (stderr, "%s: Driver %s: restart #%d\n", indi_tstamp(NULL),
                                                    dp->name, ++dp->restarts);
            startDvr (dp);
        }

}

/* put Msg mp on queue of each driver responsible for dev, or all drivers
 * if dev not specified.
 */
static void
q2RDrivers (const char *dev, Msg *mp, XMLEle *root)
{
	int sawremote = 0;
	DvrInfo *dp;

	/* queue message to each interested driver.
	 * N.B. don't send generic getProps to more than one remote driver,
	 *   otherwise they all fan out and we get multiple responses back.
	 */
	for (dp = dvrinfo; dp < &dvrinfo[ndvrinfo]; dp++) {
	    int isremote = (dp->pid == REMOTEDVR);
            if (dp->active == 0)
                continue;
	    if (dev[0] && dp->dev[0] && strcmp (dev, dp->dev))
		continue;	/* driver known to not support this dev */
	    if (!dev[0] && isremote && sawremote)
		continue;	/* already sent generic to another remote */
	    if (isremote)
		sawremote = 1;

	    /* ok: queue message to this driver */
	    mp->count++;
	    pushFQ (dp->msgq, mp);
	    if (verbose > 1)
		fprintf (stderr, "%s: Driver %s: queuing responsible for <%s device='%s' name='%s'>\n",
				    indi_tstamp(NULL), dp->name, tagXMLEle(root),
				    findXMLAttValu (root, "device"),
				    findXMLAttValu (root, "name"));
	}
}

/* put Msg mp on queue of each driver snooping dev/name.
 * if BLOB always honor current mode.
 */
static void
q2SDrivers (int isblob, const char *dev, const char *name, Msg *mp, XMLEle *root)
{
	DvrInfo *dp;

	for (dp = dvrinfo; dp < &dvrinfo[ndvrinfo]; dp++) {
            Property *sp = findSDevice (dp, dev, name);

	    /* nothing for dp if not snooping for dev/name or wrong BLOB mode */
	    if (!sp)
		continue;
	    if ((isblob && sp->blob==B_NEVER) || (!isblob && sp->blob==B_ONLY))
		continue;

	    /* ok: queue message to this device */
	    mp->count++;
	    pushFQ (dp->msgq, mp);
	    if (verbose > 1) {
		fprintf (stderr, "%s: Driver %s: queuing snooped <%s device='%s' name='%s'>\n",
				    indi_tstamp(NULL), dp->name, tagXMLEle(root),
				    findXMLAttValu (root, "device"),
				    findXMLAttValu (root, "name"));
	    }
	}
}

/* add dev/name to dp's snooping list.
 * init with blob mode set to B_NEVER.
 */
static void
addSDevice (DvrInfo *dp, const char *dev, const char *name)
{
        Property *sp;
	char *ip;

	/* no dups */
	sp = findSDevice (dp, dev, name);
	if (sp)
	    return;

	/* add dev to sdevs list */
        dp->sprops = (Property*) realloc (dp->sprops,
                                            (dp->nsprops+1)*sizeof(Property));
	sp = &dp->sprops[dp->nsprops++];

        ip = sp->dev;
	strncpy (ip, dev, MAXINDIDEVICE-1);
	ip[MAXINDIDEVICE-1] = '\0';

        ip = sp->name;
	strncpy (ip, name, MAXINDINAME-1);
	ip[MAXINDINAME-1] = '\0';

	sp->blob = B_NEVER;

	if (verbose)
	    fprintf (stderr, "%s: Driver %s: snooping on %s.%s\n", indi_tstamp(NULL),
							dp->name, dev, name);
}

/* return Property if dp is snooping dev/name, else NULL.
 */
static Property *
findSDevice (DvrInfo *dp, const char *dev, const char *name)
{
	int i;

	for (i = 0; i < dp->nsprops; i++) {
            Property *sp = &dp->sprops[i];
            if (!strcmp (sp->dev, dev) &&
                    (!sp->name[0] || !strcmp(sp->name, name)))
		return (sp);
	}

	return (NULL);
}


/* put Msg mp on queue of each client interested in dev/name, except notme.
 * if BLOB always honor current mode.
 * return -1 if had to shut down any clients, else 0.
 */
static int
q2Clients (ClInfo *notme, int isblob, const char *dev, const char *name, Msg *mp, XMLEle *root)
{
	int shutany = 0;
	ClInfo *cp;
        int ql,i=0;

	/* queue message to each interested client */
	for (cp = clinfo; cp < &clinfo[nclinfo]; cp++) {
	    /* cp in use? notme? want this dev/name? blob? */
	    if (!cp->active || cp == notme)
		continue;
            if (findClDevice (cp, dev, name) < 0)
		continue;

            //if ((isblob && cp->blob==B_NEVER) || (!isblob && cp->blob==B_ONLY))
            if (!isblob && cp->blob==B_ONLY)
                continue;

            if (isblob)
            {
                if (cp->nprops > 0)
                {
                Property *pp = NULL;
                int blob_found=0;
                for (i = 0; i < cp->nprops; i++)
                {
                    pp = &cp->props[i];
                    if (!strcmp (pp->dev, dev) && (!strcmp(pp->name, name)))
                    {
                        blob_found = 1;
                        break;
                    }
                }

                if ( (blob_found && pp->blob == B_NEVER) || (blob_found==0 && cp->blob == B_NEVER) )
                    continue;
               }
               else if (cp->blob == B_NEVER)
                   continue;
           }

	    /* shut down this client if its q is already too large */
	    ql = msgQSize(cp->msgq);
	    if (ql > maxqsiz) {
		if (verbose)
		    fprintf (stderr, "%s: Client %d: %d bytes behind, shutting down\n",
						    indi_tstamp(NULL), cp->s, ql);
		shutdownClient (cp);
		shutany++;
		continue;
	    }

	    /* ok: queue message to this client */
	    mp->count++;
	    pushFQ (cp->msgq, mp);
	    if (verbose > 1)
		fprintf (stderr, "%s: Client %d: queuing <%s device='%s' name='%s'>\n",
				    indi_tstamp(NULL), cp->s, tagXMLEle(root),
				    findXMLAttValu (root, "device"),
				    findXMLAttValu (root, "name"));
	}

	return (shutany ? -1 : 0);
}

/* return size of all Msqs on the given q */
static int
msgQSize (FQ *q)
{
	int i, l = 0;

	for (i = 0; i < nFQ(q); i++) {
	    Msg *mp = (Msg *) peekiFQ(q,i);
	    l += mp->cl;
	}

	return (l);
}

/* print root as content in Msg mp.
 */
static void
setMsgXMLEle (Msg *mp, XMLEle *root)
{
	/* want cl to only count content, but need room for final \0 */
	mp->cl = sprlXMLEle (root, 0);
	if (mp->cl < sizeof(mp->buf))
	    mp->cp = mp->buf;
	else
	    mp->cp = malloc (mp->cl+1);
	sprXMLEle (mp->cp, root, 0);
}

/* save str as content in Msg mp.
 */
static void
setMsgStr (Msg *mp, char *str)
{
	/* want cl to only count content, but need room for final \0 */
	mp->cl = strlen (str);
	if (mp->cl < sizeof(mp->buf))
	    mp->cp = mp->buf;
	else
	    mp->cp = malloc (mp->cl+1);
	strcpy (mp->cp, str);
}

/* return pointer to one new nulled Msg
 */
static Msg *
newMsg (void)
{
	return ((Msg *) calloc (1, sizeof(Msg)));
}

/* free Msg mp and everything it contains */
static void
freeMsg (Msg *mp)
{
	if (mp->cp && mp->cp != mp->buf)
	    free (mp->cp);
	free (mp);
}

/* write the next chunk of the current message in the queue to the given
 * client. pop message from queue when complete and free the message if we are
 * the last one to use it. shut down this client if trouble.
 * N.B. we assume we will never be called with cp->msgq empty.
 * return 0 if ok else -1 if had to shut down.
 */
static int
sendClientMsg (ClInfo *cp)
{
	int nsend, nw;
	Msg *mp;

	/* get current message */
	mp = (Msg *) peekFQ (cp->msgq);

	/* send next chunk, never more than MAXWSIZ to reduce blocking */
	nsend = mp->cl - cp->nsent;
	if (nsend > MAXWSIZ)
	    nsend = MAXWSIZ;
	nw = write (cp->s, &mp->cp[cp->nsent], nsend);

	/* shut down if trouble */
	if (nw <= 0) {
	    if (nw == 0)
		fprintf (stderr, "%s: Client %d: write returned 0\n",
						    indi_tstamp(NULL), cp->s);
	    else
		fprintf (stderr, "%s: Client %d: write: %s\n", indi_tstamp(NULL),
						    cp->s, strerror(errno));
	    shutdownClient (cp);
	    return (-1);
	}

	/* trace */
	if (verbose > 2) {
	    fprintf(stderr, "%s: Client %d: sending msg copy %d nq %d:\n%.*s\n",
				indi_tstamp(NULL), cp->s, mp->count, nFQ(cp->msgq),
				nw, &mp->cp[cp->nsent]);
	} else if (verbose > 1) {
	    fprintf(stderr, "%s: Client %d: sending %.50s\n", indi_tstamp(NULL),
						    cp->s, &mp->cp[cp->nsent]);
	}

	/* update amount sent. when complete: free message if we are the last
	 * to use it and pop from our queue.
	 */
	cp->nsent += nw;
	if (cp->nsent == mp->cl) {
	    if (--mp->count == 0)
		freeMsg (mp);
	    popFQ (cp->msgq);
	    cp->nsent = 0;
	}

	return (0);
}

/* write the next chunk of the current message in the queue to the given
 * driver. pop message from queue when complete and free the message if we are
 * the last one to use it. restart this driver if touble.
 * N.B. we assume we will never be called with dp->msgq empty.
 * return 0 if ok else -1 if had to shut down.
 */
static int
sendDriverMsg (DvrInfo *dp)
{
	int nsend, nw;
	Msg *mp;

	/* get current message */
	mp = (Msg *) peekFQ (dp->msgq);

	/* send next chunk, never more than MAXWSIZ to reduce blocking */
	nsend = mp->cl - dp->nsent;
	if (nsend > MAXWSIZ)
	    nsend = MAXWSIZ;
	nw = write (dp->wfd, &mp->cp[dp->nsent], nsend);

	/* restart if trouble */
	if (nw <= 0) {
	    if (nw == 0)
		fprintf (stderr, "%s: Driver %s: write returned 0\n",
						    indi_tstamp(NULL), dp->name);
	    else
		fprintf (stderr, "%s: Driver %s: write: %s\n", indi_tstamp(NULL),
						    dp->name, strerror(errno));
            shutdownDvr (dp, 1);
	    return (-1);
	}

	/* trace */
	if (verbose > 2) {
	    fprintf(stderr, "%s: Driver %s: sending msg copy %d nq %d:\n%.*s\n",
			    indi_tstamp(NULL), dp->name, mp->count, nFQ(dp->msgq),
			    nw, &mp->cp[dp->nsent]);
	} else if (verbose > 1) {
	    fprintf(stderr, "%s: Driver %s: sending %.50s\n", indi_tstamp(NULL),
						dp->name, &mp->cp[dp->nsent]);
	}

	/* update amount sent. when complete: free message if we are the last
	 * to use it and pop from our queue.
	 */
	dp->nsent += nw;
	if (dp->nsent == mp->cl) {
	    if (--mp->count == 0)
		freeMsg (mp);
	    popFQ (dp->msgq);
	    dp->nsent = 0;
	}

	return (0);
}

/* return 0 if cp may be interested in dev/name else -1
 */
static int
findClDevice (ClInfo *cp, const char *dev, const char *name)
{
	int i;

        if (cp->allprops || !dev[0])
	    return (0);
        for (i = 0; i < cp->nprops; i++)
        {
            Property *pp = &cp->props[i];
            if (!strcmp (pp->dev, dev) && (!pp->name[0] || !strcmp(pp->name, name)))
		return (0);
	}
	return (-1);
}

/* add the given device and property to the devs[] list of client if new.
 */
static void
addClDevice (ClInfo *cp, const char *dev, const char *name, int isblob)
{
	Property *pp;
	char *ip;
        int i=0;

        if (isblob)
        {
            for (i = 0; i < cp->nprops; i++)
            {
                Property *pp = &cp->props[i];
                if (!strcmp (pp->dev, dev) && (!pp->name[0] || !strcmp(pp->name, name)))
                    return;
            }
        }
	/* no dups */
        else if (!findClDevice (cp, dev, name))
	    return;

	/* add */
	cp->props = (Property *) realloc (cp->props,
                                            (cp->nprops+1)*sizeof(Property));
	pp = &cp->props[cp->nprops++];

        /*ip = pp->dev;
	strncpy (ip, dev, MAXINDIDEVICE-1);
	ip[MAXINDIDEVICE-1] = '\0';

	ip = pp->name;
	strncpy (ip, name, MAXINDINAME-1);
        ip[MAXINDINAME-1] = '\0';*/

        strncpy (pp->dev, dev, MAXINDIDEVICE);
        strncpy (pp->name, name, MAXINDINAME);
        pp->blob = B_NEVER;
}


/* block to accept a new client arriving on lsocket.
 * return private nonblocking socket or exit.
 */
static int
newClSocket ()
{
	struct sockaddr_in cli_socket;
	socklen_t cli_len;
	int cli_fd;

	/* get a private connection to new client */
	cli_len = sizeof(cli_socket);
	cli_fd = accept (lsocket, (struct sockaddr *)&cli_socket, &cli_len);
	if(cli_fd < 0) {
	    fprintf (stderr, "accept: %s\n", strerror(errno));
	    Bye();
	}

	/* ok */
	return (cli_fd);
}

/* convert the string value of enableBLOB to our B_ state value.
 * no change if unrecognized
 */
static void
crackBLOB (const char *enableBLOB, BLOBHandling *bp)
{
	if (!strcmp (enableBLOB, "Also"))
	    *bp = B_ALSO;
	else if (!strcmp (enableBLOB, "Only"))
	    *bp = B_ONLY;
	else if (!strcmp (enableBLOB, "Never"))
	    *bp = B_NEVER;
}

/* Update the client property BLOB handling policy */
static void crackBLOBHandling(const char *dev, const char *name, const char *enableBLOB, ClInfo *cp)
{
    int i=0;

    /* If we have EnableBLOB with property name, we add it to Client device list */
    if (name[0])
        addClDevice (cp, dev, name, 1);
    else
    /* Otherwise, we set the whole client blob handling to what's passed (enableBLOB) */
        crackBLOB(enableBLOB, &cp->blob);

    /* If whole client blob handling policy was updated, we need to pass that also to all children
       and if the request was for a specific property, then we apply the policy to it */
    for (i = 0; i < cp->nprops; i++)
    {
        Property *pp = &cp->props[i];
        if (!name[0])           
            crackBLOB(enableBLOB, &pp->blob);
        else if (!strcmp (pp->dev, dev) && (!strcmp(pp->name, name)))
        {
            crackBLOB(enableBLOB, &pp->blob);
            return;
        }
    }
}

/* print key attributes and values of the given xml to stderr.
 */
static void
traceMsg (XMLEle *root)
{
	static const char *prtags[] = {
	    "defNumber", "oneNumber",
	    "defText",   "oneText",
	    "defSwitch", "oneSwitch",
	    "defLight",  "oneLight",
	};
	XMLEle *e;
	const char *msg, *perm, *pcd;
	unsigned int i;

	/* print tag header */
	fprintf (stderr, "%s %s %s %s", tagXMLEle(root),
						findXMLAttValu(root,"device"),
						findXMLAttValu(root,"name"),
						findXMLAttValu(root,"state"));
	pcd = pcdataXMLEle (root);
	if (pcd[0])
	    fprintf (stderr, " %s", pcd);
	perm = findXMLAttValu(root,"perm");
	if (perm[0])
	    fprintf (stderr, " %s", perm);
	msg = findXMLAttValu(root,"message");
	if (msg[0])
	    fprintf (stderr, " '%s'", msg);

	/* print each array value */
	for (e = nextXMLEle(root,1); e; e = nextXMLEle(root,0))
	    for (i = 0; i < sizeof(prtags)/sizeof(prtags[0]); i++)
		if (strcmp (prtags[i], tagXMLEle(e)) == 0)
		    fprintf (stderr, "\n %10s='%s'", findXMLAttValu(e,"name"),
							    pcdataXMLEle (e));

	fprintf (stderr, "\n");
}

/* fill s with current UT string.
 * if no s, use a static buffer
 * return s or buffer.
 * N.B. if use our buffer, be sure to use before calling again
 */
static char *
indi_tstamp (char *s)
{
	static char sbuf[64];
	struct tm *tp;
	time_t t;

	time (&t);
	tp = gmtime (&t);
	if (!s)
	    s = sbuf;
	strftime (s, sizeof(sbuf), "%Y-%m-%dT%H:%M:%S", tp);
	return (s);
}

/* log message in root known to be from device dev to ldir, if any.
 */
static void
logDMsg (XMLEle *root, const char *dev)
{
        char stamp[64];
        char logfn[1024];
        const char *ts, *ms;
        FILE *fp;

        /* get message, if any */
        ms = findXMLAttValu (root, "message");
        if (!ms[0])
            return;

        /* get timestamp now if not provided */
        ts = findXMLAttValu (root, "timestamp");
        if (!ts[0])
        {
            indi_tstamp (stamp);
            ts = stamp;
        }

        /* append to log file, name is date portion of time stamp */
        sprintf (logfn, "%s/%.10s.islog", ldir, ts);
        fp = fopen (logfn, "a");
        if (!fp)
            return;	/* oh well */
        fprintf (fp, "%s: %s: %s\n", ts, dev, ms);
        fclose (fp);
}

/* log when then exit */
static void
Bye()
{
	fprintf (stderr, "%s: good bye\n", indi_tstamp(NULL));
	exit(1);
}
