#include <stdio.h>
#include <stdint.h>
#include <deviceInfo.h>
#include "include/json-c/json.h"
#include <string.h>
#include "AdlinkAgent.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <sys/file.h>
//#include <sys/time.h> 
#include <sys/stat.h>
#include <signal.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include "getopt.h"
#elif __linux__
#include <pthread.h>
#include <sys/file.h>
#include <sys/time.h> 
#include <unistd.h>
#endif
//#include <pthread.h>
#include <eva.h>
#include <errno.h>


volatile sig_atomic_t exitRequested = 0;
const char    *my_argv[64] = {"/opt/adlink/eva/bin/otaAgent" , "" , "" , NULL}; //Carota_Download\ Agent_toAdlink_v1.0.1 //Download\ Agent_Adlink
const char    *pipeline_argv[512] = {"/opt/adlink/eva/samples" , "python3 pipeline_app.py" , NULL , NULL};
int updateOnline = 0, silentInstall = 0, lockValue = 0, updateStatus = 0;
int runflag = 0 ;

#ifdef WIN32
HANDLE thread = NULL;
#else
static pid_t PID1;
#endif

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void SleepTime(int second)
{
  #ifdef WIN32
    Sleep(second * 1000); //milisecond
  #else
    sleep(second);
  #endif
}

int writeToJsonFile(char* filename, int CodeValue, char* MessageString)
{
  int ret = 0;
	json_object *json_obj = NULL;
  json_object *code_obj = NULL;
  json_object *msg_obj = NULL;

  int fileLockStatus = 0;
  
#ifdef WIN32
  BOOL fSuccess;
  // Create the file, open for both read and write.
  HANDLE hFile = CreateFile(TEXT("da.state"),               // file to open
                       GENERIC_READ | GENERIC_WRITE,          // open for reading
                       FILE_SHARE_READ | FILE_SHARE_WRITE,       // share for reading
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // normal file
                       NULL);                 // no attr. templatetr. template

  if (hFile == INVALID_HANDLE_VALUE) 
  { 
        printf("CreateFile fail.\n");
  }

  OVERLAPPED sOverlapped ={ 0 };
  //sOverlapped.Offset = 10 * 3;
  //sOverlapped.OffsetHigh = 0;

  fSuccess = LockFileEx(hFile,         // exclusive access, 
                          LOCKFILE_EXCLUSIVE_LOCK | 
                          LOCKFILE_FAIL_IMMEDIATELY,
                          0,             // reserved, must be zero
                          MAXDWORD,    // number of bytes to lock
                          MAXDWORD,
                          &sOverlapped); // contains the file offset
  if (!fSuccess) 
  {
       // Handle the error.
      printf ("LockFileEx failed (%d)\n", GetLastError());
  }
  else 
  {
      printf("LockFileEx by Agent succeeded\n");
      // Unlock the file.
      fSuccess = UnlockFileEx(hFile, 
                              0,             // reserved, must be zero
                              MAXDWORD,    // num. of bytes to unlock
                              MAXDWORD,
                              &sOverlapped); // contains the file offset
      if (!fSuccess) 
      {
        // Handle the error.
        printf ("UnlockFileEx failed (%d)\n", GetLastError());
      }
      else
      {
        printf("UnLockFileEx by Agent succeeded\n");
         //writeToJsonFile("da.state", 8, "Test info8");
      }
  }

  SleepTime(2);
  CloseHandle(hFile);

  json_obj = json_object_new_object();
  if (!json_obj)
  {
      printf("Cannot create object\n");
      ret = -1;
      goto error;
  }

  //new a code's string
  code_obj = json_object_new_int(CodeValue);
  if (!code_obj)
  {
      printf("Cannot create code int object for %s\n", CODE_STRING);
      ret = -1;
      goto error;
  }
  json_object_object_add(json_obj, CODE_STRING, code_obj);
  code_obj = NULL;

  //new a message's string
  msg_obj = json_object_new_string(MessageString);
  if (!msg_obj)
  {
      printf("Cannot create message string object for %s\n", MESSAGE_STRING);
      ret = -1;
      goto error;
  }
  json_object_object_add(json_obj, MESSAGE_STRING, msg_obj);
  msg_obj = NULL;

  //write the base object to write.json
  json_object_to_file(filename, json_obj); 

  error:
  json_object_put(json_obj);
  json_object_put(msg_obj);
  json_object_put(code_obj);

  return 0;

#else

  fileLockStatus=open(filename,O_WRONLY|O_CREAT, 0655);  //O_APPEND

  printf("fileLockStatus.(%d)\n", fileLockStatus);

  if(flock(fileLockStatus,LOCK_EX|LOCK_NB)==0)
  {
      printf("The file was locked by the ADLINK agent.\n");;
      //Write the json file and new a base object
      json_obj = json_object_new_object();
      if (!json_obj)
      {
        printf("Cannot create object\n");
        ret = -1;
        goto error;
      }

      //new a code's string
      code_obj = json_object_new_int(CodeValue);
      if (!code_obj)
      {
        printf("Cannot create code int object for %s\n", CODE_STRING);
        ret = -1;
        goto error;
      }
      json_object_object_add(json_obj, CODE_STRING, code_obj);
      code_obj = NULL;

      //new a message's string
      msg_obj = json_object_new_string(MessageString);
      if (!msg_obj)
      {
        printf("Cannot create message string object for %s\n", MESSAGE_STRING);
        ret = -1;
        goto error;
      }
      json_object_object_add(json_obj, MESSAGE_STRING, msg_obj);
      msg_obj = NULL;

      //write the base object to write.json
      json_object_to_file(filename, json_obj); 

      // unlock the file
      flock(fileLockStatus,LOCK_UN);

      error:
      json_object_put(json_obj);
      json_object_put(msg_obj);
      json_object_put(code_obj);

      return 0;
  }
  // else
  // {
  //     ret = -3; //File lock
  //     printf("The file was locked by other process.Can't write it.\n");
  // }

  close(fileLockStatus);

#endif
	return ret;
}


