#include "builtin.h"
#include <sys/proc_info.h>

std::string getProcInfo(pid_t pid) {
	std::string status = getProcessStatus(pid);
	std::string exec_path = getExecPath(pid);
	std::string memory_usage = getMemoryUsage(pid);
	return "PID -- " + std::to_string(pid) + "\n" +
	"Process Status -- " + status + "\n" +
	"Memory -- " + memory_usage + "\n" +
	"Executable Path -- " + exec_path; 
}

std::string getExecPath(pid_t pid) {
	std::string exec_path;
	char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
	if (proc_pidpath(pid, pathbuf, sizeof(pathbuf)) <= 0) {
		exec_path = "Unable to get exec path";
	} else {
		exec_path = std::string(pathbuf);
	}
	return exec_path;
}

std::string getProcessStatus(pid_t pid) {
	std::string status;
	struct proc_bsdinfo proc;
	if (proc_pidinfo(pid, PROC_PIDTBSDINFO, 0, &proc, sizeof(proc)) > 0) {
		switch (proc.pbi_status) {
		case 1:
			status = "R";
			break;
		case 2:
			status = "S";
			break;
		case 4:
			status = "Z";
			break;
		case 8:
			status = "T";
			break;
		default:
			status = "Unknown";
		}	
		pid_t fg_pgid = tcgetpgrp(STDIN_FILENO);  
		if (fg_pgid == proc.pbi_pgid) {   
			status += "+";
		}
	} else {
		status = "Unable to get process status";
	}
	return status;
}

std::string getMemoryUsage(pid_t pid) {
	std::string memory_usage;
	struct proc_taskinfo taskinfo;
	if (proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &taskinfo, sizeof(taskinfo)) > 0) {
		memory_usage = std::to_string(taskinfo.pti_virtual_size / 1024) + " KB";
	} else {
		memory_usage = "Unable to get memory usage";
	}
	return memory_usage;
}