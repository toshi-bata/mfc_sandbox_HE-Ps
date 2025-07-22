# HOOPS Visualize - Exchange & Parasolid integration samples
This sample demonstrates how to integrate HOOPS Exchange and Siemens Parasolid into HOOPS Visualize HPS using sprockets.<br>
This sample project was developed for the following Tech Soft 3D Forum article:<br>
https://forum.techsoft3d.com/t/hoops-visualize-exchange-parasolid-integration-samples/2004

## Environment
### SDKs Used (Version)
* HOOPS Visualize HPS (2025.5.0)
* HOOPS Exchange (2025.5.0)
* Siemens Parasolid (35.1)

### Tested server OS
* Windows 11

## Projects
This sample includes two Visual Studio 2019 projects:
* hps_mfc_sandbox_HEPs_v142<br>
Uses the ExchangeParasolid sprocket.
* hps_mfc_sandbox_HE_PsOp_v142<br>
Uses the Exchnage sprocket and the Parasolid connector of Exchange.

### Build
1. Unzip the HOOPS Visualize HPS, Exchange and Parasolid SDKs 
2. Locate a valid `hoops_license.h` in `includ/` folders of both HOOPS Visualize and Exchange 
3. Edit `_VS2019_HEPs.bat` accoording to your environment, and launch `hps_mfc_sandbox_HEPs_v142.sln` using the bat file
4. Edit `_VS2019_HEHP_PsOp.bat` accoording to your environment, and launch `hps_mfc_sandbox_HE_PsOp_v142.sln` using the bat file
5. Build the solutions

> **Note:**  
> If you change the HOOPS Visualize version, be sure to replace the `properties.h` file from the new SDK's `\samples\mfc_sandbox\` directory.