int readJsonFile(char *filename, char *Message)
{
  int ret = 0;
	json_object *read_obj = NULL;
  json_object *code_obj = NULL;
  json_object *msg_obj = NULL;

  //get json object from file
	read_obj = json_object_from_file(filename);
	if (!read_obj)
	{
		printf("Cannot open %s\n", filename);
		ret = -1;
		goto error;
	}


  int codevalue;
  {
    json_object * jCodeValue;
    if ( json_object_object_get_ex(read_obj, "code", & jCodeValue) ) {
        codevalue = json_object_get_int(jCodeValue);
    } else {
        codevalue = 1000;
    }
  }


  //get code int
	//code_obj = json_object_object_get(read_obj, CODE_STRING);
  /*code_obj = json_object_object_get_ex(read_obj, CODE_STRING, & code_obj);
	if (code_obj)
	{
		printf("Cannot get %s object\n", CODE_STRING);
		ret = -1;
		goto error;
	}
  printf("%s = %d\n", CODE_STRING, json_object_get_int(code_obj));*/

  //get message string
	msg_obj = json_object_object_get(read_obj, MESSAGE_STRING);
	if (!msg_obj)
	{
		printf("Cannot get %s object\n", MESSAGE_STRING);
		ret = -1;
		goto error;
	}
	//printf("%s = %s\n", MESSAGE_STRING, json_object_get_string(msg_obj));

  strncpy(Message, json_object_get_string(msg_obj), 100);

  ret = codevalue;

  error:
	json_object_put(read_obj);

	return ret;

}


int FileCheckValid(char* FileName, char* FileMode , int code , char* Message)
{
  int ret = 0;
  int fileLockStatus;

  //File check is valid or not
  FILE* fp = fopen(FileName, FileMode);
  if (fp) {
      // file exists
      printf("File %s exit!\n", FileName);
      fclose(fp);
      return 0;
  } else {
      // file doesn't exist then set the Json file
      if(fp!=NULL)
      { fclose(fp);}
       
    
      ret = writeToJsonFile(FileName, code, Message);  //"ADLINK agent message."

      if(ret!=0)
        printf("Write the json %s file fail=%d\n", FileName, ret);
      else {printf("Write the json %s file successfully=%d\n", FileName, ret);}
   
      return -1;
  }
}

int checkFileLockStatus()
{
  int ret = 0;
  int fd;
  int fileLockStatus = 0;

  //printf("checkFileLockStatus() File open.\n");
  fd=open(EXAMPLE_JSON,O_WRONLY|O_CREAT|O_APPEND, 0655);
  
  #ifdef WIN32
  //Do the windows
  #else
  if(flock(fd,LOCK_EX|LOCK_NB)==0)
  {
     //printf("The file was locked by the ADLINK agent.\n");
     //Writefuntion
     flock(fd,LOCK_UN);

     return 0;
  }
  else
  {
     return -1;
     //printf("the file was locked by other process.Can't write.\n");
  }
  
  //fileLockStatus  = flock(fp,LOCK_EX|LOCK_NB);
  #endif
}

