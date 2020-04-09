/******************************************************************************************\
*                                                                                          *
*   OpenRGB.cpp                                                                            *
*                                                                                          *
*       Functions for communicating with RGBController API devices on Windows and Linux    *
*                                                                                          *
\******************************************************************************************/

#include "RGBController.h"
#include "i2c_smbus.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "i2c_smbus_linux.h"
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>


std::vector<i2c_smbus_interface*> busses;
std::vector<RGBController*> rgb_controllers;


/******************************************************************************************\
*                                                                                          *
*   DetectI2CBusses (Linux)                                                                *
*                                                                                          *
*       Detects available SMBUS adapters and enumerates i2c_smbus_interface objects for    *
*       them                                                                               *
*                                                                                          *
\******************************************************************************************/

void DetectI2CBusses()
{
    i2c_smbus_linux *       bus;
    char                    device_string[1024];
    DIR *                   dir;
    char                    driver_path[512];
    struct dirent *         ent;
    int                     test_fd;

    // Start looking for I2C adapters in /sys/bus/i2c/devices/
    strcpy(driver_path, "/sys/bus/i2c/devices/");
    dir = opendir(driver_path);

    if(dir == NULL)
    {
        return;
    }

    // Loop through all entries in i2c-adapter list
    ent = readdir(dir);
    while(ent != NULL)
    {
        if(ent->d_type == DT_DIR || ent->d_type == DT_LNK)
        {
            if(strncmp(ent->d_name, "i2c-", 4) == 0)
            {
                strcpy(device_string, driver_path);
                strcat(device_string, ent->d_name);
                strcat(device_string, "/name");
                test_fd = open(device_string, O_RDONLY);

                if(test_fd)
                {
                    read(test_fd, device_string, sizeof(device_string));
                    close(test_fd);
                    
                    bus = new i2c_smbus_linux();
                    strcpy(bus->device_name, device_string);

                    strcpy(device_string, "/dev/");
                    strcat(device_string, ent->d_name);
                    test_fd = open(device_string, O_RDWR);

                    if (test_fd < 0)
                    {
                        delete bus;
                        ent = readdir(dir);
                        continue;
                    }

                    bus->handle = test_fd;
                    busses.push_back(bus);
                }
            }
        }
        ent = readdir(dir);
    }

}   /* DetectI2CBusses() */


void DetectAuraSMBusControllers(std::vector<i2c_smbus_interface*> &busses, std::vector<RGBController*> &rgb_controllers);
/******************************************************************************************\
*                                                                                          *
*   DetectRGBConrollers                                                                    *
*                                                                                          *
*       Detect and populate RGB Controllers vector                                         *
*                                                                                          *
\******************************************************************************************/
#include "RGBController_Dummy.h"
void DetectRGBControllers(void)
{
    DetectI2CBusses();

    DetectAuraSMBusControllers(busses, rgb_controllers);

}   /* DetectRGBControllers() */
