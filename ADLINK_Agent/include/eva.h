#ifndef __EVA_H__
#define __EVA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h> //PATH_MAX from here

extern struct exInfo updPkgQueueInfo;

enum updComponent{
    BIOS_UPDATE,
    BMC_UPDATE,
    ML_UPDATE,
    AI_MODEL_UPDATE,
    AI_PIPELINE_UPDATE,
    CUSTOMER_RESERVE_1_UPDATE,
    CUSTOMER_RESERVE_2_UPDATE,
    CUSTOMER_RESERVE_3_UPDATE,
    CUSTOMER_RESERVE_4_UPDATE,
    CUSTOMER_RESERVE_5_UPDATE,
    CUSTOMER_RESERVE_6_UPDATE,
    CUSTOMER_RESERVE_7_UPDATE,
    CUSTOMER_RESERVE_8_UPDATE,
    CUSTOMER_RESERVE_9_UPDATE,
    CUSTOMER_RESERVE_0_UPDATE,
};

enum keyIdentifier{
    ADLINK,
    ID_RESERVE_1,
    ID_RESERVE_2,
    ID_RESERVE_3,
    ID_RESERVE_4,
    ID_RESERVE_5,
    ID_RESERVE_6,
    ID_RESERVE_7,
    ID_RESERVE_8,
    ID_RESERVE_9,
};

struct certInfo{
    int ValidityPeriod;
    char *CountryName;
    char *State;
    char *LocalityName;
    char *OrganizationName;
    char *OrganizationalUnitName;
    char *CommonName;
    char *Email;
};

#define MAX_MODEL_LIST  255
#define MAX_MODELFILE_LIST  100

#define UPD_PKG_SUCCESS     0
#define UPD_PKG_FILE_NOT_FOUND  -1
#define UPD_PKG_UNPACK_FAIL  -2
#define UPD_PKG_FILE_INVALID  -3
#define UPD_PKG_CERT_INVALID  -4
#define UPD_PKG_SIGNATURE_INVALID  -5

struct modelList {
    char inference[50];
    char modelName[50];
    char precision[50];
    char version[50];
    char location[37];
};

struct modelFilesList {
    char location[37];
    char fileName[1024];
    char filechksum[33];
};

//***Initialize***
/* Initial EVA
Parameters : 
    N/A
Output: 
    0¡G Initialize success
    -1 : Initialize failure
*/
int EVAInit(void);

int testFuntionC();

/* Deinitial EVA
Parameters : 
    N/A
Output: 
    N/A
*/
void EVAdeinit(void);


//***KeyManagementTool***
/* create key pair
Parameters : 
    char *keyFileName: Private & Public key main file name string(include path)
Output: 
    0¡G Create key pair success
    -1 : Create key pair failure
*/
int createKeyPair(char *keyFileName);

/* create update package certificate
Parameters : 
    char *privKey: Private key file name string(include path)
    char *certName: Certificate file name string(include path)
    struct certInfo: Certificate content {
        int ValidityPeriod: Certificate validity period(days)
        char *countryName: Country name string(abbreviation)
        char *State: State or Province Name string(full name)
        char *LocalityName: Locality Name string(eg, city)
        char *OrganizationName: Organization Name string(eg, company)
        char *OrganizationalUnitName: Organization Name string(eg, company)
        char *CommonName: Common Name string(e.g. server FQDN or YOUR name)
        char *email: Email Address string
    }
Output: 
    0¡G Create certificate success
    -1 : Create certificate failure
*/
int createCertificate(char *privKey,char *certName,struct certInfo *cerInfomation);

/* create new update package certificate with old certificate
Parameters : 
    char *origPrivKey: Original private key file name string(include path)
    char *oldCertName: Old certificate file name string(include path)
    char *newCertName: New certificate file name string(include path)
    int ValidityPeriod: Certificate validity period(days)
Output: 
    0¡G Renew certificate success
    -1 : Renew certificate failure
*/
int createCertificateWithOldCert(char *origPrivKey,char *oldCertName, char *newCertName,int ValidityPeriod);


//***CertificateStore***
/* install update package certificate
Parameters : 
    char *certificateFile: Certificate file name(include path)
    char *pubKeyFile: Public key file name(include path)
Output: 
    0¡G Certificate install success
    -1 : Certificate install failure
*/
int installPkgCertificate(char *certificateFile, char *pubKeyFile);

/* verify update package certificate
Parameters : 
    char *privKeyFile:Private key file name(include path)
Output: 
    0¡G Certificate uninstall success
    -1 : Certificate uninstall failure
*/
int uninstallPkgCertificate(char *privKeyFile);

/* verify update package certificate
Parameters : 
	certificateFile: certificate file name(include file path)
Output: 
    0¡G Is valid certificate
    -1 : Invalid certificate
*/
int isValidCertificate(char *certificateFile);