void UpgrageTheModelFile()
{
    int cmd_opt=0,listCount=0,listFileCount=0,i,j,ret;
    char *modelFileLocation=NULL;
    char *modelFileLocationLast=NULL;
    char targetFileLocation[1024]={0};
    char modelInferenceEngine[1024]={0},modelName[1024]={0},modelPrecision[1024]={0},modelVersion[1024]={0};
    char command[1024]={0};
    struct modelList modelsFilterResult[256];
    struct modelFilesList modelFilesFilterResult[256];

    listModels(modelInferenceEngine,modelName,modelPrecision,modelVersion,modelsFilterResult,&listCount);

    for(i=0;i<listCount;i++){
        modelFileLocation = getModelFilePath(modelsFilterResult[i].inference,modelsFilterResult[i].modelName,modelsFilterResult[i].precision,modelsFilterResult[i].version);
        printf("%03d:\t%s,%s,%s,%s,%s\n",i+1,modelsFilterResult[i].inference,modelsFilterResult[i].modelName,modelsFilterResult[i].precision,modelsFilterResult[i].version,modelFileLocation);
               
    }
    //printf("======================================================================================================\n");
    printf("============================================listModelFiles=================================================\n");
    printf("listCount = %d\n", listCount);
    //listModelFiles(modelInferenceEngine,modelName,modelPrecision,modelVersion,modelFilesFilterResult,&listCount);
    //listModelFiles(modelsFilterResult[listCount-1].inference,modelsFilterResult[listCount-1].modelName,modelsFilterResult[listCount-1].precision,modelsFilterResult[listCount-1].version,modelFilesFilterResult,&listFileCount);
    //for(j=0;j<listCount;j++)
    j = listCount - 1;
    printf("%03d:\t%s,%s,%s,%s\n",i+1,modelsFilterResult[j].inference,modelsFilterResult[j].modelName,modelsFilterResult[j].precision,modelsFilterResult[j].version);
    {
        listModelFiles(modelsFilterResult[j].inference,modelsFilterResult[j].modelName,modelsFilterResult[j].precision,modelsFilterResult[j].version,modelFilesFilterResult,&listFileCount);
        printf("%03d:\t%s,%s,%s\n",1,modelFilesFilterResult[0].location,modelFilesFilterResult[0].fileName,modelFilesFilterResult[0].filechksum);
    }
    printf("listModelFiles inference= %s\n",modelFilesFilterResult[0].fileName);
    printf("Find the model file = %d\n", listFileCount);

    if(strcmp(modelsFilterResult[listCount-1].inference,"OpenVino") ==0)
    {
        modelFileLocationLast = getModelFilePath(modelsFilterResult[listCount-1].inference,modelsFilterResult[listCount-1].modelName,modelsFilterResult[listCount-1].precision,modelsFilterResult[listCount-1].version);
        strcat(modelFileLocationLast, "/");
        strcat(modelFileLocationLast, modelFilesFilterResult[0].fileName); 
        //printf("modelFileLocationLast= %s\n",modelFileLocationLast);

        strcpy(targetFileLocation, " /home/adlink/vino-model");    
        strcat(modelFileLocationLast, targetFileLocation);

        if(modelFileLocationLast!=NULL){
              sprintf(command,"cp %s",modelFileLocationLast);
              system(command);
              printf("Transfer the model file into %s", command);
        }else{ printf("Model file location is NULL string! Please check your package is normally.\n");}

        /*ret = CopyFile(modelFileLocationLast, "/home/adlink/vino-model");

        if(ret != 0)
        {printf("File copy error%s\n",modelFileLocationLast);}*/
    }
    else if(strcmp(modelsFilterResult[listCount-1].inference,"TensorRT") ==0)
    {
        modelFileLocationLast = getModelFilePath(modelsFilterResult[listCount-1].inference,modelsFilterResult[listCount-1].modelName,modelsFilterResult[listCount-1].precision,modelsFilterResult[listCount-1].version);
        strcat(modelFileLocationLast, "/");
        strcat(modelFileLocationLast, modelFilesFilterResult[0].fileName); 
        //printf("modelFileLocationLast= %s\n",modelFileLocationLast);

        strcpy(targetFileLocation, " /home/adlink/tensor-model");    
        strcat(modelFileLocationLast, targetFileLocation);

        if(modelFileLocationLast!=NULL){
              sprintf(command,"cp %s",modelFileLocationLast);
              system(command);
              printf("Transfer the model file into %s", command);
        }else{ printf("Model file location is NULL string! Please check your package is normally.\n");}
    }
    else
    {
      printf("Please check your model definition\n");
    }
  
}

void listModelFileList()
{
    int cmd_opt=0,listCount=0,i;
    char *modelFileLocation=NULL;
    char modelInferenceEngine[1024]={0},modelName[1024]={0},modelPrecision[1024]={0},modelVersion[1024]={0};
    struct modelList modelsFilterResult[256];

    listModels(modelInferenceEngine,modelName,modelPrecision,modelVersion,modelsFilterResult,&listCount);
    printf("Total model counts: %d\n",listCount);
    
    printf("=================================================================================================================================================================\n");
    for(i=0;i<listCount;i++){
        modelFileLocation = getModelFilePath(modelsFilterResult[i].inference,modelsFilterResult[i].modelName,modelsFilterResult[i].precision,modelsFilterResult[i].version);
        printf("%03d:\t%s,%s,%s,%s,%s\n",i+1,modelsFilterResult[i].inference,modelsFilterResult[i].modelName,modelsFilterResult[i].precision,modelsFilterResult[i].version,modelFileLocation);
               
    }
    printf("=================================================================================================================================================================\n");
}

int ReadThePipelineFile(char * filename)
{
    char buffer[2048]; //home/adlink/pipeconfig/evaconfig.txt

    printf("file path = %s\n", filename);
    FILE *file = fopen(filename, "r");
    // Checks if the file was opened successfully
    if (file == NULL)
    {
        fputs("Failed to open the file\n", stderr);
        return -1;
    }
    
    // fgets here reads an entire line or 99 characters (+1 for \0) at a time, whichever comes first
    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        //printf("Line read = %s\n", buffer);
        if (buffer != NULL){
            if(strlen(buffer) != 0)
            {
                runflag = 1;
                system(buffer);
            }
        }
    }
    runflag = 0;
    printf(BLU_BOLD"Freeing EVA pipeline , flag = %d\n"RESET, runflag);
    fclose(file);
    return 0;
}


