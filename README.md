# EVA-OTA-Sample

## Introduction 
 
This reference code uses ADLINK managebility library for efficiently deploying AI models onto the ADLINK edge platform.
Examples are provided for OTA procedure. See the API reference section for detailed reference documentation of the C libraries. <h2>
  
## Dependencies 
* ADLINK managebility runtime library 4.0.3 or newer
* OpenSSL
  ```bash
    sudo apt-get install libssl-dev
  ```
* Json-c
  ```bash
    sudo apt install libjson-c-dev
  ```
<h2>

## Windows Installation
   `To be determined `

## Ubuntu Installation
* Install ADLINK Managebility runtime library 4.0.X or newer version
* Install ADLINK EVA runtime library 4.X or newer version
  
## OTA Download Agent link
Please downlad the excutable file and copy the `otaAgent` to the path under `/opt/adlink/eva/bin`
* Ubuntu X86
* ARM (aarch64)
 
## EVA pipeline module link
Please downlad the shared library copy the `libpipelineUpdComp.so` to the path under `/usr/lib/OTA`
* Ubuntu X86
* ARM (aarch64)
  
## Compiling
  Sample need a recent version of Meson installed, please see as following.
    http://mesonbuild.com/Getting-meson.html
  
### Windows
  `To be determined `

### Ubuntu (amd64/aarch64)
Compiling the reference code by meson script
```bash
git-clone https://github.com/IST-EVA-Support/EVA-OTA-Sample.git
cd ADLINK_Agent
meson build
ninja -C build
```
Creating the repository under `/data/carota/`
cd ADLINK_Agent
run the `AgentScript.sh` 
 
Running the ADLINK OTA agent

 ```c
 // Running the slinet upadate
./AdlinkAgent -s
```
   
<h2>

## Sample description and notes
Please make sure the ADLINK managebility and EVA library have been installation successfully.
>> It is because that `otaAgent` need to get the `upgrade.result ` info to report,
we suggest that `otaAgent` will be start up by `Adlink Agent`
when device bootup.  
 
>> Check the ADLINK device information.
Please run the ADLINK_Agent command 
`./AdlinkAgent -i`
  The ADLINK egde device will show device informarion. The information will be used for update procedure.
  
>> The ADLINK edge device need to register the OTA project token. Please contact the service provider.
It is recommended to write the project token into NVRAM. This method can avoid the
change of update leads to the loss of project token information. Once this information is lost, it will cause 
Agent behaves unexpectedly. <h2>


## See also
 
 * EVA OTA protal and scenario
 * EVA Managebility SDK
 

