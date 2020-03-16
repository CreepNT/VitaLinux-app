/*
 * Simple kplugin loader by xerpi
 * Linux fork by CreepNT
 */

#include <stdio.h>
#include <taihen.h>
#include <psp2/ctrl.h>
#include <psp2/appmgr.h>
#include <psp2/io/fcntl.h>
#include "debugScreen.h"

#define printf(...) psvDebugScreenPrintf(__VA_ARGS__)

#define BOOTSTRAPPER_PATH "ux0:app/VITALINUX/baremetal-loader.skprx"
#define PAYLOAD_PATH "ux0:linux/vita-baremetal-linux-loader.bin"

static void wait_key_press(SceCtrlButtons key_mask);
static void show_exit_prompt();

int main(int argc, char *argv[])
{
	int ret;
	SceUID mod_id;
	SceCtrlButtons keys_to_press;

	psvDebugScreenInit();

	printf("Plugin Loader by xerpi\n");
	printf("Linux fork by CreepNT\n");
	
	ret = sceAppMgrUmount("app0:"); //Unmount app0: to get R/W access to ux0:app/VITALINUX/
	if (ret < 0){
		printf("Error unmounting app0: 0x%08X\n",ret);
		show_exit_prompt(); 
	}
	
	SceUID fd = sceIoOpen(BOOTSTRAPPER_PATH, SCE_O_RDONLY|SCE_O_NBLOCK,0777); //Check if the plugin is present (should never fail)
	if (fd < 0){
		printf("Error opening " BOOTSTRAPPER_PATH " : 0x%08X\n",fd);
		sceIoClose(fd); //Close the file
		show_exit_prompt(); 
		return 0;
	}
	
	fd = sceIoOpen(PAYLOAD_PATH, SCE_O_RDONLY|SCE_O_NBLOCK, 0777); //Check if the payload is present
	if (fd < 0){
		printf("Error opening the payload : 0x%08X\n",fd);
		printf("Did you place the payload ?\n");
		sceIoClose(fd); //Close the file
		show_exit_prompt(); 
		return 0;
	}
	
	keys_to_press = SCE_CTRL_CROSS;
	printf("\nPress X to launch the bootstrapper.\n");
	wait_key_press(keys_to_press);

	tai_module_args_t argg; //Setup parameters for a kernel plugin
	argg.size = sizeof(argg);
	argg.pid = KERNEL_PID;
	argg.args = 0;
	argg.argp = NULL;
	argg.flags = 0;
	mod_id = taiLoadStartKernelModuleForUser(BOOTSTRAPPER_PATH, &argg); //Load the plugin

	if (mod_id < 0)
		printf("Error loading " BOOTSTRAPPER_PATH ": 0x%08X\n", mod_id);
		show_exit_prompt();
		return 0;

	printf("Module loaded with ID: 0x%08X\n", mod_id);
	//At this point, the bootstrapper should setup the baremetal payload and launch it.

	if (mod_id >= 0) {
		tai_module_args_t argg;
		argg.size = sizeof(argg);
		argg.pid = KERNEL_PID;
		argg.args = 0;
		argg.argp = NULL;
		argg.flags = 0;
		ret = taiStopUnloadKernelModuleForUser(mod_id, &argg, NULL, NULL);
		printf("Stop unload module: 0x%08X\n", ret);
	}
	
	show_exit_prompt();

	return 0;
}

void wait_key_press(SceCtrlButtons key_mask)
{
	SceCtrlData pad;

	while (1) {
		sceCtrlPeekBufferPositive(0, &pad, 1);
		if (pad.buttons == key_mask)
			break;
		sceKernelDelayThread(200 * 1000);
	}
}

void show_exit_prompt(){
	SceCtrlButtons keys_to_press = SCE_CTRL_START | SCE_CTRL_SELECT;
	printf("\nPress START + SELECT to exit.\n");
	wait_key_press(keys_to_press);
}