int UpdatePackageOffline()
{

  
  char *aiUpdPkg = "/home/adlink/car/UpdPkg_v1.1.Sign.zip";
  char aiUpdPkgPath[1024] = "/home/adlink/car/package/";
  int ret = 0, errCode;
  char message[1024];
  char packageFie[1024] ={ 0 };
  char input[64];
  char szDestination[1024];


  printf("%s\n", "========================Update package process starting========================");
  printf(BLU_BOLD"Which file do you want to update?\n"RESET);
  system("ls /home/adlink/car/package");

  scanf("%s",input);
  strcat(szDestination, input);

  memset(packageFie, '\0', sizeof(packageFie));
  strcpy(packageFie, szDestination);
  printf("Update the package file path?%s\n", aiUpdPkgPath);
  printf("Update the package file?%s\n", packageFie);
  strcat(aiUpdPkgPath, packageFie);


  ret = EVAInit();

  if (ret < 0) {
        printf("Error: Initial fail!\n");
        return -1;
   }

  ret = updPackage(aiUpdPkgPath,&errCode);
  switch (ret) {
      case UPD_PKG_SUCCESS: //UPD_PKG_SUCCESS
                printf("Updating the EVA package successfully!\n");
                strcpy(message, "Updating the EVA package successfully!");
                break;
      case UPD_PKG_FILE_NOT_FOUND:
                printf("Package file is not found!(%d)\n",errCode);
                strcpy(message, "Package file is not found!");
                break;
      case UPD_PKG_UNPACK_FAIL:
                printf("UNpack package fail!(%d)\n",errCode);
                strcpy(message, "UNpack package fail!");
                break;
      case UPD_PKG_FILE_INVALID:
                printf("Package content invalid!(%d)\n",errCode);
                strcpy(message, "Package content invalid!");
                break;
      case UPD_PKG_CERT_INVALID:
                printf("Package include invalid certificate!(%d)\n",errCode);
                strcpy(message, "Package include invalid certificate!");
                break;
      case UPD_PKG_SIGNATURE_INVALID:
                printf("Package signature invalid!(%d)\n",errCode);
                strcpy(message, "Package signature invalid!");
             break;
    }
    EVAdeinit();
    printf("Message = %s, Error code = %d\n", message, ret);
    printf("%s\n", "========================Update package process finish========================");
    if(ret == 0)
    {
      listModelFileList(); 
      UpgrageTheModelFile();
      ReadThePipelineFile(EVA_CONFIG);
    }

    return ret;
    
}


int UpdatePackageProcess()
{
    char input[512];
    char message[1024];
    char *aiUpdPkg = "/data/carota/update.zip";
    enum State code;
    code = StartUpgade;
    int ret = 0, errCode;

    //pthread_mutex_lock(&mutex);

    printf("%s\n", "========================Update package process starting========================");
    writeToJsonFile(da_state, code, "Start upgrading equipement.");

    SleepTime(2);
    memset(message, 0, sizeof(message));
    
    if(silentInstall == 1)
    { strcpy(input, "yes"); } 
    else {
      question1:printf(BLU_BOLD"Would you like to upgrade the package? (yes/no)\n"RESET);
      scanf("%s", input);
    }
    
    //FileCheckValid(upgrade_result, "wb");
    if(strcmp(input, "yes") == 0) 
    {
        ret = EVAInit();

        if (ret < 0) {
            printf("Error: Initial fail!\n");
            return -1;
        }

        ret = updPackage(aiUpdPkg,&errCode);
        switch (ret) {
            case UPD_PKG_SUCCESS: //UPD_PKG_SUCCESS
                printf("Updating the EVA package successfully!\n");
                strcpy(message, "Updating the EVA package successfully!");
                break;
            case UPD_PKG_FILE_NOT_FOUND:
                printf("Package is not found!(%d)\n",errCode);
                strcpy(message, "Package is not found!");
                break;
            case UPD_PKG_UNPACK_FAIL:
                printf("UNpack package fail!(%d)\n",errCode);
                strcpy(message, "UNpack package fail!");
                break;
            case UPD_PKG_FILE_INVALID:
                printf("Package content invalid!(%d)\n",errCode);
                strcpy(message, "Package content invalid!");
                break;
            case UPD_PKG_CERT_INVALID:
                printf("Package include invalid certificate!(%d)\n",errCode);
                strcpy(message, "Package include invalid certificate!");
                break;
            case UPD_PKG_SIGNATURE_INVALID:
                printf("Package signature invalid!(%d)\n",errCode);
                strcpy(message, "Package signature invalid!");
                break;
        }
        EVAdeinit(); 

        SleepTime(2);

        printf("%s\n", "========================Updating package procedure is finish========================");

        if(ret == UPD_PKG_SUCCESS)
        {
            //listModelFileList(); 
            //TerminatePipeline();
            SleepTime(2);

            if(silentInstall == 1)
            { strcpy(input, "yes"); } 
            else {
              question:printf(BLU_BOLD"Would you like to upgrade the model(yes/no/exit)?\n"RESET);
              scanf("%s", input);
            }

            if(strcmp(input, "yes") == 0) 
            {
                //Clear the model file
                //deletePackageModel();
                
                //Upgrade the mode file
                UpgrageTheModelFile();

                code = UpgadeSuccess;
                FileCheckValid(upgrade_result, "wb", code, "Upgrade equipment successfully.");
                writeToJsonFile(upgrade_result, code, "Upgrade equipment successfully.");
                SleepTime(2);
                printf("%s\n", "========================Creating the pipleine command========================");
                //CreatePipeline();/*Start the EVA pipeline based on the gst-launch-1.0 or gstreamer application*/
            }
            else if(strcmp(input, "no") == 0)
            {
                code = UpgadeFail;
                FileCheckValid(upgrade_result, "wb", code, "Stopping upgrade equipment.");
                writeToJsonFile(upgrade_result, code, "Stopping upgrade equipment by Adlink agent.");

            }
            else if(strcmp(input, "exit") == 0)
            {
                exitRequested = 1;
                printf("Stopping the carota agent\n");
            }
            else goto question;
        }
        else 
        {
          printf(RED_BOLD"Upgrade the package/model file fail. Please contact your adminstrator! \n"RESET);
          printf(RED_BOLD"%s \n"RESET, message);
          code = UpgadeFail;
          FileCheckValid(upgrade_result, "wb", code, "Upgrade equipment fail!");
          strcat(message, ". Upgrade equipment fail!");
          writeToJsonFile(upgrade_result, code, message);
        }
      
    }
    else if(strcmp(input, "no") == 0) 
    {
       printf("Waitting for the upgrade proccess.\n");
       goto question1;
       
    } 
    //pthread_mutex_unlock(&mutex);

    return ret;

}

