#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

#include "ApnCamera.h"

int parse_options (int argc, char **argv);
static void wfits (unsigned short *p, int w, int h);

/* Declare the camera object. All camera functions and parameters are
 * accessed using the methods and instance variables of this object
 *
 * Their declarations can be found in ApnCamera.h
 */

CApnCamera *alta;

/* Declare globals to store the input command line options */

char imagename[256];
double texposure=1.0;
int  shutter=1;
int ip[4]={0,0,0,0};
int xbin=1;
int ybin=1;
int xstart=0;
int xend=0;
int ystart=0;
int yend=0;
int biascols=0;
int fanmode=0;
double cooling=99.0;
int numexp=1;
int ipause=0;
int verbose=0;
int camnum=1;
int highspeed=0;



/* Main executable starts here -----------------------------------------------*/

int main (int argc, char **argv) 
{

	int status;
	unsigned short *pImageBuffer;
	unsigned short Width, Height;
	unsigned long Count;
	int i;
        double t;

	alta = (CApnCamera *)new CApnCamera();
	if (!alta->InitDriver(camnum,0,0)) {
	    printf ("Init failed\n");
	    exit(1);
	}
	alta->ResetSystem();

	/* overscan */
	alta->write_DigitizeOverscan(1);
	alta->write_RoiPixelsH (alta->read_OverscanColumns()
				    + alta->m_ApnSensorInfo->m_ImagingColumns);
	printf ("RoiPixelsH = %d\n", alta->read_RoiPixelsH());

/*	Set the desired fan mode */
        alta->write_FanMode(0);
        alta->write_FanMode(atoi(argv[1]));
	printf ("temp now %g\n", alta->read_TempCCD());

	/* set led states */
	alta->write_LedState (0, Apn_LedState_Expose);
	alta->write_LedState (1, Apn_LedState_AtTemp);


/*	If a particular CCD temperature was requested, then enable 
	cooling and wait for the correct setpoint value */
        if ((cooling = atof(argv[2])) < 20) {
	   printf ("setting cooler temp %.1f\n", cooling);
           alta->write_CoolerEnable(1);
           alta->write_CoolerSetPoint(cooling);

/*	   loop until we get within 0.2 degrees, about the best we can hope for */
           do {
               sleep(1);
               t = alta->read_TempCCD();
               printf ("Cooler temp = %.1f, status = %d\n", t, alta->read_CoolerStatus());
           } while (fabs(cooling - t) > 0.2);
        }


	texposure = atof (argv[3]);

/*	Setup binning, defaults to full frame */
        i = 0;

/*	Set up a region of interest, defaults to full frame */
        if (xstart > 0) {
          alta->m_pvtRoiStartX = xstart;
          alta->m_pvtRoiStartY = ystart;
          alta->m_pvtRoiPixelsH = (xend-xstart+1);
          alta->m_pvtRoiPixelsV = (yend-ystart+1);
        }

/*      Set up binning */
	xbin = ybin = 2;
        alta->m_pvtRoiPixelsH = alta->m_pvtRoiPixelsH / xbin;
        alta->m_pvtRoiPixelsV = alta->m_pvtRoiPixelsV / ybin;
	alta->write_RoiBinningH(xbin);
	alta->write_RoiBinningV(ybin);

/*	Single image per download */
	alta->write_ImageCount(1);

/*	Start an exposure */
	status = alta->Expose(texposure,shutter);

/*	Wait until done */
	for (i = 0; i < texposure+2; i++) {
	    if (alta->read_ImagingStatus() == Apn_Status_ImageReady)
		break;
	    printf ("waiting...\n");
	    sleep (1);
	}

/*	Readout the image and save in a named buffer (tempobs) */
	pImageBuffer = (unsigned short *) calloc (17000000, 2);
	printf ("%ld\n", alta->GetImageData(pImageBuffer, Width, Height,Count));
	printf ("W=%d H=%d C=%ld\n", Width, Height, Count);
	wfits (pImageBuffer, Width, Height);


/*	All done, tidy up */
	alta->CloseDriver();

}

/* hack out a FITS file */
static void
wfits (unsigned short *pix, int w, int h)
{
	unsigned short *p, *pend;
	int npix = w*h;
	FILE *fp;
	int n;

	/* swap bytes ... fits is BE, intel is LE */
	for (p = pix, pend = p + npix; p < pend; p++)
	    *p = (*p<<8) | ((*p>>8)&0xff);

	fp = fopen ("x.fts", "w");
	n = 0;
	n += fprintf (fp, "%-80s", "SIMPLE  =                    T");
	n += fprintf (fp, "%-80s", "BITPIX  =                   16");
	n += fprintf (fp, "%-80s", "NAXIS   =                    2");
	n += fprintf (fp, "NAXIS1  =                %5d%50s", w, "");
	n += fprintf (fp, "NAXIS2  =                %5d%50s", h, "");
	// n += fprintf (fp, "%-80s", "BZERO   =                32768");
	// n += fprintf (fp, "%-80s", "BSCALE  =                    1");
	n += fprintf (fp, "%-80s", "END");
	n += fprintf (fp, "%*s", 2880-n, "");
	n += fwrite (pix, 2, npix, fp);
	fprintf (fp, "%*s", 2880 - (n%2880), "");
	fclose (fp);
}

/*  Helper routines start here-------------------------------------------------*/

/*  This routine provides a very simple command line parser
 *  Unknown options should be ignored, but strict type
 *  checking is NOT done.
 */

