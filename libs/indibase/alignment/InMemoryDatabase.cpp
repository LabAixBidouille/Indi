/*!
 * \file InMemoryDatabase.cpp
 *
 * \author Roger James
 * \date 13th November 2013
 *
 */

#include "InMemoryDatabase.h"

#include "indibase/basedevice.h"
#include "indicom.h"

#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>

namespace INDI {
namespace AlignmentSubsystem {

bool InMemoryDatabase::LoadDatabase(const char* DeviceName)
{
    char DatabaseFileName[MAXRBUF];
    char Errmsg[MAXRBUF];
    XMLEle *FileRoot = NULL;
    XMLEle *EntryRoot = NULL;
    XMLEle *Element = NULL;
    LilXML *Parser = newLilXML();

    FILE *fp = NULL;

    snprintf(DatabaseFileName, MAXRBUF, "%s/.indi/%s_alignment_database.xml", getenv("HOME"), DeviceName);


    fp = fopen(DatabaseFileName, "r");
    if (fp == NULL)
    {
         snprintf(Errmsg, MAXRBUF, "Unable to read alignment database file. Error loading file %s: %s\n", DatabaseFileName, strerror(errno));
         return false;
    }

    FileRoot = readXMLFile(fp, Parser, Errmsg);

    if (FileRoot == NULL)
    {
        snprintf(Errmsg, MAXRBUF, "Unable to parse database XML: %s", Errmsg);
        return false;
    }

    if (strcmp(tagXMLEle(FileRoot), "INDIAlignmentDatabase") != 0)
    {
        return false;
    }

    MySyncPoints.clear();

    for (EntryRoot = nextXMLEle (FileRoot, 1); EntryRoot != NULL; EntryRoot = nextXMLEle (FileRoot, 0))
    {
        AlignmentDatabaseEntry CurrentValues;
        if (strcmp(tagXMLEle(EntryRoot), "INDIAlignmentDatabaseEntry") != 0)
        {
            return false;
        }
        for (Element = nextXMLEle (EntryRoot, 1); Element != NULL; Element = nextXMLEle (EntryRoot, 0))
        {
            if (strcmp(tagXMLEle(Element), "ObservationDate") == 0)
            {
                sscanf(pcdataXMLEle(Element), "%d", &CurrentValues.ObservationDate);
            }
            else if (strcmp(tagXMLEle(Element), "ObservationTime") == 0)
            {
                f_scansexa(pcdataXMLEle(Element), &CurrentValues.ObservationTime);
            }
            else if (strcmp(tagXMLEle(Element), "RightAscension") == 0)
            {
                f_scansexa(pcdataXMLEle(Element), &CurrentValues.RightAscension);
            }
            else if (strcmp(tagXMLEle(Element), "Declination") == 0)
            {
                f_scansexa(pcdataXMLEle(Element), &CurrentValues.Declination);
            }
            else if (strcmp(tagXMLEle(Element), "TelescopeDirectionVectorX") == 0)
            {
                sscanf(pcdataXMLEle(Element), "%lf", &CurrentValues.TelescopeDirection.x);
            }
            else if (strcmp(tagXMLEle(Element), "TelescopeDirectionVectorY") == 0)
            {
                sscanf(pcdataXMLEle(Element), "%lf", &CurrentValues.TelescopeDirection.y);
            }
            else if (strcmp(tagXMLEle(Element), "TelescopeDirectionVectorZ") == 0)
            {
                sscanf(pcdataXMLEle(Element), "%lf", &CurrentValues.TelescopeDirection.z);
            }
            else
                return false;
        }
        MySyncPoints.push_back(CurrentValues);
    }

    fclose(fp);
    delXMLEle(FileRoot);
    delXMLEle(EntryRoot);
    delXMLEle(Element);
    delLilXML(Parser);

    if (NULL != LoadDatabaseCallback)
        (*LoadDatabaseCallback)(LoadDatabaseCallbackThisPointer);

    return true;

}

bool InMemoryDatabase::SaveDatabase(const char* DeviceName)
{
    char ConfigDir[MAXRBUF];
    char DatabaseFileName[MAXRBUF];
    char Errmsg[MAXRBUF];
    struct stat Status;
    FILE* fp;

    snprintf(ConfigDir, MAXRBUF, "%s/.indi/", getenv("HOME"));
    snprintf(DatabaseFileName, MAXRBUF, "%s%s_alignment_database.xml", ConfigDir, DeviceName);

    if(stat(ConfigDir, &Status) != 0)
    {
        if (mkdir(ConfigDir, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) < 0)
        {
            snprintf(Errmsg, MAXRBUF, "Unable to create config directory. Error %s: %s\n", ConfigDir, strerror(errno));
            return false;
        }
    }

    fp = fopen(DatabaseFileName, "w");
    if (fp == NULL)
    {
        snprintf(Errmsg, MAXRBUF, "Unable to open database file. Error opening file %s: %s\n", DatabaseFileName, strerror(errno));
        return false;
    }

    fprintf(fp, "<INDIAlignmentDatabase>\n");

    for (AlignmentDatabaseType::const_iterator Itr = MySyncPoints.begin(); Itr != MySyncPoints.end(); Itr++)
    {
        char SexaString[12]; // Long enough to hold xx:xx:xx.xx
        fprintf(fp, "   <INDIAlignmentDatabaseEntry>\n");

        fprintf(fp, "      <ObservationDate>%d</ObservationDate>\n", (*Itr).ObservationDate);
        fs_sexa(SexaString, (*Itr).ObservationTime, 2, 360000);
        fprintf(fp, "      <ObservationTime>%s</ObservationTime>\n", SexaString);
        fs_sexa(SexaString, (*Itr).RightAscension, 2, 3600);
        fprintf(fp, "      <RightAscension>%s</RightAscension>\n", SexaString);
        fs_sexa(SexaString, (*Itr).Declination, 2, 3600);
        fprintf(fp, "      <Declination>%s</Declination>\n", SexaString);
        fprintf(fp, "      <TelescopeDirectionVectorX>%lf</TelescopeDirectionVectorX>\n", (*Itr).TelescopeDirection.x);
        fprintf(fp, "      <TelescopeDirectionVectorY>%lf</TelescopeDirectionVectorY>\n", (*Itr).TelescopeDirection.y);
        fprintf(fp, "      <TelescopeDirectionVectorZ>%lf</TelescopeDirectionVectorZ>\n", (*Itr).TelescopeDirection.z);

        fprintf(fp, "   </INDIAlignmentDatabaseEntry>\n");
    }

    fprintf(fp, "</INDIAlignmentDatabase>\n");

    fclose(fp);

    return true;
}

void InMemoryDatabase::SetLoadDatabaseCallback(LoadDatabaseCallbackPointer CallbackPointer, void *ThisPointer)
{
    LoadDatabaseCallback = CallbackPointer;
    LoadDatabaseCallbackThisPointer = ThisPointer;
}


} // namespace AlignmentSubsystem
} // namespace INDI