void deletePackageModel()
{
    int cmd_opt=0,listCount=0,retCode, i, j;
    char *modelFileLocation=NULL;
    char modelInferenceEngine[FILE_LENGHTH]={0},modelName[FILE_LENGHTH]={0},modelPrecision[FILE_LENGHTH]={0},modelVersion[FILE_LENGHTH]={0};
    struct modelList modelsFilterResult[256];
    char exeCmd[COMMAND_LENGHTH];
    char input[512]; 

    if(silentInstall == 1)
    { strcpy(input, "no"); }
    else{
      printf(BLU_BOLD"Do you want to remove the older package/model files (yes/no)?\n"RESET);
      scanf("%s", input);
    } 

    if(strcmp(input, "yes") == 0) 
    {
      listModels(modelInferenceEngine,modelName,modelPrecision,modelVersion,modelsFilterResult,&listCount);
      
      for(i=0;i<listCount;i++){
          printf("%03d:\t%s,%s,%s,%s\n",i+1,modelsFilterResult[i].inference,modelsFilterResult[i].modelName,modelsFilterResult[i].precision,modelsFilterResult[i].version);
          modelFileLocation = getModelFilePath(modelsFilterResult[i].inference,modelsFilterResult[i].modelName,modelsFilterResult[i].precision,modelsFilterResult[i].version);
          if(modelFileLocation!=NULL){
              sprintf(exeCmd,"rm -rf %s",modelFileLocation);
              system(exeCmd);
          }
          removeModel(modelsFilterResult[i].inference,modelsFilterResult[i].modelName,modelsFilterResult[i].precision,modelsFilterResult[i].version);    
      }

    }
    else if(strcmp(input, "no") == 0)printf("Keeping the model file list\n");
  
}


// void SignalHandler(int signumber)
// {
//     int retval;

//     if (signumber == SIGINT) {
        
//         wait(&retval);
//         printf("CATCH SIGINT PID=%d\n",getpid());
//         printf("Stopping the ADLINK Agent\n");
//         exitRequested = 1;
//     } 
// }

void SignalHandler(int signal)
{
    int retval;
    if (signal == SIGINT) {
        printf("CATCH SIGINT PID=%d\n",getpid());
        printf("Stopping the ADLINK Agent\n");
        exitRequested = 1;
    } else {
        // ...
    }
}

int CheckFileIsExit()
{
  /*The Download Agent will create the da.state*/
  int ret = 0;
  if(FileCheckValid(project_token, "wb", 0, "Create Project Token.")!=0)
  {
    printf("Create the project token file.=%d\n", ret);
    return 0;
  }
  else{ return -1; }
    
  //if(FileCheckValid(upgrade_result)!=0)
    //printf("Create the upgrade.result file.=%d\n", ret);

}

void CreateTheProjectToken()
{

    int value;
    char Message[100];
    char dest[100];
    char input[64];

    //Create or read the porject token file
    if(CheckFileIsExit() != 0)
    {
        printf("Create the project token.(10 digits)\n");
        scanf("%s", input);
        writeToJsonFile(project_token, 0, input);
    }
    else
    {
        writeToJsonFile(project_token, 0, project_token_num);
    }

    memset(dest, 0, sizeof(dest));
    memset(Message, 0, sizeof(Message));
    readJsonFile(project_token, Message);
    memcpy(dest, Message, sizeof(Message));

    strcpy(dest, Message);
    setProjToken(dest);
}

int CopyFile(const char *pathfrom, const char *pathto)
{
    FILE *fpi = NULL, *fpo = NULL;
    int  nByteRead = 0;
    char szBuffer[MAX_PATH];
    int bRet;
 
    if( (fpi = fopen( pathfrom, "rb" )) != NULL ) {
       if( (fpo = fopen( pathto, "wb" )) !=  NULL ) {
      while( (nByteRead = fread(szBuffer, sizeof(char), MAX_PATH, fpi))> 0 ) 
            fwrite(szBuffer, sizeof(char), nByteRead, fpo);   
      }
      else
         goto err_out;
   }
   else
      goto err_out;
 
   bRet = 0;
 
err_out:
   if(fpi)
      fclose(fpi);
   if(fpo)
   	  fclose(fpo);
   return bRet;
}

void TerminatePipeline()
{
    char command[COMMAND_LENGHTH];

    memset(command, '\0', sizeof(command));
    strcpy(command, "killall -2 gst-launch-1.0" );
    system(command);

    printf(YEL_BOLD"Stopping the EVA pipeline.\n"RESET);
 
}

#ifdef WIN32
#else
static int exec_prog(const char **argv , pid_t my_pid)
{
 
    int  status, timeout /* unused ifdef WAIT_FOR_COMPLETION */;

    if (-1 == execve(argv[0], (char **)argv , NULL)) {
        perror("child process execve failed [%m]");
    }

#ifdef WAIT_FOR_COMPLETION
    timeout = 1000;

    while (0 == waitpid(my_pid , &status , WNOHANG)) {
            if ( --timeout < 0 ) {
                    perror("timeout");
                    return -1;
            }
            sleep(1);
    }

    printf("%s WEXITSTATUS %d WIFEXITED %d [status %d]\n",
            argv[0], WEXITSTATUS(status), WIFEXITED(status), status);

    if (1 != WIFEXITED(status) || 0 != WEXITSTATUS(status)) {
            perror("%s failed, halt system");
            return -1;
    }

#endif
    return 0;
}
#endif