/* verify update package signature
Parameters : 
	certFile: certificate file name(include file path)
	updPackage: update package file name (include file path)
	signatureFile: image signature file(include file path),file extension must hash function name. Ex: xxxxxxxx.md5/xxxxxxxx.sha256/xxxxxxx.sha512....
Output: 
    0: Update package is valid
    -1 : Update package is invalid
*/
int verifyPackage(char* certFile, char* updPackage, char* signatureFile);

//***IdentityService***
/* validate HW
Parameters : 
    int identifier: Identification enum(e.g. ADLINK)
Output: 
    0¡G Is valid hardware
    -1 : Invalid hardware
*/

__declspec(dllimport) int isValidHW(enum keyIdentifier identifier);

int addUpdImgToPkg(char *pkgFile, char *signedUpdImg, int compName, int verCheck);

/* delete an update image from update package
Parameters : 
    char *pkgFile: Update package file name(include path)
    char *signedUpdImg: Signed update image file name(include path)
Output: 
    0¡G Delete signed update image from update package success
    -1 : Delete signed update image from update package failure
*/
int delUpdImgFromPkg(char *pkgFile, char *signedUpdImg);

/* list update image from update package
Parameters : 
    char *pkgFile: Update package file name(include path)
    char *imgList[]: Store signed image list buffer
    int *listCount: return update image files count
Output: 
    0¡G List all signed update image from update package success
    -1 : List all signed update image from update package failure
*/
int listUpdImgInPkg(char *pkgFile, char *imgList[], int *listCount);

/* Enable/Disable transaction mode
Parameters : 
    char *pkgFile: Update package file name(include path)
    int transactionFlag: set 1 means enable transaction mode, set 0 is disable transaction mode
Output: 
    0¡G set transaction flag success
    -1 : set transaction flag failure
*/
int enablePkgTransaction(char *pkgFile, int transactionFlag);

/* Complete sign update package
Parameters : 
    char *pkgFile: Update package file name(include path)
    char *privKey: Private key file name for update package sign(include path)
    char *certFile: Certificate file name for update package verify(include path)
    char *signedPkgFile: Output signed package file name(include path)
Output: 
    0¡G Sign update package success(Signed package generate in current directory and name is "[pkgName].signed.zip")
    -1 : Sign update package failure
*/
int completeSignUpdPkg(char *pkgFile, char *privKey,char *certFile, char *signedPkgFile);

/* Complete no sign update package
Parameters : 
    char *pkgFile: Update package file name(include path)
    char *outNoSignPkgFile: Output signed package file name(include path)
Output: 
    0¡G Complete no sign update package success(Signed package generate in current directory and name is "[pkgName].signed.zip")
    -1 : Complete no sign update package failure
*/
int completeNoSignUpdPkg(char *pkgFile, char *outNoSignPkgFile);

//***UpdatePackageModule***
/* get package update progress
Parameters : 
    char *signedPkgFile: Signed update package name string
Output: 
    0~100: progress percentage
    -1: Not support
*/
int getPkgUpdProgress(char *signedPkgFile);

/* execute package update
Parameters : 
    char *signedPkgFile: Signed update package name string
    int *errCode: Error code, what error happen will set indicate bit value to 1
Output: 
    0 : call updPackage success
    -1 : Not found update package
    -2 : unpack update package fail
    -3 : update package not valid
    -4 : update package include an invalid certificate
    -5 : update package signature invalid
*/
int updPackage(char *signedPkgFile,int *err);

/* get package update status
Parameters : 
    char *signedPkgFile: Signed update package name string
Output: 
    0 : Signed package update process not start
    1 : Signed package update in progress
    2 : Signed package update complete
    999 : update task canceled
    -1 : Get package update status failure
*/
int getUpdPkgStatus(char * signedPkgFile);

/* ckear package update status
Parameters : 
    char *signedPkgFile: Signed update package name string
Output: 
    0 : Signed package update process not start
    -1 : Clear old status failure
*/
int clearUpdPkgStatus(char * signedPkgFile);

/*
int compName: Component name enum (e.g. "AI_MODEL_UPDATE")
char *imgVersion: A string buffer to return image version
*/
int getCurrImgVerByName(int compName, char *imgVersion);

/* cancel package update
Parameters : 
    char *signedPkgFile: Signed update package name string
Output: 
    0 : Signed package update process canceled
    -1 : Package update process cancel failure
*/
int cancelPkgUpdate(char * signedPkgFile);

//***Cryptography helper functions***
/* Sign data
Parameters : 
    char *privKey: Private key file name string(include path)
    unsigned char *srcData: Need to sign bytes data
    int srcDataLen: sign bytes data length
    unsigned char *signatureData: Signature bytes data
    int *signatureDataLen: Signature bytes data length
Output: 
    0¡GSign data success
    -1 : Sign data failure
*/
int signData(char *privKey, char *srcData, int srcDataLen, char *signatureData, int *signatureDataLen);

