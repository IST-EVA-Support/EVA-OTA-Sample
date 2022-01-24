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
* Install ADLINK managebility runtime library 4.0.3 or newer version
* Install ADLINK EVA runtime library 4.X or newer version
  
## OTA Agent Download link
Please copy the `otaAgent` to `/opt/adlink/eva/bin`
* Ubuntu X86
* ARM (aarch64)
  
## Compiling
  need a recent version of Meson installed, see
    http://mesonbuild.com/Getting-meson.html
  
### Windows
  `To be determined `

### Ubuntu (amd64/aarch64)
Compiling the reference code by meson script
```bash
git-clone 
cd ADLINK_Agent
meson build
ninja -C build
```
Creating the `da.state` file  under /data/carota/
cd ADLINK_Agent
run the `AgentScript.sh` 
  
<h2>

## Example description and notes
Please make sure the ADLINK managebility and EVA library have been installation successfully.
>> Cause Carota Agent need to get the `upgrade.result ` info to report,
we suggest that Carota Agent will be brought up by Adlink Agent
when device bootup.  
>> Check the ADLINK device information
Run the ADLINK_Agent command 
`./AdlinkAgent -i`
  The ADLINK egde device will show device informarion. The information will be used for update procedure.
  
  
## See also




  
  

 
  
