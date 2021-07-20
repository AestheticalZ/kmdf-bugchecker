#include <ntddk.h>
#include <wdf.h>
#include <wdm.h>

#define IOCTL_BUGCHECK CTL_CODE(FILE_DEVICE_UNKNOWN, 0x2100, METHOD_BUFFERED, FILE_ANY_ACCESS)

UNICODE_STRING DEV_NAME = RTL_CONSTANT_STRING(L"\\Device\\BugcheckerDevice");
UNICODE_STRING DEVSYM_NAME = RTL_CONSTANT_STRING(L"\\??\\BugcheckerSymlink");

void DriverUnload(PDRIVER_OBJECT pdriverObj)
{
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "KmdfBugchecker: Unloading driver."));
	IoDeleteDevice(pdriverObj->DeviceObject);
	IoDeleteSymbolicLink(&DEVSYM_NAME);
}

NTSTATUS HandleBugcheckIoctl(PDEVICE_OBJECT deviceObj, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(deviceObj);

	PIO_STACK_LOCATION pioStackLoc = IoGetCurrentIrpStackLocation(pIrp);

	if (pioStackLoc->Parameters.DeviceIoControl.IoControlCode == IOCTL_BUGCHECK)
	{
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "KmdfBugchecker: Received IOCTL_BUGCHECK!"));

		//This is where the fun begins!
		KeBugCheck(*(unsigned long*)pIrp->AssociatedIrp.SystemBuffer);
	}

	//Not really reachable lol
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS MjCreationAndClose(PDEVICE_OBJECT deviceObj, PIRP pIrp)
{
	UNREFERENCED_PARAMETER(deviceObj);

	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(pIrp);

	switch (stackLocation->MajorFunction)
	{
		case IRP_MJ_CREATE:
			KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL,
				"KmdfBugchecker: Symlink handle opened for %wZ", DEVSYM_NAME));
			break;
		case IRP_MJ_CLOSE:
			KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL,
				"KmdfBugchecker: Symlink handle closed for %wZ", DEVSYM_NAME));
			break;
		default:
			break;
	}

	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObj, PUNICODE_STRING registryPath)
{
	UNREFERENCED_PARAMETER(registryPath);

	NTSTATUS ntstatus = STATUS_SUCCESS;
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "KmdfBugchecker: Initializing driver."));

	driverObj->DriverUnload = DriverUnload;
	driverObj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HandleBugcheckIoctl;

	driverObj->MajorFunction[IRP_MJ_CREATE] = MjCreationAndClose;
	driverObj->MajorFunction[IRP_MJ_CLOSE] = MjCreationAndClose;

	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "KmdfBugchecker: Driver initialized."));

	ntstatus = IoCreateDevice(driverObj, 0, &DEV_NAME, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN,
			FALSE, &driverObj->DeviceObject);
	if (!NT_SUCCESS(ntstatus))
	{
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Could not create device: %wZ", DEV_NAME));
	}
	else KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Created device: %wZ", DEV_NAME));

	ntstatus = IoCreateSymbolicLink(&DEVSYM_NAME, &DEV_NAME);
	if (!NT_SUCCESS(ntstatus))
	{
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Error creating symlink: %wZ", DEVSYM_NAME));
	}
	else KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Created symlink: %wZ", DEVSYM_NAME));

	return STATUS_SUCCESS;
}