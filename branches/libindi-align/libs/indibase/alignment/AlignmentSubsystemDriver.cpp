/*!
 * \file AlignmentSubsystemDriver.cpp
 *
 * \author Roger James
 * \date 13th November 2013
 *
 * This file contains implementations of functions for the alignment subsystem.
 */

#include "AlignmentSubsystem.h"

#include "indicom.h"

#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <sys/stat.h>
#include <cfloat>

bool INDI::AlignmentSubsystemDriver::EnumerateMathPlugins(std::vector<std::string>& MathPlugins)
{
    MathPlugins.clear();
    MathPlugins.push_back("Builtin Math Plugin");
    return true;
}

bool INDI::AlignmentSubsystemDriver::SelectMathPlugin(const std::string& MathPluginName)
{
    return true;
}

const INDI::AlignmentSubsystemMathPlugin* INDI::AlignmentSubsystemDriver::GetMathPluginPointer(void)
{
    return CurrentMathPlugin;
}

void INDI::AlignmentSubsystemDriver::InitAlignmentProperties(Telescope* ChildTelescope)
{
    // TODO Find out the available math plugins and populate the array
    // Just use the default built in plugin for the time being
    AlignmentSubsystemMathPlugins.reset(new ISwitch[1]);
    IUFillSwitch(AlignmentSubsystemMathPlugins.get(), "INBUILT_MATH_PLUGIN", "Inbuilt Math Plugin", ISS_ON);
    IUFillSwitchVector(&AlignmentSubsystemMathPluginsV, AlignmentSubsystemMathPlugins.get(), 1, ChildTelescope->getDeviceName(),
                    "ALIGNMENT_SUBSYSTEM_MATH_PLUGINS", "Math Plugins", ALIGNMENT_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);
    ChildTelescope->registerProperty(&AlignmentSubsystemMathPluginsV, INDI_SWITCH);

    // The following property is used for configuration purposes only and is not exposed to the client.
    IUFillText(&AlignmentSubsystemCurrentMathPlugin, "ALIGNMENT_SUBSYSTEM_CURRENT_MATH_PLUGIN", "Current Math Plugin",
        AlignmentSubsystemMathPlugins.get()[0].label);
    IUFillTextVector(&AlignmentSubsystemCurrentMathPluginV, &AlignmentSubsystemCurrentMathPlugin, 1, ChildTelescope->getDeviceName(),
                "ALIGNMENT_SUBSYSTEM_CURRENT_MATH_PLUGIN", "Current Math Plugin", ALIGNMENT_TAB, IP_RO, 60, IPS_IDLE);

    IUFillNumber(&AlignmentPointSetEntry[0], "ALIGNMENT_POINT_ENTRY_OBSERVATION_JULIAN_DATE", "Observation Julian date", "%g", 0, 60000, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[1], "ALIGNMENT_POINT_ENTRY_OBSERVATION_LOCAL_SIDEREAL_TIME", "Observation local sidereal time (hh:mm:ss.ss)", "%010.9m", 0, 24, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[2], "ALIGNMENT_POINT_ENTRY_RA", "Right Ascension (hh:mm:ss)", "%010.6m", 0, 24, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[3]," ALIGNMENT_POINT_ENTRY_DEC", "Declination (dd:mm:ss)", "%010.6m", -90, 90, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[4], "ALIGNMENT_POINT_ENTRY_VECTOR_X", "Telescope direction vector x", "%g", -FLT_MAX, FLT_MAX, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[5]," ALIGNMENT_POINT_ENTRY_VECTOR_Y", "Telescope direction vector y", "%g", -FLT_MAX, FLT_MAX, 0, 0);
    IUFillNumber(&AlignmentPointSetEntry[6]," ALIGNMENT_POINT_ENTRY_VECTOR_Z", "Telescope direction vector z", "%g", -FLT_MAX, FLT_MAX, 0, 0);
    IUFillNumberVector(&AlignmentPointSetEntryV, AlignmentPointSetEntry, 7, ChildTelescope->getDeviceName(),
                    "ALIGNMENT_POINT_MANDATORY_NUMBERS", "Mandatory sync point numeric fields", ALIGNMENT_TAB, IP_RW, 60, IPS_IDLE);
    ChildTelescope->registerProperty(&AlignmentPointSetEntryV, INDI_NUMBER);

    IUFillBLOB(&AlignmentPointSetPrivateBinaryData, "ALIGNMENT_POINT_ENTRY_PRIVATE", "Private binary data", "");
    IUFillBLOBVector(&AlignmentPointSetPrivateBinaryDataV, &AlignmentPointSetPrivateBinaryData, 1, ChildTelescope->getDeviceName(),
                    "ALIGNMENT_POINT_OPTIONAL_BINARY_BLOB", "Optional sync point binary data", ALIGNMENT_TAB, IP_RW, 60, IPS_IDLE);
    ChildTelescope->registerProperty(&AlignmentPointSetPrivateBinaryDataV, INDI_BLOB);

    IUFillNumber(&AlignmentPointSetSize, "ALIGNMENT_POINTSET_SIZE", "Size", "%g", 0, 100000, 0, 0);
    IUFillNumberVector(&AlignmentPointSetSizeV, &AlignmentPointSetSize, 1, ChildTelescope->getDeviceName(),
                    "ALIGNMENT_POINTSET_SIZE", "Current Set", ALIGNMENT_TAB, IP_RO, 60, IPS_IDLE);
    ChildTelescope->registerProperty(&AlignmentPointSetSizeV, INDI_NUMBER);

    IUFillNumber(&AlignmentPointSetPointer, "ALIGNMENT_POINTSET_CURRENT_ENTRY", "Pointer", "%g", 0, 100000, 0, 0);
    IUFillNumberVector(&AlignmentPointSetPointerV, &AlignmentPointSetPointer, 1, ChildTelescope->getDeviceName(),
                    "ALIGNMENT_POINTSET_CURRENT_ENTRY", "Current Set", ALIGNMENT_TAB, IP_RW, 60, IPS_IDLE);
    ChildTelescope->registerProperty(&AlignmentPointSetPointerV, INDI_NUMBER);

    IUFillSwitch(&AlignmentPointSetAction[0], "APPEND", "Add entries at end of set", ISS_ON);
    IUFillSwitch(&AlignmentPointSetAction[1], "INSERT", "Insert entries at current index", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[2], "EDIT", "Overwrite entry at current index", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[3], "DELETE", "Delete entry at current index", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[4], "CLEAR","Delete all the entries in the set", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[5], "READ", "Read the entry at the current pointer", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[6], "READ INCREMENT", "Increment the pointer after reading the entry", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[7], "LOAD DATABASE", "Load the alignment database from local storage", ISS_OFF);
    IUFillSwitch(&AlignmentPointSetAction[8], "SAVE DATABASE", "Save the alignment database to local storage", ISS_OFF);
    IUFillSwitchVector(&AlignmentPointSetActionV, AlignmentPointSetAction, 9, ChildTelescope->getDeviceName(),
                    "ALIGNMENT_POINTSET_ACTION", "Action to take", ALIGNMENT_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);
    ChildTelescope->registerProperty(&AlignmentPointSetActionV, INDI_SWITCH);

    IUFillSwitch(&AlignmentPointSetCommit, "ALIGNMENT_POINTSET_COMMIT", "Execute the action", ISS_ON);
    IUFillSwitchVector(&AlignmentPointSetCommitV, &AlignmentPointSetCommit, 1, ChildTelescope->getDeviceName(),
                    "ALIGNMENT_POINTSET_COMMIT", "Execute", ALIGNMENT_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);
    ChildTelescope->registerProperty(&AlignmentPointSetCommitV, INDI_SWITCH);
}

