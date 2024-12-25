#include <DiepDesktop/shared/file.h>
#include <DiepDesktop/shared/debug.h>
#include <DiepDesktop/shared/alloc_ext.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


bool
FileWrite(
	const char* Path,
	FileFile File
	)
{
	int Descriptor = open(Path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if(Descriptor < 0)
	{
		return false;
	}

	if(ftruncate(Descriptor, File.Length) == -1)
	{
		return false;
	}

	if(lseek(Descriptor, 0, SEEK_SET) == -1)
	{
		return false;
	}

	ssize_t Bytes = write(Descriptor, File.Buffer, File.Length);
	close(Descriptor);

	return Bytes == File.Length ? true : false;
}


bool
FileRead(
	const char* Path,
	FileFile* File
	)
{
	int Descriptor = open(Path, O_RDONLY);
	if(Descriptor < 0)
	{
		return false;
	}

	bool Status = false;

	if(lseek(Descriptor, 0, SEEK_SET) == -1)
	{
		goto goto_end;
	}

	struct stat Stat;
	if(fstat(Descriptor, &Stat) == -1)
	{
		goto goto_end;
	}

	File->Length = Stat.st_size;
	File->Buffer = AllocMalloc(File->Length);
	HardenedAssertNotNull(File->Buffer);

	if(read(Descriptor, File->Buffer, File->Length) == -1)
	{
		FileFree(*File);
	}
	else
	{
		Status = true;
	}



	goto_end:

	close(Descriptor);
	return Status;
}


void
FileFree(
	FileFile File
	)
{
	AllocFree(File.Length, File.Buffer);
}
