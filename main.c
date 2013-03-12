#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>

#define LOG_TAG "USB3G"
#include <cutils/log.h>

#include "UEventFramework.h"

static char usb_vid[0x10], usb_pid[0x10];
static char busb_vid[5], busb_pid[5];
static int switchcount = 0;


static void do_coldboot(DIR *d, int lvl)
{
    struct dirent *de;
    int dfd, fd;

    dfd = dirfd(d);

    fd = openat(dfd, "uevent", O_WRONLY);
    if(fd >= 0) {
        write(fd, "add\n", 4);
        close(fd);
    }

    while((de = readdir(d))) {
        DIR *d2;

        if (de->d_name[0] == '.')
            continue;

        if (de->d_type != DT_DIR && lvl > 0)
            continue;

        fd = openat(dfd, de->d_name, O_RDONLY | O_DIRECTORY);
        if(fd < 0)
            continue;

        d2 = fdopendir(fd);
        if(d2 == 0)
            close(fd);
        else {
            do_coldboot(d2, lvl + 1);
            closedir(d2);
        }
    }
}

static void coldboot(const char *path)
{
    DIR *d = opendir(path);
    if(d) {
        do_coldboot(d, 0);
        closedir(d);
    }
}

int read_vid_pid(char * path)
{
	int fd,size;
	char usb_path[0x60] = {0};
	
	memset(usb_vid,0,sizeof(usb_vid));
	memset(usb_pid,0,sizeof(usb_pid));
	
	//read Vid
	memset(usb_path,0,0x60);
	strcat(usb_path,path);
	strcat(usb_path,"/idVendor");
	fd=open(usb_path,O_RDONLY);
	size=read(fd,usb_vid,sizeof(usb_vid));
	close(fd);
	SLOGI("VID :size %d,vid_path '%s',VID  '%s'.\n",size,usb_path,usb_vid);	
	if(size<=0)
	{
		SLOGE("VID :err\n");
		return -1;	
	}	
	usb_vid[size-1] = 0;
	
	memset(usb_path,0,0x60);
	strcat(usb_path,path);
	strcat(usb_path,"/idProduct");
	fd=open(usb_path,O_RDONLY);
	size=read(fd,usb_pid,sizeof(usb_pid));
	close(fd);

	SLOGI("PID :size %d,Pid_path '%s',PID  '%s'.\n",size,usb_path,usb_pid);	
	if(size<=0)
	{
		SLOGE("PID :err\n");	
		return -1;
	}	
	usb_pid[size-1] = 0;
	
	return 0;
}


int addmanually(char *vid, char *pid)
{
char szt[200];
sprintf(szt, "echo \"%s %s\" > /sys/bus/usb-serial/drivers/option1/new_id &",vid, pid);
SLOGI("cmd=%s,", szt);
return system(szt); 	
}