void CheckFilePath(char *filepath)
{
  struct stat fileState;
  int ret;
  char exeCmd[COMMAND_LENGHTH];

  ret = stat(filepath, &fileState);

  if(ret<0)
  {
    if(errno == ENOENT)
    {
      sprintf(exeCmd,"mkdir -m 777 %s",filepath);
      system(exeCmd);
    }
  }else{printf("Path is %s exit\n", filepath);}

}

void SetEVAEnviroment()
{
    char command[COMMAND_LENGHTH];
    //EVA_SOURCE_COMMAMD
    strcpy(command, EVA_SOURCE_COMMAMD);

    if (command != NULL){
      if(strlen(command) != 0){
        system(command);
      }
    }
}


void CreatePipeline()
{
    char command[COMMAND_LENGHTH];

    int cmd_opt=0,listCount=0,i ,ret=0 ;
    char *modelFileLocation=NULL;
    char modelInferenceEngine[1024]={0},modelName[1024]={0},modelPrecision[1024]={0},modelVersion[1024]={0};
    struct modelList modelsFilterResult[256];
    struct modelFilesList modelFilesFilterResult[256];
    char output[64];
    char modelfile[1024]={0};

    listModels(modelInferenceEngine,modelName,modelPrecision,modelVersion,modelsFilterResult,&listCount);
    //listModelFiles(modelInferenceEngine,modelName,modelPrecision,modelVersion,modelFilesFilterResult,&listCount);

    for(i=0;i<listCount;i++){
        modelFileLocation = getModelFilePath(modelsFilterResult[i].inference,modelsFilterResult[i].modelName,modelsFilterResult[i].precision,modelsFilterResult[i].version);
        printf("%03d:\t%s,%s,%s,%s,%s\n",i+1,modelsFilterResult[i].inference,modelsFilterResult[i].modelName,modelsFilterResult[i].precision,modelsFilterResult[i].version,modelFileLocation);
               
    }
    ////////////////////////////////
    //Example model information/////
    ////////////////////////////////
    //001:TensorRT_7.2,Yolo,FP32,1.0.1,/usr/local/lib/ADMODELZOO/2cf02def-d77b-4877-8d61-b0f6c340f55
    //002:OpenVino,GoogleNet,FP32,1.2.1,/usr/local/lib/ADMODELZOO/594c0b34-94dd-4df3-a4e0-c3acc0d4515

    if(strcmp(getDevModel(), "NEON-1000-MDX") == 0 ) //NEON-1000-MDX
    {
       printf("Running at NEON-1000-MDX.\n");
       printf("Model Name: %s\n", modelsFilterResult[listCount-1].modelName);
       
       if(strncmp(modelsFilterResult[listCount-1].modelName ,"GoogleNet", 9) == 0) //Classfication
       {
         printf("Running the (GoogleNet)classfication.\n");
         //strcpy(command, EXAMPLE_CLASSFICATION_COMMAND_MYRIAD);
       }  
       else if(strncmp(modelsFilterResult[listCount-1].modelName ,"Yolo", 4) == 0) //Detection
       {
         printf("Running the (Yolo) detection.\n");
         //strcpy(command, EXAMPLE_DETECTION_COMMAND_MYRIAD);
       }
       else if(strncmp(modelsFilterResult[listCount-1].modelName ,"SSD", 3) == 0) //Detection
       {
         printf("Running the Single Shot MultiBox Detector (SSD).\n");
         //strcpy(command, EXAMPLE_DETECTION_COMMAND_MYRIAD);
       }   
       else 
          printf("Model definition error. Please check the Model schema.\n");

    }
    else if(strcmp(getDevModel(), "NEON-1000-JNX") == 0)
    {
      //TODO
       printf("Running at NEON-1000-JNX.\n"); 
    }
    else if(strcmp(getDevModel(), "NEON-1000-JT2") == 0)
    {
      //TODO
       printf("Running at NEON-1000-JT2\n");  
    }
    else{printf("Unknown device model\n"); }    

    printf(BLU_BOLD"ReadThePipelineFile = %d\n"RESET, ret); 
    ret = ReadThePipelineFile(EVA_CONFIG);
    
    if(ret != 0)
      printf(RED_BOLD"Running the EVA config error = %d\n"RESET, ret); 

    /*pid_t my_pid;

    if (command != NULL){
      if(strlen(command) != 0)
      {
        system(command);
        runflag = 1;
      }
    }*/
    

}

void removeTheCrt()
{
  char input[64];
  char cerFile[1024]={0},keyFile[1024]={0},prikeyFile[1024]={0};
  int ret;

  strcpy(cerFile, "/home/adlink/car/updPub.crt");
  strcpy(keyFile, "/home/adlink/car/updPriv.pub");
  strcpy(prikeyFile, "/home/adlink/car/updPriv.priv");

  printf(BLU_BOLD"Would you like to uninstall the certificate? (yes/no)\n"RESET);
  scanf("%s", input);

  if(strcmp(input, "yes") == 0) 
  {
      ret = uninstallPkgCertificate(prikeyFile);
      if (ret)
          printf("Certificate uninstall fail!\n");
      else
          printf("Certificate uninstall successfully!\n");
  }
  else if (strcmp(input, "no") == 0) 
  {

  }else{}

}