void INDI::AlignmentSubsystemDriver::ProcessAlignmentTextProperties(Telescope* pTelescope, const char *name, char *texts[], char *names[], int n)
{
    DEBUGFDEVICE(pTelescope->getDeviceName(), INDI::Logger::DBG_DEBUG, "ProcessAlignmentTextProperties - name(%s)", name);
    if (strcmp(name, AlignmentSubsystemCurrentMathPluginV.name) == 0)
    {
        AlignmentSubsystemCurrentMathPluginV.s = IPS_OK;
        IUUpdateText(&AlignmentSubsystemCurrentMathPluginV, texts, names, n);
    }
}

void INDI::AlignmentSubsystemDriver::ProcessAlignmentNumberProperties(Telescope* pTelescope, const char *name, double values[], char *names[], int n)
{
    DEBUGFDEVICE(pTelescope->getDeviceName(), INDI::Logger::DBG_DEBUG, "ProcessAlignmentNumberProperties - name(%s)", name);
    if (strcmp(name, AlignmentPointSetEntryV.name) == 0)
    {
        AlignmentPointSetEntryV.s = IPS_OK;
        IUUpdateNumber(&AlignmentPointSetEntryV, values, names, n);
        //  Update client display
        IDSetNumber(&AlignmentPointSetEntryV, NULL);
    } else if (strcmp(name, AlignmentPointSetPointerV.name) == 0)
    {
        AlignmentPointSetPointerV.s = IPS_OK;
        IUUpdateNumber(&AlignmentPointSetPointerV, values, names, n);
        //  Update client display
        IDSetNumber(&AlignmentPointSetPointerV, NULL);
    }
}

