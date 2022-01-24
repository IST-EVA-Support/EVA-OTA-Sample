#meson.build

project(
  'ADLINK agent',
  'c',
  version: '1.0',
  meson_version: '>= 0.55',
  default_options: [
    'warning_level=1',
  ],
)

project_description = 'An example of OTA update.'

cc = meson.get_compiler('c')

if target_machine.system() == 'linux'
devInfo = 'devInfo'
jsonc = 'json-c'
UpdMod = 'libUpdMod'
caStore = 'libcaStore'
SSL = 'libssl'
crypto = 'libcrypto'
#sqlite3 = 'libsqlite3'
dl_dep = 'libdl'
#thread = 'pthread'
else
devInfo = 'C:\Program Files\ADLINK\Manageabilty\Library\devInfo'
jsonc = 'C:\\vcpkg\\packages\\json-c_x64-windows\\lib\\json-c'
UpdMod = 'C:\Program Files\ADLINK\Manageabilty\Library\UpdMod'
caStore = 'C:\Program Files\ADLINK\Manageabilty\Library\caStore'
SSL = 'C:\Program Files\OpenSSL-Win64\lib\libssl'
crypto = 'C:\Program Files\OpenSSL-Win64\lib\libcrypto'
#sqlite3 = 'C:\Program Files\ADLINK\Manageabilty\Library\sqlite3'
dl_dep = 'kernel32'
endif


## Declare agent dependency
libs_agent = []
req_agent_libs = [devInfo, jsonc, UpdMod, caStore,
             crypto, SSL]

foreach lib_name : req_agent_libs
  libs_agent += cc.find_library(lib_name,
                  required : true)
endforeach


#Declare the ADLINK manegebility dependancy
# mydeps = cc.find_library(devInfo, required : true)
# udpdeps = cc.find_library(UpdMod, required : true)
# libcaStore = cc.find_library(caStore, required : true)
# libcaStore = cc.find_library('caStore',
#                             dirs : ['C:\Program Files\ADLINK\Manageabilty\Library'],
#                             required : true)

#Declare the third party dependancy
# jsondeps = cc.find_library(json-c, required : true)
# libssl = cc.find_library(SSL, required : true)
# libcrypto = cc.find_library(crypto, required : true)
# libsqlite3 = cc.find_library(sqlite3, required : true)
# dl_dep = cc.find_library('kernel32', required : true)
#libsqlite3 = cc.find_library('C:\\Program Files\\ADLINK\\Manageabilty\\Library\\libsqlite3', required : true)

if target_machine.system() == 'windows'
libsqlite3 = cc.find_library('sqlite3',
                  dirs : ['C:\\Program Files\\ADLINK\\Manageabilty\\Library'])

libmisc = cc.find_library('misc',
                  dirs : ['C:\\Program Files\\ADLINK\\Manageabilty\\Library'])
else
libsqlite3 = cc.find_library('libsqlite3', required : true)
thread = cc.find_library('pthread', required : true)
endif


if target_machine.system() == 'windows'
inc = include_directories('C:/vcpkg/packages/json-c_x64-windows/include/json-c')
else
inc = include_directories('/usr/include/json-c')
endif

public_headers = include_directories('include')

dir_base = meson.current_source_dir()
incdir = include_directories('.')

if target_machine.system() == 'windows'
agent_sources = [
  'AdlinkAgent.c', 'getopt.c',
]
else
agent_sources = [
  'AdlinkAgent.c'
]
endif

agent_install_dir = join_paths(meson.source_root())

if target_machine.system() == 'windows'
executable('AdlinkAgent', agent_sources , 
dependencies: [libs_agent, libsqlite3],  
include_directories : [inc,public_headers],
install : true,
install_dir : agent_install_dir)
else
executable('AdlinkAgent', agent_sources , 
dependencies: [libs_agent, thread, libsqlite3],  
include_directories : [inc,public_headers],
install : true,
install_dir : agent_install_dir)
endif




