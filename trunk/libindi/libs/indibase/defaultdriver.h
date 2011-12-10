#ifndef INDIDEFAULTDRIVER_H
#define INDIDEFAULTDRIVER_H

#include "basedriver.h"
#include "indidriver.h"

extern const char *COMMUNICATION_TAB;
extern const char *MAIN_CONTROL_TAB;
extern const char *MOTION_TAB;
extern const char *DATETIME_TAB;
extern const char *SITE_TAB;
extern const char *OPTIONS_TAB;

/**
 * \class INDI::DefaultDriver
   \brief Class to provide extended functionary for drivers in addition
to the functionality provided by INDI::BaseDriver. This class should \e only be subclassed by
drivers directly as it is linked with main(). Virtual drivers cannot employ INDI::DefaultDriver.

   INDI::DefaultDriver provides capability to add Debug, Simulation, and Configuration controls. These controls (switches) are
   defined to the client. Configuration options permit saving and loading of AS-IS property values.

\see <a href='tutorial__four_8h_source.html'>Tutorial Four</a>
\author Jasem Mutlaq
 */
class INDI::DefaultDriver : public INDI::BaseDriver
{
public:
    DefaultDriver();
    virtual ~DefaultDriver() {}

    /** \brief Add Debug, Simulation, and Configuration options to the driver */
    void addAuxControls();

    /** \brief Add Debug control to the driver */
    void addDebugControl();

    /** \brief Add Simulation control to the driver */
    void addSimulationControl();

    /** \brief Add Configuration control to the driver */
    void addConfigurationControl();

    /** \brief Set all properties to IDLE state */
    void resetProperties();

    /** \brief Define number vector to client & register it. Alternatively, IDDefNumber can be used but the property will not
               get registered and the driver will not be able to save configuration files.
         \param nvp The number vector property to be defined
    */
    void defineNumber(INumberVectorProperty *nvp);

    /** \brief Define text vector to client & register it. Alternatively, IDDefText can be used but the property will not
               get registered and the driver will not be able to save configuration files.
         \param tvp The text vector property to be defined
    */
    void defineText(ITextVectorProperty *tvp);

    /** \brief Define switch vector to client & register it. Alternatively, IDDefswitch can be used but the property will not
               get registered and the driver will not be able to save configuration files.
         \param svp The switch vector property to be defined
    */
    void defineSwitch(ISwitchVectorProperty *svp);

    /** \brief Define light vector to client & register it. Alternatively, IDDeflight can be used but the property will not
               get registered and the driver will not be able to save configuration files.
         \param lvp The light vector property to be defined
    */
    void defineLight(ILightVectorProperty *lvp);

    /** \brief Define BLOB vector to client & register it. Alternatively, IDDefBLOB can be used but the property will not
               get registered and the driver will not be able to save configuration files.
         \param bvp The BLOB vector property to be defined
    */
    void defineBLOB(IBLOBVectorProperty *bvp);


    /** \brief Delete a property and unregister it. It will also be deleted from all clients.
        \param propertyName name of property to be deleted.
    */
    virtual bool deleteProperty(const char *propertyName);

    /** \brief Connect or Disconnect a device.
      \param status If true, the driver will attempt to connect to the device (CONNECT=ON). If false, it will attempt
to disconnect the device.
      \param msg A message to be sent along with connect/disconnect command.
    */
    virtual void setConnected(bool status, IPState state=IPS_OK, const char *msg = NULL);

    int SetTimer(int);
    void RemoveTimer(int);
    virtual void TimerHit();

protected:

    /** \brief define the driver's properties to the client.
      \param dev name of the device
      \note This function is called by the INDI framework, do not call it directly.
    */
    virtual void ISGetProperties (const char *dev);

    /** \brief Process the client newSwitch command
      \note This function is called by the INDI framework, do not call it directly.
      \returns True if any property was successfully processed, false otherwise.
    */
    virtual bool ISNewSwitch (const char *dev, const char *name, ISState *states, char *names[], int n);

    /** \brief Process the client newNumber command
      \note This function is called by the INDI framework, do not call it directly.
      \returns True if any property was successfully processed, false otherwise.
    */
    virtual bool ISNewNumber (const char *dev, const char *name, double values[], char *names[], int n) {return false;}

    /** \brief Process the client newSwitch command
      \note This function is called by the INDI framework, do not call it directly.
      \returns True if any property was successfully processed, false otherwise.
    */
    virtual bool ISNewText (const char *dev, const char *name, char *texts[], char *names[], int n) {return false;}

    // Configuration

    /** \brief Load the last saved configuration file
        \return True if successful, false otherwise.
    */
    bool loadConfig();

    /** \brief Save the current properties in a configuration file
        \return True if successful, false otherwise.
    */
    bool saveConfig();

    /** \brief Load the default configuration file
        \return True if successful, false otherwise.
    */
    bool loadDefaultConfig();

    // Simulatin & Debug

    /** \brief Toggle driver debug status

      A driver can be more verbose if Debug option is enabled by the client.

      \param enable If true, the Debug option is set to ON.
    */
    void setDebug(bool enable);

    /** \brief Toggle driver simulation status

      A driver can run in simulation mode if Simulation option is enabled by the client.

      \param enable If true, the Simulation option is set to ON.
    */
    void setSimulation(bool enable);

    /** \return True if Debug is on, False otherwise. */
    bool isDebug();

    /** \return True if Simulation is on, False otherwise. */
    bool isSimulation();

    /** \brief Initilize properties initial state and value. The child class must implement this function.
        \return True if initilization is successful, false otherwise.
    */
    virtual bool initProperties();

    /** \brief updateProperties is called whenever there is a change in the CONNECTION status of the driver.
        This will enable the driver to react to changes of switching ON/OFF a device. For example, a driver
        may only define a set of properties after a device is connected, but not before.
        \return True if update is successful, false otherwise.
    */
    virtual bool updateProperties();

    /** \brief Connect to a device. Child classes must implement this function and perform the connection
        routine in the function.
        \return True if connection to device is successful, false otherwise.
    */
    virtual bool Connect()=0;

    /** \brief Disconnect from a device. Child classes must implement this function and perform the disconnection
        routine in the function.
        \return True if disconnection from a device is successful, false otherwise.
    */
    virtual bool Disconnect()=0;

    /** \return Default name of the device. */
    virtual const char *getDefaultName()=0;



private:

    bool pDebug;
    bool pSimulation;

    ISwitch DebugS[2];
    ISwitch SimulationS[2];
    ISwitch ConfigProcessS[3];

    ISwitchVectorProperty *DebugSP;
    ISwitchVectorProperty *SimulationSP;
    ISwitchVectorProperty *ConfigProcessSP;

    //  These are the properties we define, that are generic to pretty much all devices
    //  They are public to make them available to all dervied classes and thier children
    ISwitchVectorProperty *ConnectionSP;
    ISwitch ConnectionS[2];


};

#endif // INDIDEFAULTDRIVER_H
