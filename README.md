# KmdfBugchecker
Kernel mode driver that allows user space applications to trigger a bugcheck with ANY code through IOCTL.

# Usage
Download and drop the KmdfBugchecker.sys file anywhere, then on an elevated command prompt run

**sc create Bugchecker binPath=C:\Where\The\File\Is\KmdfBugchecker.sys type=kernel start=auto**

and then **sc start Bugchecker** to actually start it.

> Okay yeah, but how do I call a BSOD?

Use [this tool.](https://github.com/AestheticalZ/bugchecker-communicator)