void installTheCrt()
{
  char input[64];
  char cerFile[1024]={0},keyFile[1024]={0},prikeyFile[1024]={0};
  int ret;

  strcpy(cerFile, "updPub.crt");
  strcpy(keyFile, "updPriv.pub");
  strcpy(prikeyFile, "updPriv.priv");

  printf(BLU_BOLD"Would you like to install the certificate? (yes/no)\n"RESET);
  scanf("%s", input);

  if(strcmp(input, "yes") == 0) 
  {
      printf("========================Checking certificate is invalid or not.========================\n");
      //ret = isValidCertificate(cerFile);
      ret = installPkgCertificate(cerFile, keyFile);//
      if(ret != 0)
      {
              printf("Install PkgCertificate. Error: %d\n", ret);
      }
      else{printf("Install Pkg Certificate successfully: %d\n", ret);}
      
      ret = isValidCertificate(cerFile);
      if(ret == 0)
      {
         printf("Certificate is valid :%d\n", ret);   

      }else{printf("Certificate is invalid :%d\n", ret);}

      
  }
  else if (strcmp(input, "no") == 0) 
  {
      //TODO
  }
  
}

void ListDeviceInfo()
{
   char output[64];
  //setPackageID();
  
  printf("--[Device information]--\n");

  //snprintf(output, sizeof(output), getProjToken());
  printf("[Project token]:%s\n", getProjToken());

  //snprintf(output, sizeof(output), getPackageID());
  printf("[Pcakage id]:%s\n", getPackageID());

  //snprintf(output, sizeof(output), getDevID());
  printf("[Device id]:%s\n", getDevID());

  //snprintf(output, sizeof(output), getDevBrand());
  printf("[Device brand]:%s\n", getDevBrand());

  //snprintf(output, sizeof(output), getDevModel());
  printf("[Device model]:%s\n", getDevModel());
}

void* Agent(void* data) {
  char *str = (char*) data; 
  int codeValue, count, ret;
  char Message[100];
  enum State code;

  while(!exitRequested)
  {
      SleepTime(5);
      printf("%s\n", "Thread function"); 
      // //printf("%s\n", str); 
      SleepTime(2);
      //Reading the Json file 
      codeValue = readJsonFile(da_state, Message);

      printf("[Child]Thread running! \n");
      if(silentInstall == 1 && codeValue == DownloadAgent)
      {
          //TerminatePipeline();
       
          printf(YEL_BOLD"The new package is avalible. The agent will be intall the new package automatically.\n"RESET);
          writeToJsonFile(da_state, ConfirmDownload, "Start downloading EVA package.");

          count = 0;
      
      }
      else if(silentInstall == 1 && codeValue == DownloadSuccess)
      {
          SleepTime(2);
          count ++;
          //printf("Upgrading equipement's package.\n");

          if(count == 1)
          {
              printf(GRN_BOLD"[ADLINK Agent #1] Download the EVA package file successfully! \n"RESET);
              updateStatus = StartUpgade;
              //Run the upgrate porcess
              ret = UpdatePackageProcess(); 
              if(ret == 0)
              {
                updateStatus = UpgadeSuccess;
                printf(GRN_BOLD"[ADLINK Agent #1] Upgrade package successfully then terminate pipeline! \n"RESET);
                TerminatePipeline();
              }
              else
              {
                printf(RED_BOLD"[ADLINK Agent #1] Upgrade package fail! Please check the package is valid or not! \n"RESET);
                printf("\n");
                updateStatus = UpgadeFail;
              }

          }
          if(count == 10)
            count = 0;
      }
      else
      {
          if(codeValue == DownloadAgent && (runflag == 0))
          {
              printf(YEL_BOLD"[ADLINK Agent #1]The new package is avalible. If you want to update the package. Please terminate the pipeline.\n"RESET);
          }
      }
  }

  #ifdef WIN32
  // Close all thread handles 
  WaitForSingleObject(thread, INFINITE);
  CloseHandle(thread);
  thread = NULL;
  #else
  pthread_exit(NULL);
  #endif
  
}

void help_usage(void)
{
    printf("Option: [Description] \n");
    printf("	-o [ Upgrade the EVA package offline. ]\n");
    printf("	-r [ Running the EVA pipeline offline by gst-launch-1.0]\n");
    printf("	-k [ Install / Uninstall the edge assert's key certification. ]\n");
    printf("	-i [ Showing the edge assert's information. ]\n");
    printf("	-s [ Silent upgrade the EVA package file. ]\n");
    printf("	-t [ Creating the carota project token. ]\n");
    printf("	-h [ This is help message. ]\n");
    return;
}