void handleUsbEvent(struct uevent *evt)
{
	pid_t pid;
    const char *devtype = evt->devtype;  
    char *p, *cmd = NULL, path[0x60] = {0};  
    char *argv_rc[] =
	{
		NULL,
		NULL,
		NULL
	};
    int ret,status;
    char buffer[256];
    char oldVid[5] = "0000", oldPid[5] = "0000";
    

		SLOGI("event { '%s', '%s', '%s', '%s', %d, %d }\n", evt->action, evt->path, evt->subsystem, evt->firmware, evt->major, evt->minor);              
					
		if(!strcmp(evt->action, "add") && !strcmp(devtype, "usb_device")) {   
    
        /*call usb mode switch function*/  
		SLOGI("event { '%s', '%s', '%s', '%s', %d, %d }\n", evt->action, evt->path, evt->subsystem, evt->firmware, evt->major, evt->minor);   
		p = strstr(evt->path,"usb");
		
        if(p == NULL)     
        {
        	return;	
        }	
        p += sizeof("usb");
        p = strchr(p,'-');
        if(!p) return;	
          
        strcat(path,"/sys");
        strcat(path,evt->path);
        SLOGI("path : '%s'\n",path); 
        ret = read_vid_pid(path);
        if((ret < 0) || (usb_pid == NULL) || (usb_vid == NULL)) return;	
               
        // add for zoomdata,StrongRising 3g dongle
        if(!strncmp(usb_vid,"8888",4)&& !strncmp(usb_pid, "6500",4))
        	sleep(8);
        // work around for varius dongles -- bartoszj
		if(!strncmp(usb_vid,"19d2",4) && !strncmp(usb_pid,"1514",4)) {
		SLOGI("Probably ZTE MF195 connected...Adding manually to option drv...\n");
		addmanually(usb_vid,usb_pid);
		}	
		else if(!strncmp(usb_vid,"0bda",4) && !strncmp(usb_pid,"8176",4)) {
		SLOGI("Internal wifi module detected, skipping...\n");
		return;
		}	
		else if(!strncmp(usb_vid,"19d2",4) && !strncmp(usb_pid,"1515",4)) {
		SLOGI("Probably ZTE MF195 connected...Adding manually to option drv...\n");
		addmanually(usb_vid,usb_pid);
		}			
		else if(!strncmp(usb_vid,"19d2",4) && !strncmp(usb_pid,"1506",4)) {
		SLOGI("Probably ZTE MF398u1 connected...Adding manually to option drv...\n");
		addmanually(usb_vid,usb_pid);
		}				
		else if(!strncmp(usb_vid,"19d2",4) && !strncmp(usb_pid,"0167",4)) {
		SLOGI("Probably ZTE MF821 connected...Adding manually to option drv...\n");
		addmanually(usb_vid,usb_pid);
		}		
		else if(!strncmp(usb_vid,"0421",4) && !strncmp(usb_pid,"061e",4)) {
		SLOGI("Probably NOKIA CS-11 connected...Adding manually to option drv...\n");
		addmanually(usb_vid,usb_pid);
		}	
	    else if(!strncmp(usb_vid,"0421",4) && !strncmp(usb_pid,"0638",4)) {
		SLOGI("Probably NOKIA 21M-02 connected...Adding manually to option drv...\n");
		addmanually(usb_vid,usb_pid);
		}	
		else if(!strncmp(usb_vid,"0af0",4) && !strncmp(usb_pid,"d157",4)) {
		SLOGI("Probably OPTION ICON 515M connected...Adding manually to option drv...\n");
		addmanually(usb_vid,usb_pid);
		}	
		else if(!strncmp(usb_vid,"12d1",4) && !strncmp(usb_pid,"1411",4)) {
		SLOGI("HUAWEI E510 connected...Adding manually to option drv...\n");
		addmanually(usb_vid,usb_pid);
		}
		else if(!strncmp(usb_vid,"2001",4) && (!strncmp(usb_pid,"7d01",4) || !strncmp(usb_pid,"7d02",4) || !strncmp(usb_pid,"7d03",4))) {
		SLOGI("A D-Link MODEM connected...Adding manually to option drv...\n");
		addmanually(usb_vid,usb_pid);
		}	
		FILE *fcmd=popen("getprop SWITCH_VID", "r");
		fgets(oldVid, sizeof(oldVid), fcmd);
		pclose(fcmd);
		 
		fcmd=popen("getprop SWITCH_PID", "r");
		fgets(oldPid, sizeof(oldPid), fcmd);
	    pclose(fcmd);
		switchcount++;
		if(!strncmp(oldVid,"19d2",4) && !strncmp(oldPid,"f006",4) && !strncmp(usb_pid,"2000",4)) {
		SLOGI("Probably ZTE K3570-Z connected...Reseting...\n");
		switchcount--;
		oldVid[0]=0xA;
		oldPid[0]=0xA;
		}	
		if((oldVid[0]!=0xA && oldPid[0]!=0xA) || (oldPid[0]!=0xA && strcmp(oldPid,usb_pid))) {
		SLOGI("Old device was %s_%s\n",oldVid,oldPid);
		SLOGI("Device already switched, skipping...\n");
		//addmanually(usb_vid,usb_pid);	
		}
		else {
		char fname[255] = {0};
		sprintf(fname,"/system/etc/usb_modeswitch.d/%s_%s",usb_vid, usb_pid);
		FILE * f = fopen(fname,"r");
		if(!f) SLOGI("Warning! Configuration file doesn't exist for %s_%s!",usb_vid, usb_pid);
		else fclose(f);
		asprintf(&cmd, "/system/bin/usb_modeswitch -W -I \"-v %s -p %s -c %s\" &",usb_vid, usb_pid, fname);
        ret = system(cmd); 
        free(cmd);
        SLOGI("excute ret : %d,err:%s\n",ret,strerror(errno));
		if(oldPid[0]==0xA) {
	
		SLOGI("Setting envs to %s and %s",usb_vid,usb_pid); 
        asprintf(&cmd,"setprop SWITCH_VID %s &",usb_vid);
        system(cmd);
        free(cmd);
        asprintf(&cmd,"setprop SWITCH_PID %s &",usb_pid);
        system(cmd);
        free(cmd);
        	}
   		 }
    } 
	if(!strcmp(evt->action, "remove") && !strcmp(devtype, "usb_device")) {    
		SLOGI("event { '%s', '%s', '%s', '%s', %d, %d }\n", evt->action, evt->path, evt->subsystem,
                    evt->firmware, evt->major, evt->minor);
                    
        FILE *fcmd=popen("getprop SWITCH_PID", "r");
   		fgets(oldPid, sizeof(oldPid), fcmd);
  		pclose(fcmd);		
        if((strcmp(usb_pid,oldPid) && oldPid[0] != 0xA) || switchcount>1){
        asprintf(&cmd,"setprop SWITCH_VID \"\" &");
        system(cmd);
        free(cmd);
        asprintf(&cmd,"setprop SWITCH_PID \"\" &");
        system(cmd);
        free(cmd);
        switchcount = 0;
         }
	}
}    
    
/* uevent callback function */
static void on_uevent(struct uevent *event)
{
	const char *subsys = event->subsystem;
                   
	if (!strcmp(subsys, "usb")) {
    	handleUsbEvent(event);
    }                    
    
}

int main()
{
	SLOGI("USB3G Monitor ver. 0.1e fixed by lolet -- built on %s, %s\n",__DATE__, __TIME__); 
	
	uevent_init();
	coldboot("/sys/devices");
	uevent_next_event(on_uevent);	
	return 0;
}