void INDI::AlignmentSubsystemDriver::ProcessAlignmentSwitchProperties(Telescope* pTelescope, const char *name, ISState *states, char *names[], int n)
{
    DEBUGFDEVICE(pTelescope->getDeviceName(), INDI::Logger::DBG_DEBUG, "ProcessAlignmentSwitchProperties - name(%s)", name);
    if (strcmp(name, AlignmentPointSetActionV.name) == 0)
    {
        AlignmentPointSetActionV.s=IPS_OK;
        IUUpdateSwitch(&AlignmentPointSetActionV, states, names, n);
        //  Update client display
        IDSetSwitch(&AlignmentPointSetActionV, NULL);
    } else if (strcmp(name, AlignmentPointSetCommitV.name) == 0)
    {
        AlignmentPointSetCommitV.s=IPS_OK;
        IUUpdateSwitch(&AlignmentPointSetCommitV, states, names, n);

        // Perform the database action
        AlignmentDatabaseEntry CurrentValues;
        CurrentValues.ObservationDate = AlignmentPointSetEntry[ENTRY_OBSERVATION_JULIAN_DATE].value;
        CurrentValues.ObservationTime = AlignmentPointSetEntry[ENTRY_OBSERVATION_LOCAL_SIDEREAL_TIME].value;
        CurrentValues.RightAscension = AlignmentPointSetEntry[ENTRY_RA].value;
        CurrentValues.ObservationTime = AlignmentPointSetEntry[ENTRY_DEC].value;
        CurrentValues.TelescopeDirection.x = AlignmentPointSetEntry[ENTRY_VECTOR_X].value;
        CurrentValues.TelescopeDirection.y = AlignmentPointSetEntry[ENTRY_VECTOR_Y].value;
        CurrentValues.TelescopeDirection.z = AlignmentPointSetEntry[ENTRY_VECTOR_Z].value;

        if (AlignmentPointSetAction[APPEND].s == ISS_ON)
        {
            AppendSyncPoint(CurrentValues);
            AlignmentPointSetSize.value = GetDatabaseSize();
            //  Update client display
            IDSetNumber(&AlignmentPointSetSizeV, NULL);
        }
        else if (AlignmentPointSetAction[INSERT].s == ISS_ON)
        {
            InsertSyncPoint(int(AlignmentPointSetPointer.value), CurrentValues);
            AlignmentPointSetSize.value = GetDatabaseSize();
            //  Update client display
            IDSetNumber(&AlignmentPointSetSizeV, NULL);
        }
        else if (AlignmentPointSetAction[EDIT].s == ISS_ON)
        {
            EditSyncPoint(int(AlignmentPointSetPointer.value), CurrentValues);
        }
        else if (AlignmentPointSetAction[DELETE].s == ISS_ON)
        {
            DeleteSyncPoint(int(AlignmentPointSetPointer.value));
            AlignmentPointSetSize.value = GetDatabaseSize();
            //  Update client display
            IDSetNumber(&AlignmentPointSetSizeV, NULL);
        }
        else if (AlignmentPointSetAction[CLEAR].s == ISS_ON)
        {
            // SyncPointsType().swap(SyncPoints); // Do it this wasy to force a reallocation
            ClearSyncPoints();
            AlignmentPointSetSize.value = GetDatabaseSize();
            //  Update client display
            IDSetNumber(&AlignmentPointSetSizeV, NULL);
        }
        else if ((AlignmentPointSetAction[READ].s == ISS_ON) || (AlignmentPointSetAction[READ_INCREMENT].s == ISS_ON))
        {
            ReadSyncPoint(int(AlignmentPointSetPointer.value), CurrentValues);
            AlignmentPointSetEntry[ENTRY_OBSERVATION_JULIAN_DATE].value = CurrentValues.ObservationDate;
            AlignmentPointSetEntry[ENTRY_OBSERVATION_LOCAL_SIDEREAL_TIME].value = CurrentValues.ObservationTime;
            AlignmentPointSetEntry[ENTRY_RA].value = CurrentValues.RightAscension;
            AlignmentPointSetEntry[ENTRY_DEC].value = CurrentValues.ObservationTime;
            AlignmentPointSetEntry[ENTRY_VECTOR_X].value = CurrentValues.TelescopeDirection.x;
            AlignmentPointSetEntry[ENTRY_VECTOR_Y].value = CurrentValues.TelescopeDirection.y;
            AlignmentPointSetEntry[ENTRY_VECTOR_Z].value= CurrentValues.TelescopeDirection.z;

            //  Update client display
            IDSetNumber(&AlignmentPointSetEntryV, NULL);
            if (AlignmentPointSetAction[READ_INCREMENT].s == ISS_ON)
            {
                AlignmentPointSetPointer.value++;
                IDSetNumber(&AlignmentPointSetPointerV, NULL);
            }
        }
        else if (AlignmentPointSetAction[LOAD_DATABASE].s == ISS_ON)
        {
            LoadDatabase(pTelescope->getDeviceName());
            AlignmentPointSetSize.value = SyncPoints.size();
            //  Update client display
            IDSetNumber(&AlignmentPointSetSizeV, NULL);
        }
        else if (AlignmentPointSetAction[SAVE_DATABASE].s == ISS_ON)
        {
            SaveDatabase(pTelescope->getDeviceName());
        }

        //  Update client display
        IDSetSwitch(&AlignmentPointSetCommitV, NULL);
    }
}

void INDI::AlignmentSubsystemDriver::ProcessAlignmentBlobProperties(Telescope* pTelescope, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
    DEBUGFDEVICE(pTelescope->getDeviceName(), INDI::Logger::DBG_DEBUG, "ProcessAlignmentBlobProperties - name(%s)", name);
}

void INDI::AlignmentSubsystemDriver::SaveAlignmentConfigProperties(FILE *fp)
{
    IUSaveConfigText(fp, &AlignmentSubsystemCurrentMathPluginV);
}

bool INDI::AlignmentSubsystemDriver::LoadDatabase(const char* DeviceName)
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

    SyncPoints.clear();

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
        SyncPoints.push_back(CurrentValues);
    }

    fclose(fp);
    delXMLEle(FileRoot);
    delXMLEle(EntryRoot);
    delXMLEle(Element);
    delLilXML(Parser);

    return true;

}

bool INDI::AlignmentSubsystemDriver::SaveDatabase(const char* DeviceName)
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

    for (SyncPointsType::const_iterator Itr = SyncPoints.begin(); Itr != SyncPoints.end(); Itr++)
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

