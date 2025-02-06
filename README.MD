# HOOPS Visualize - Exchange & Parasolid integration samples
This sample illustrates how to implement HOOPS Exchange and Siemens Parasolid into HOOPS Visualize HPS sprockets.<br>
This sample project was developed for the following article of Tech Soft 3D Forum:<br>
https://forum.techsoft3d.com/t/hoops-visualize-exchange-parasolid-integration-samples/2004

## Environment
### SDKs in use (version)
* HOOPS Visualize HPS (2024)
* HOOPS Exchange (2024)
* Siemens Parasolid (34.1.153)

### Tested server OS
* Windows 10

## Projects
This sample contains two Visual Studio 2019 projects:
* hps_mfc_sandbox_HEPs_v142 (uses ExchangeParasolid sprocket)
* hps_mfc_sandbox_HE_PsOp_v142 (Uses Exchnage sprocket and Parasolid connector of Exchange)

### Build
1. Unzip HOOPS Visualize HPS, Exchange and Parasolid SDKs 
2. Locate a valid hoops_license.h in `includ/` folders of HOOPS Visualize and Exchange 
3. Edit _VS2019_HEPs.bat accoording to your environment and open hps_mfc_sandbox_HEPs_v142.sln using the bat file
4. Edit _VS2019_HEHP_PsOp.bat accoording to your environment and open hps_mfc_sandbox_HE_PsOp_v142.sln using the bat file
5. Build the solutions
