#ifdef _WIN32
#define da_state 	    "da.state"
#define project_token 	""
#define upgrade_result	""
#elif __linux__
#define da_state 	    "/data/carota/da.state"
#define project_token 	    "/data/carota/pro.info"
#define upgrade_result	"/data/carota/upgrade.result"
#endif

#define CODE_STRING		"code"
#define MESSAGE_STRING	"msg"
#define CAROTA_PATH	    "/data/carota"
#define EXAMPLE_JSON	"/data/carota/test.state"

#define project_token_num 	    "CAROTATEST"   //b9DDH1pEpT
#define EXAMPLE_COMMAMD "gst-launch-1.0 videotestsrc ! videoconvert ! xvimagesink"
#define EVA_SOURCE_COMMAMD "source /opt/adlink/eva/scripts/setup_eva_envs.sh"
#define COMMAND_LENGHTH   1024
#define FILE_LENGHTH      1024
#define MAX_PATH          2048

#define EXAMPLE_CLASSFICATION_COMMAND_MYRIAD "gst-launch-1.0 filesrc location=/home/adlink/Videos/animal.mp4 !\
qtdemux ! avdec_h264 ! videoscale ! video/x-raw, width=640, height=480 ! videoconvert !\
advino device=MYRIAD model=/home/adlink/vino-model/googlenet-v2.xml !\
adtrans_classifier class-num=1000 label=/home/adlink/vino-model/googlenet-v2.txt ! admetadrawer ! videoconvert ! xvimagesink sync=false"

#define EXAMPLE_DETECTION_COMMAND_MYRIAD "gst-launch-1.0 filesrc location=/home/adlink/Videos/face.mp4 !\
qtdemux ! avdec_h264 ! videoscale ! video/x-raw, width=640, height=480 ! videoconvert ! advino device=MYRIAD model=/home/adlink/vino-model/yolo.xml !\
adtrans_yolo mask='(3,4,5),(0,1,2),(6,7,8)' blob-size=26,52,13 input-width=416 input-height=416 label=/home/adlink/vino-model/yolo.txt ! admetadrawer ! videoconvert ! xvimagesink sync=false"

#define EXAMPLE_COMMAMD1 "gst-inspect-1.0 advino"
#define EVA_CONFIG "/tmp/pipeconfig/evaconfig.txt"

#ifdef __linux__
#define RED_BOLD "\x1b[;31;1m"
#define BLU_BOLD "\x1b[;34;1m"
#define YEL_BOLD "\x1b[;33;1m"
#define GRN_BOLD "\x1b[;32;1m"
#define CYAN_BOLD_ITALIC "\x1b[;36;1;3m"
#define RESET "\x1b[0;m"
#elif _WIN32
#define RED_BOLD ""
#define BLU_BOLD ""
#define YEL_BOLD ""
#define GRN_BOLD ""
#define CYAN_BOLD_ITALIC ""
#define RESET ""
#endif

int writeToJsonFile(char* filename, int CodeValue, char* MessageString);
int readJsonFile(char *filename, char *Message);

//static pid_t processid;


/* Defines an CarOTA state code enumeration type */
enum State         
{
    CheckServer = 0,       /* Wait for periodical check with server.*/
    GotUpdateRask = 1,     /* Got the update campaign info from server.*/
    DownloadAgent = 2,     /* Need Adlink Agent to feedback the confirm for downloading Package (FW).*/
    ConfirmDownload,       /* Adlink Agent confirms to download FW.*/
    StartDownload,         /* Start downloading FW. */
    DownloadSuccess,       /* Downloading Successful.*/
    DownloadFail,          /* Downloading Fail.*/
    StartUpgade,
    UpgadeSuccess,
    UpgadeFail,
} State_Code;