/* Verify data
Parameters : 
    char *cerFile: Certificate file name string(include path)
    unsigned char *srcData: Need to sign bytes data
    int srcDataLen: sign bytes data length
    unsigned char *signatureData: Signature bytes data
    int *signatureDataLen: Signature bytes data length
Output: 
    0¡GValid data
    -1 : Invalid data
*/
int verifyData(char *cerFile, char *srcData, int srcDataLen, char *signatureData, int *signatureDataLen);

/* register encrypt callback function
Parameters : 
    void *funcPtr: Encrypt callback function pointer
Output: 
    N/A
*/
void regEncryptCallback(int (*funcPtr)());

/* register decrypt callback function
Parameters : 
    void *funcPtr: Decrypt callback function pointer
Output: 
    N/A
*/
void regDecryptCallback(int (*funcPtr)());

/* register AES key for ADLINK decrypt callback function
Parameters : 
    unsigned char *cryptKey: Crypt key value
    int keyLen: AES key length
Output: 
    N/A
*/
void regCryptokey(unsigned char *cryptKey, int keyLen);

/* Encrypt data
Parameters : 
    char *modelFile: Original model file
    char *enDataFile: Encrypted model file(include path)
Output: 
    0¡GEncrypt data success
    -1 : Encrypt data failure
*/
int encryptData(char *srcDataFile, char *enDataFile);

/* Decrypt data
Parameters : 
    char *enDataFile: Encrypted data file
    char *deDataFile: Decrypted bytes data
Output: 
    0¡GDecrypt data success
    -1 : Decrypt data failure
*/
int decryptData(char *enDataFile, char *deDataFile);

/* get Model files list frm model zoo
Parameters : 
    char *inferenceEngine: Inference engine name
    char *modelName: AI model name
    char *precision: Floating point precision
    char *version: Model version
    struct modelFilesList *modelZooFileList[]: Model file list
Output: 
    >0¡GModel file list count
    -1 : Get model file list failure
*/
int listModelFiles(char *inferenceEngine, char *modelName, char *precision, char *version, struct modelFilesList modelZooFilesList[],int *listCount);

/* get Model files list frm model zoo
Parameters : 
    char *inferenceEngine: Inference engine name
    char *modelName: AI model name
    char *precision: Floating point precision
    char *version: Model version
    struct modelList *modelZooList[]: Model list
Output: 
    >0¡GModel file list count
    -1 : Get model file list failure
*/
int listModels(char *inferenceEngine, char *modelName, char *precision, char *version, struct modelList modelZooList[],int *listCount);

/* remove Model files from model zoo
Parameters : 
    char *inferenceEngine: Inference engine name
    char *modelName: AI model name
    char *precision: Floating point precision
    char *version: Model version
Output: 
    0 : Remove model success    
    -1 : Remove model failure
*/
int removeModel(char *inferenceEngine, char *modelName, char *precision, char *version);

/* get Model file location
Parameters : 
    char *inferenceEngine: Inference engine name
    char *modelName: AI model name
    char *precision: Floating point precision
    char *version: Model version
Output: 
    NULL : Not get model file
    char *: Model file path string
*/
char *getModelFilePath(char *inferenceEngine, char *modelName, char *precision, char *version);

/* Create Model update image file
Parameters : 
    char *modelIdxSchemaFile: Model zoo repository schema index
    char *outputPackUpdImg: Output packed model update image file name(include path)
Output: 
    0¡G Create model update image success
    -1 : Createmodel update image failure
*/
int createModelUpdImg(char *modelIdxSchemaFile, char *outputPackUpdImg);

/* Add model or meta file into update image file
Parameters : 
    char *modelUpdFile: Model/Meta file name(include path)
    char *outputPackUpdImg: Output packed model update image file name(include path)
Output: 
    0¡G Add model update image success
    -1 : Add model update image failure
*/
int addFilesToModelUpdImg(char *modelUpdFile, char *outputPackUpdImg);

/* Delete model or meta file from update image file
Parameters : 
    char *modelUpdFile: Model/Meta file name(include path)
    char *outputPackUpdImg: Output packed model update image file name(include path)
Output: 
    0¡G Delete model update image success
    -1 : Delete model update image failure
*/
int delFilesFromModelUpdImg(char *modelUpdFile, char *outputPackUpdImg);

/* Check model update image file is complete
Parameters : 
    char *modelPackFile: Model pack file name(include path)
    char *outputPackUpdImg: Output packed model update image file name(include path)
Output: 
    0¡G Model update image pack complete
    -1 : Model update image pack is not complete
*/
int completeModelUpdImg(char *modelPackFile, char *outputPackUpdImg);

#ifdef __cplusplus
}
#endif

#endif