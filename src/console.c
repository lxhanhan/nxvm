/* This file is a part of NVMx86 project. */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "nvm/vmachine.h"

#include "global.h"
#include "debug.h"
#include "console.h"

static int exitFlag;

static void parse(char *s)
{
	int i = 0;
	while(s[i] != '\0') {
		if(s[i] == '\n') {
			s[i] = '\0';
			break;
		}
		if(s[i] > 0x40 && s[i] < 0x5b)
			s[i] += 0x20;
		i++;
	}
}

void NSExec()
{
	char execmd[MAXLINE];
	FILE *load;
	t_nubit8 c;
	t_nubit16 i = 0,end;
	t_nubit32 len = 0;
	fprintf(stdout,".COM File: ");
	fgets(execmd,MAXLINE,stdin);
	parse(execmd);
	if(!strlen(execmd)) return;
	load = fopen(execmd,"rb");
	if(!load) {
		fprintf(stdout,"File not found\n");
		return;
	} else {
		vcpu.ax = vcpu.bx = vcpu.cx = vcpu.dx = 0x0000;
		vcpu.si = vcpu.di = vcpu.bp = 0x0000;
		vcpu.sp = 0xffee;	vcpu.ip = 0x0100;
		vcpu.ds = vcpu.es = vcpu.ss = vcpu.cs = 0x0001;
		cpuTermFlag = 0;
		c = fgetc(load);
		while(!feof(load)) {
			vmemorySetByte(vcpu.cs+i,vcpu.ip+len++,c);
			i = len / 0x10000;
			c = fgetc(load);
		}
		end = vcpu.ip+len;
		fprintf(stdout,"File '%s' is loaded to 0001:0100, length is %d bytes.\n\n",execmd,len);
		fclose(load);
		while(vcpu.ip < end && !cpuTermFlag) vcpuInsExec();
		fprintf(stdout,"\n");
	}
}

void NSExit()
{
	exitFlag = 1;
}

void NSHelp()
{
	fprintf(stdout,"\nNVM Console\n");
	fprintf(stdout,"===========\n");
	fprintf(stdout,"DEBUG\t\tRuns Debug, a program testing and editing tool.\n");
	fprintf(stdout,"EXEC\t\tRuns a binary of '.COM' format from host machine.\n");
	fprintf(stdout,"EXIT\t\tQuits the NVM console (command interpreter).\n");
	fprintf(stdout,"HELP\t\tProvides Help information for NVM console commands.\n\n");
	fprintf(stdout,"NVM Operations\n");
	fprintf(stdout,"============\n");
	fprintf(stdout,"ON\t\tPowers on Neko's x86 Virtual Machine.\n");
	//fprintf(stdout,"STATUS\t\tPrints the status of NVM.\n");
	fprintf(stdout,"MEMORY\t\tAssigns the memory size of NVM.\n");
	fprintf(stdout,"MEMORYTEST\tTests the memory size of NVM.\n\n");
/*	fprintf(stdout,"NVM Operations\n");
	fprintf(stdout,"==============\n");
	fprintf(stdout,"OFF\t\tTurns off Neko's x86 Virtual Machine.\n");
	fprintf(stdout,"RESET\t\tRestarts Neko's x86 Virtual Machine.\n");*/
	
}

void NSMemory()
{
	t_nubit32 tempSize;
	char str[MAXLINE];
	if(runFlag) {fprintf(stdout,"Cannot modify memory size now.\n");return;}
	fflush(stdin);
	fprintf(stdout,"Size(KB): ");
	fgets(str,MAXLINE,stdin);
	tempSize = atoi(str);
	if(tempSize > 0x400) {
		memorySize = (tempSize<<0x0a);
		MemoryTerm();
		MemoryInit();
	} else {
		memorySize = (1024<<0x0a);
		MemoryTerm();
		MemoryInit();
	}
}
void NSMemoryTest()
{
	t_bool testFlag = 0;
	t_nubit32 i;
	if(runFlag || !initFlag) {fprintf(stdout,"Cannot perform memory test now.\n");return;}
	for(i = 0;i < memorySize;++i)
	{
		if(i % 1024 == 0) fprintf(stdout,"\rMemory Testing : %dK",i/1024);
		if(*(t_nubit8 *)(memoryBase+i) != 0) {
			fprintf(stdout,"\nMemory test failed.\n");
			testFlag = 1;
			break;
		}
	}
	if(!testFlag) fprintf(stdout,"\rMemory Testing : %dK OK\n",i/1024);
	return;
}

void NSOn()
{
	if(initFlag) NVMTerm();
	NVMPowerOn();
}
/*void NSOff()
{
	NVMPowerOff();
	if(!initFlag) NVMInit();
}
void NSReset()
{
	if(runFlag) {
		NVMPowerOff();
		NVMPowerOn();
	}
}*/

void NSConsole()
{
	char cmdl[MAXLINE];
	exitFlag = 0;
	if(!initFlag) NVMInit();
	fprintf(stdout,"\nPlease enter 'HELP' for information.\n\n");
	while(!exitFlag) {
		fflush(stdin);
		fprintf(stdout,"Console> ");
		fgets(cmdl,MAXLINE,stdin);
		parse(cmdl);
		if(!strlen(cmdl)) continue;
		else if(!strcmp(cmdl,"debug")) NSDebug();
		else if(!strcmp(cmdl,"exec")) NSExec();
		else if(!strcmp(cmdl,"exit")) NSExit();
		else if(!strcmp(cmdl,"help")) NSHelp();
		//else if(!strcmp(cmdl,"pwd")) fprintf(stdout,"%s\n",argv[0]);

		else if(!strcmp(cmdl,"memory")) NSMemory();
		else if(!strcmp(cmdl,"memorytest")) NSMemoryTest();

		else if(!strcmp(cmdl,"on")) NSOn();
		/*else if(!strcmp(cmdl,"off")) NSOff();
		else if(!strcmp(cmdl,"reset")) NSReset();*/

		else fprintf(stdout,"Illegal command '%s'.\n",cmdl);
		fprintf(stdout,"\n");
	}
	if(initFlag) NVMTerm();
}