int parse_options (int argc, char **argv)
{
   int i;
   int goti,gott,gots,gota;

/* Zero out counters for required options */
   goti=0;
   gott=0;
   gots=0;
   gota=0;
   i=1;

/* Default fanmode to medium */
   fanmode = 2;

/* Loop thru all provided options */
   while (i<argc) {

/*     Image name */
       if (!strncmp(argv[i],"-i",2)) {
          strcpy(imagename,argv[i+1]);
          goti = 1;
       }

/*     Exposure time */
       if (!strncmp(argv[i],"-t",2)) {
          sscanf(argv[i+1],"%lf",&texposure);
          gott = 1;
       }

/*     Shutter state */
       if (!strncmp(argv[i],"-s",2)) {
          sscanf(argv[i+1],"%d",&shutter);
          gots= 1;
       }

/*     IP address for ALTA-E models */
       if (!strncmp(argv[i],"-a",2)) {
          sscanf(argv[i+1],"%d.%d.%d.%d",ip,ip+1,ip+2,ip+3);
          gota = 1;
       }

/*     Fast readout mode for ALTA-U models */
       if (!strncmp(argv[i],"-F",2)) {
          sscanf(argv[i+1],"%d",&highspeed);
       }

/*     Horizontal binning */
       if (!strncmp(argv[i],"-x",2)) {
          sscanf(argv[i+1],"%d",&xbin);
       }

/*     Vertical binning */
       if (!strncmp(argv[i],"-y",2)) {
          sscanf(argv[i+1],"%d",&ybin);
       }

/*     Region of interest */
       if (!strncmp(argv[i],"-r",2)) {
          sscanf(argv[i+1],"%d,%d,%d,%d",&xstart,&ystart,&xend,&yend);
       }

/*     Bias subtraction */
       if (!strncmp(argv[i],"-b",2)) {
          sscanf(argv[i+1],"%d",&biascols);
       }

/*     Fan mode */
       if (!strncmp(argv[i],"-f",2)) {
          if (!strncmp(argv[i+1],"off",3)==0) fanmode=0;
          if (!strncmp(argv[i+1],"slow",4)==0) fanmode=1;
          if (!strncmp(argv[i+1],"medium",6)==0) fanmode=2;
          if (!strncmp(argv[i+1],"fast",4)==0) fanmode=3;
       }

/*     Setpoint temperature */
       if (!strncmp(argv[i],"-c",2)) {
          sscanf(argv[i+1],"%lf",&cooling);
       }

/*     Sequence of exposures */
       if (!strncmp(argv[i],"-n",2)) {
          sscanf(argv[i+1],"%d",&numexp);
       }

/*     USB camera number */
       if (!strncmp(argv[i],"-u",2)) {
          sscanf(argv[i+1],"%d",&camnum);
       }

/*     Interval to pause between exposures */
       if (!strncmp(argv[i],"-p",2)) {
          sscanf(argv[i+1],"%d",&ipause);
       }

/*     Be more verbose */
       if (!strncmp(argv[i],"-v",2)) {
          sscanf(argv[i+1],"%d",&verbose);
       }

/*     Print usage info */
       if (!strncmp(argv[i],"-h",2)) {
          printf("Apogee image tester -  Usage: \n \
	 -i imagename    Name of image (required) \n \
	 -t time         Exposure time is seconds (required)\n \
	 -s 0/1          1 = Shutter open, 0 = Shutter closed (required)\n \
	 -a a.b.c.d      IP address of camera e.g. 192.168.0.1 (required for ALTA-E models only)\n \
	 -F 0/1          Fast readout mode (ALTA-U models only)\n \
	 -u num          Camera number (default 1 , ALTA-U only) \n \
	 -x num          Binning factor in x, default 1 \n \
	 -y num          Binning factor in y, default 1 \n \
	 -r xs,ys,xe,ye  Image subregion in the format startx,starty,endx,endy \n \
	 -b biascols     Number of Bias columns to subtract \n \
	 -f mode         Fanmode during exposure, off,slow,medium,fast (default medium) \n \
	 -c temp         Required temperature for exposure, default is current value \n \
	 -n num          Number of exposures \n \
	 -p time         Number of seconds to pause between multiple exposures \n \
	 -v verbosity    Print more details about exposure\n");
         exit(0);
        }

/*      All options are 2 args long! */
        i = i+2;
   } 

/* Complain about missing required options, then give up */
   if ( goti == 0 ) printf("Missing argument  -i imagename\n");
   if ( gott == 0 ) printf("Missing argument  -t exposure time\n");
   if ( gots == 0 ) printf("Missing argument  -s shutter state\n");
#ifdef ALTA_NET
   if ( gota == 0 ) printf("Missing argument  -a IP address\n");
   if (goti+gots+gott+gota != 4) exit(1);
#else
   if (goti+gots+gott != 3) exit(1);
#endif

/* Print exposure details */
   if (verbose > 0) {
      printf("Apogee ALTA image test - V1.6\n");
      printf("	Image name is %s\n",imagename);
      printf("	Exposure time is %lf\n",texposure);
      if (numexp    > 1) printf("	Sequence of %d exposures requested\n",numexp);
      if (ipause    > 0.0) printf("	Pause of %d seconds between exposures\n",ipause);
      printf("	Shutter state during exposure will be %d\n",shutter);
#ifdef ALTA_NET
      if (ip[0]     != 0) printf("	ALTA-E ip address is %d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
#endif
      if (xbin      > 1) printf("	X binning selected xbin=%d\n",xbin);
      if (ybin      > 1) printf("	Y binning selected ybin=%d\n",ybin);
      if (xstart    != 0) printf("	Subregion readout %d,%d,%d,%d\n",xstart,xend,ystart,yend);
      if (biascols  != 0) printf("	Bias subtraction using %d columns\n",biascols);
      if (fanmode > 0) printf("	Fan set to mode = %d\n",fanmode);
      if (cooling < 99.0) printf("	Requested ccd temperature for exposure is %lf\n",cooling);
   }
   return(0);

}