int main(int argc ,char * argv[],char **envp) {

  int ret = 0;
  int *codeValue, *codeValueUpgrade;
  char input[512]; 
  char Message[100], UpgradeMessage[100];
  char dest[100];
  int Json_file_count = 2 , runPackageMode = 0 ,runPipelineLocal = 0, keyFlag = 0 ,cmd_opt = 0;
  int project_token_init = 0;
  int deviceInfoFlag = 0;
  extern char **environ;
  int opt = 1;
  
  //SleepTime(10);
  #ifdef _WIN32
  //Do the windows
  #else
  pthread_t thread;
  #endif

  //printf ("Start getopt = (%s)\n", "");
 
  while (opt) {
        cmd_opt = getopt(argc, argv, "orkisth?");

        /* End condition always first */
        if (cmd_opt == -1) {
            break;
        }
        /* Lets parse */
        switch (cmd_opt) 
        {
            case 'h':
            case '?':
                help_usage();
                return 0;
            case 'o':
                runPackageMode = 1;
                opt = 0;
                break;
            case 'r':
                runPipelineLocal = 1;
                opt = 0;
                break;
            case 'k':
                keyFlag = 1;
                opt = 0;
                break;
            case 'i':
                deviceInfoFlag = 1;
                opt = 0;
                break;
            case 's':
                silentInstall = 1;
                opt = 0;
                break;
            case 't':
                project_token_init = 1;
                opt = 0;
                break;
        }
  }
  //printf ("Stop getopt = (%s)\n", "");

  if(runPipelineLocal == 1){
      if(ReadThePipelineFile(EVA_CONFIG) != 0 ){
        return 0;
      }
  }

  //SetEVAEnviroment();
  if(keyFlag == 1) {
      removeTheCrt();
      installTheCrt();
      return 0;
  }
  //List the Edge device's information
  if(deviceInfoFlag == 1)
  {

    ListDeviceInfo(); 
    return 0; 
    
  }

  //Create the project token
  if(project_token_init == 1)
  {CreateTheProjectToken(); return 0;}
  
  //List the model file in the edge assert
  listModelFileList();
  //Delete the package model information
  deletePackageModel();

   //Upgrade the package offline
  if(runPackageMode == 1)
  {UpdatePackageOffline();}
  
  //FileCheckValid(EXAMPLE_JSON, "wb");
  //writeToJsonFile(EXAMPLE_JSON, 0, "project_token_num");
  //setexite("b9DDH1pEpT");
   
  #ifdef WIN32
  // STARTUPINFO info={sizeof(info)};
  // PROCESS_INFORMATION processInfo;
  // if (CreateProcess("c:\\ADLINK\\eva\\Managebility\\bin\\otaAgent.exe", NULL, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
  // {
  //     WaitForSingleObject(processInfo.hProcess, INFINITE);
  //     CloseHandle(processInfo.hProcess);
  //     CloseHandle(processInfo.hThread);
  // }

  #else
  pid_t pid;
  
  //fork PID == 0 is child process
  if (0 == (pid = fork()))
  {
      printf("Create the process PID1=%d\n", pid);
      PID1 = getpid();

      printf("[Child] Child's PID1 is %d\n", getpid());
      int rc = exec_prog(my_argv, pid);
      if(rc!=0)
        {printf("Create the process fail.=%d\n", ret);}
 
  }
  #endif

  //ListDeviceInfo();

  signal(SIGINT, SignalHandler);

  #ifdef WIN32
  HANDLE thread = CreateThread(NULL, 0, Agent, NULL, 0, NULL);
  if (thread) {
    // Optionally do stuff, such as wait on the thread.
  }
  #else
  pthread_create(&thread, NULL, Agent, "Child Process");
  #endif

  while(!exitRequested)
  {
      int codeState, upgrageState = 0;
      int err = 0;
      //printf("%s\n", "========================CarOTA Update Process========================"); 

      //Write the json file
      //writeToJsonFile(EXAMPLE_JSON, i, "ADLINK agent message.");
      SleepTime(3);
      //Reading the Json file 
      codeState = readJsonFile(da_state, Message);

      //Reading the udgrade status Json file
      //err = readJsonFile(upgrade_result, &codeValueUpgrade, UpgradeMessage);
      //upgrageState = codeValueUpgrade;

      switch( codeState )
      {
          case CheckServer:
                printf("Please wait for periodical check with server.\n");
                break;
          case DownloadAgent:
                if(silentInstall == 1)
                { strcpy(input, "yes"); } 
                else {
                  questionDownload:printf(BLU_BOLD"Do you want to download the package (yes/no/exit)?\n"RESET);
                  scanf("%s", input);
                }

                if(strcmp(input, "yes") == 0) 
                {
                    //printf("Downloading Package....\n");/
                      writeToJsonFile(da_state, ConfirmDownload, "Start downloading EVA package.");
                }
                else if(strcmp(input, "no") == 0)printf(BLU_BOLD"Waitting for downloading?\n"RESET);
                else if(strcmp(input, "exit") == 0){
                  exitRequested = 1;
                  printf(YEL_BOLD"Stopping the carota agent\n"RESET);
                }
                else goto questionDownload; 
                break;
          case StartDownload :
                printf("Please wait downloading files in carota service.\n");  
                break;
          case DownloadFail :
                printf("Downloading package fail. Please check the carota service.\n");  
                break;
          case DownloadSuccess :
                //sleep(2);
                printf(BLU_BOLD"[ADLINK Agent]Download the EVA package file successfully! \n"RESET);
                break;
      }
      //printf(BLU_BOLD"[ADLINK Agent]Running the EVA runtime pipeline!  value = %d, err = %d, flag = %d\n"RESET, updateStatus, err, runflag);

      if((err == 0) && (updateStatus == UpgadeSuccess) && (runflag == 0))
      {
          //TerminatePipeline();
          //sleep(1);
          printf(BLU_BOLD"[ADLINK Agent]Running the EVA runtime pipeline!  value = %d, err = %d, flag = %d\n"RESET, updateStatus, err, runflag);
          //updateStatus = CheckServer;
          CreatePipeline();/*Start the EVA pipeline based on the gst-launch-1.0 or gstreamer application*/
      }
      else if(updateStatus == UpgadeFail)
      {
          updateStatus = CheckServer;
      }

     
  }

  //List the device information
  ListDeviceInfo();
   
   #ifdef __linux__
   {
      //Stop the thread
      pthread_join(thread, NULL); 
      SleepTime(1);
      //Terminate through the pid
      kill(PID1, SIGTERM);
   }
   #endif


  return 0;
}
