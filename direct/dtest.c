/*
* Test code for the "direct" interface. 
*/


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "yaffsfs.h"

void dumpDir(const char *dname);

char xx[600];

void copy_in_a_file(char *yaffsName,char *inName)
{
	int inh,outh;
	unsigned char buffer[100];
	int ni,no;
	inh = open(inName,O_RDONLY);
	outh = yaffs_open(yaffsName, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
	
	while((ni = read(inh,buffer,100)) > 0)
	{
		no = yaffs_write(outh,buffer,ni);
		if(ni != no)
		{
			printf("problem writing yaffs file\n");
		}
		
	}
	
	yaffs_close(outh);
	close(inh);
}

void make_a_file(char *yaffsName,char bval,int sizeOfFile)
{
	int outh;
	int i;
	unsigned char buffer[100];

	outh = yaffs_open(yaffsName, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
	
	memset(buffer,bval,100);
	
	do{
		i = sizeOfFile;
		if(i > 100) i = 100;
		sizeOfFile -= i;
		
		yaffs_write(outh,buffer,i);
		
	} while (sizeOfFile > 0);
	
		
	yaffs_close(outh);

}

void make_pattern_file(char *fn,int size)
{
	int outh;
	int marker;
	int i;
	outh = yaffs_open(fn, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
	yaffs_lseek(outh,size-1,SEEK_SET);
	yaffs_write(outh,"A",1);
	
	for(i = 0; i < size; i+=256)
	{
		marker = ~i;
		yaffs_lseek(outh,i,SEEK_SET);
		yaffs_write(outh,&marker,sizeof(marker));
	}
	yaffs_close(outh);
	
}

int check_pattern_file(char *fn)
{
	int h;
	int marker;
	int i;
	int size;
	int ok = 1;
	
	h = yaffs_open(fn, O_RDWR,0);
	size = yaffs_lseek(h,0,SEEK_END);
		
	for(i = 0; i < size && ok; i+=256)
	{
		yaffs_lseek(h,i,SEEK_SET);
		yaffs_read(h,&marker,sizeof(marker));
		ok = (marker == ~i);
		if(!ok)
		{
		   printf("pattern check failed on file %s, size %d at position %d. Got %x instead of %x\n",
					fn,size,i,marker,~i);
		}
	}
	yaffs_close(h);
	return ok;
}



void dump_file(const char *fn)
{
	int i;
	int size;
	int h;
	
	h = yaffs_open(fn,O_RDONLY,0);
	if(h < 0)
	{
		printf("*****\nDump file %s does not exist\n",fn);
	}
	else
	{
		size = yaffs_lseek(h,0,SEEK_SET);
		printf("*****\nDump file %s size %d\n",fn,size);
		for(i = 0; i < size; i++)
		{
			
		}
	}
}

void short_scan_test(const char *path, int fsize, int niterations)
{
	int i;
	char fn[100];
	
	sprintf(fn,"%s/%s",path,"f1");
	
	yaffs_StartUp();
	for(i = 0; i < niterations; i++)
	{
		printf("\n*****************\nIteration %d\n",i);
		yaffs_mount(path);
		printf("\nmount: Directory look-up of %s\n",path);
		dumpDir(path);
		make_a_file(fn,1,fsize);
		yaffs_unmount(path);
	}
}

void scan_pattern_test(const char *path, int fsize, int niterations)
{
	int i;
	int j;
	char fn[3][100];
	int result;
	
	sprintf(fn[0],"%s/%s",path,"f0");
	sprintf(fn[1],"%s/%s",path,"f1");
	sprintf(fn[2],"%s/%s",path,"f2");
	
	yaffs_StartUp();
	
	for(i = 0; i < niterations; i++)
	{
		printf("\n*****************\nIteration %d\n",i);
		yaffs_mount(path);
		printf("\nmount: Directory look-up of %s\n",path);
		dumpDir(path);
		for(j = 0; j < 3; j++)
		{
			result = check_pattern_file(fn[j]);
			make_pattern_file(fn[j],fsize); 
			result = check_pattern_file(fn[j]);
		}
		yaffs_unmount(path);
	}
}

void fill_disk(char *path,int nfiles)
{
	int h;
	int n;
	int result;
	int f;
	
	char str[50];
	
	for(n = 0; n < nfiles; n++)
	{
		sprintf(str,"%s/%d",path,n);
		
		h = yaffs_open(str, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
		
		printf("writing file %s handle %d ",str, h);
		
		while ((result = yaffs_write(h,xx,600)) == 600)
		{
			f = yaffs_freespace("/boot");
		}
		result = yaffs_close(h);
		printf(" close %d\n",result);
	}
}

void fill_disk_and_delete(char *path, int nfiles, int ncycles)
{
	int i,j;
	char str[50];
	int result;
	
	for(i = 0; i < ncycles; i++)
	{
		printf("@@@@@@@@@@@@@@ cycle %d\n",i);
		fill_disk(path,nfiles);
		
		for(j = 0; j < nfiles; j++)
		{
			sprintf(str,"%s/%d",path,j);
			result = yaffs_unlink(str);
			printf("unlinking file %s, result %d\n",str,result);
		}
	}
}


void fill_files(char *path,int flags, int maxIterations,int siz)
{
	int i;
	int j;
	char str[50];
	int h;
	
	i = 0;
	
	do{
		sprintf(str,"%s/%d",path,i);
		h = yaffs_open(str, O_CREAT | O_TRUNC | O_RDWR,S_IREAD | S_IWRITE);
		yaffs_close(h);

		if(h >= 0)
		{
			for(j = 0; j < siz; j++)
			{
				yaffs_write(h,str,1);
			}
		}
		
		if( flags & 1)
		{
			yaffs_unlink(str);
		}
		i++;
	} while(h >= 0 && i < maxIterations);
	
	if(flags & 2)
	{
		i = 0;
		do{
			sprintf(str,"%s/%d",path,i);
			printf("unlink %s\n",str);
			i++;
		} while(yaffs_unlink(str) >= 0);
	}
}

void leave_unlinked_file(char *path,int maxIterations,int siz)
{
	int i;
	char str[50];
	int h;
	
	i = 0;
	
	do{
		sprintf(str,"%s/%d",path,i);
		printf("create %s\n",str);
		h = yaffs_open(str, O_CREAT | O_TRUNC | O_RDWR,S_IREAD | S_IWRITE);
		if(h >= 0)
		{
			yaffs_unlink(str);
		}
		i++;
	} while(h < 0 && i < maxIterations);
	
	if(h >= 0)
	{
		for(i = 0; i < siz; i++)
		{
			yaffs_write(h,str,1);
		}
	}
	
	printf("Leaving file %s open\n",str);

}

void dumpDirFollow(const char *dname)
{
	yaffs_DIR *d;
	yaffs_dirent *de;
	struct yaffs_stat s;
	char str[100];
			
	d = yaffs_opendir(dname);
	
	if(!d)
	{
		printf("opendir failed\n");
	}
	else
	{
		while((de = yaffs_readdir(d)) != NULL)
		{
			sprintf(str,"%s/%s",dname,de->d_name);
			
			yaffs_stat(str,&s);
			
			printf("%s length %d mode %X ",de->d_name,(int)s.st_size,s.st_mode);
			switch(s.st_mode & S_IFMT)
			{
				case S_IFREG: printf("data file"); break;
				case S_IFDIR: printf("directory"); break;
				case S_IFLNK: printf("symlink -->");
							  if(yaffs_readlink(str,str,100) < 0)
								printf("no alias");
							  else
								printf("\"%s\"",str);    
							  break;
				default: printf("unknown"); break;
			}
			
			printf("\n");           
		}
		
		yaffs_closedir(d);
	}
	printf("\n");
	
	printf("Free space in %s is %d\n\n",dname,(int)yaffs_freespace(dname));

}


void dumpDir(const char *dname)
{
	yaffs_DIR *d;
	yaffs_dirent *de;
	struct yaffs_stat s;
	char str[100];
			
	d = yaffs_opendir(dname);
	
	if(!d)
	{
		printf("opendir failed\n");
	}
	else
	{
		while((de = yaffs_readdir(d)) != NULL)
		{
			sprintf(str,"%s/%s",dname,de->d_name);
			
			yaffs_lstat(str,&s);
			
			printf("%s length %d mode %X ",de->d_name,(int)s.st_size,s.st_mode);
			switch(s.st_mode & S_IFMT)
			{
				case S_IFREG: printf("data file"); break;
				case S_IFDIR: printf("directory"); break;
				case S_IFLNK: printf("symlink -->");
							  if(yaffs_readlink(str,str,100) < 0)
								printf("no alias");
							  else
								printf("\"%s\"",str);    
							  break;
				default: printf("unknown"); break;
			}
			
			printf("\n");           
		}
		
		yaffs_closedir(d);
	}
	printf("\n");
	
	printf("Free space in %s is %d\n\n",dname,(int)yaffs_freespace(dname));

}


static void PermissionsCheck(const char *path, mode_t tmode, int tflags,int expectedResult)
{
	int fd;
	
	if(yaffs_chmod(path,tmode)< 0) printf("chmod failed\n");
	
	fd = yaffs_open(path,tflags,0);
	
	if((fd >= 0) != (expectedResult > 0))
	{
		printf("Permissions check %x %x %d failed\n",tmode,tflags,expectedResult);
	}
	else
	{
		printf("Permissions check %x %x %d OK\n",tmode,tflags,expectedResult);
	}
	
	
	yaffs_close(fd);
	
	
}

int long_test(int argc, char *argv[])
{

	int f;
	int r;
	char buffer[20];
	
	char str[100];
	
	int h;
	mode_t temp_mode;
	struct yaffs_stat ystat;
	
	yaffs_StartUp();
	
	yaffs_mount("/boot");
	yaffs_mount("/data");
	yaffs_mount("/flash");
	yaffs_mount("/ram");
	
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	printf("\nDirectory look-up of /data\n");
	dumpDir("/data");
	printf("\nDirectory look-up of /flash\n");
	dumpDir("/flash");

	//leave_unlinked_file("/flash",20000,0);
	//leave_unlinked_file("/data",20000,0);
	
	leave_unlinked_file("/ram",20,0);
	

	f = yaffs_open("/boot/b1", O_RDONLY,0);
	
	printf("open /boot/b1 readonly, f=%d\n",f);
	
	f = yaffs_open("/boot/b1", O_CREAT,S_IREAD | S_IWRITE);
	
	printf("open /boot/b1 O_CREAT, f=%d\n",f);
	
	
	r = yaffs_write(f,"hello",1);
	printf("write %d attempted to write to a read-only file\n",r);
	
	r = yaffs_close(f);
	
	printf("close %d\n",r);

	f = yaffs_open("/boot/b1", O_RDWR,0);
	
	printf("open /boot/b1 O_RDWR,f=%d\n",f);
	
	
	r = yaffs_write(f,"hello",2);
	printf("write %d attempted to write to a writeable file\n",r);
	r = yaffs_write(f,"world",3);
	printf("write %d attempted to write to a writeable file\n",r);
	
	r= yaffs_lseek(f,0,SEEK_END);
	printf("seek end %d\n",r);
	memset(buffer,0,20);
	r = yaffs_read(f,buffer,10);
	printf("read %d \"%s\"\n",r,buffer);
	r= yaffs_lseek(f,0,SEEK_SET);
	printf("seek set %d\n",r);
	memset(buffer,0,20);
	r = yaffs_read(f,buffer,10);
	printf("read %d \"%s\"\n",r,buffer);
	memset(buffer,0,20);
	r = yaffs_read(f,buffer,10);
	printf("read %d \"%s\"\n",r,buffer);

	// Check values reading at end.
	// A read past end of file should return 0 for 0 bytes read.
		
	r= yaffs_lseek(f,0,SEEK_END);
	r = yaffs_read(f,buffer,10);
	printf("read at end returned  %d\n",r); 
	r= yaffs_lseek(f,500,SEEK_END);
	r = yaffs_read(f,buffer,10);
	printf("read past end returned  %d\n",r);       
	
	r = yaffs_close(f);
	
	printf("close %d\n",r);
	
	copy_in_a_file("/boot/yyfile","xxx");
	
	// Create a file with a long name
	
	copy_in_a_file("/boot/file with a long name","xxx");
	
	
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");

	// Check stat
	r = yaffs_stat("/boot/file with a long name",&ystat);
	
	// Check rename
	
	r = yaffs_rename("/boot/file with a long name","/boot/r1");
	
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	
	// Check unlink
	r = yaffs_unlink("/boot/r1");
	
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");

	// Check mkdir
	
	r = yaffs_mkdir("/boot/directory1",0);
	
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	printf("\nDirectory look-up of /boot/directory1\n");
	dumpDir("/boot/directory1");

	// add a file to the directory                  
	copy_in_a_file("/boot/directory1/file with a long name","xxx");
	
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	printf("\nDirectory look-up of /boot/directory1\n");
	dumpDir("/boot/directory1");
	
	//  Attempt to delete directory (should fail)
	
	r = yaffs_rmdir("/boot/directory1");
	
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	printf("\nDirectory look-up of /boot/directory1\n");
	dumpDir("/boot/directory1");
	
	// Delete file first, then rmdir should work
	r = yaffs_unlink("/boot/directory1/file with a long name");
	r = yaffs_rmdir("/boot/directory1");
	
	
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	printf("\nDirectory look-up of /boot/directory1\n");
	dumpDir("/boot/directory1");

#if 0
	fill_disk_and_delete("/boot",20,20);
			
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
#endif

	yaffs_symlink("yyfile","/boot/slink");
	
	yaffs_readlink("/boot/slink",str,100);
	printf("symlink alias is %s\n",str);
	
	
	
	
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	printf("\nDirectory look-up of /boot (using stat instead of lstat)\n");
	dumpDirFollow("/boot");
	printf("\nDirectory look-up of /boot/directory1\n");
	dumpDir("/boot/directory1");

	h = yaffs_open("/boot/slink",O_RDWR,0);
	
	printf("file length is %d\n",(int)yaffs_lseek(h,0,SEEK_END));
	
	yaffs_close(h);
	
	yaffs_unlink("/boot/slink");

	
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	
	// Check chmod
	
	yaffs_stat("/boot/yyfile",&ystat);
	temp_mode = ystat.st_mode;
	
	yaffs_chmod("/boot/yyfile",0x55555);
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	
	yaffs_chmod("/boot/yyfile",temp_mode);
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	
	// Permission checks...
	PermissionsCheck("/boot/yyfile",0, O_WRONLY,0);
	PermissionsCheck("/boot/yyfile",0, O_RDONLY,0);
	PermissionsCheck("/boot/yyfile",0, O_RDWR,0);

	PermissionsCheck("/boot/yyfile",S_IREAD, O_WRONLY,0);
	PermissionsCheck("/boot/yyfile",S_IREAD, O_RDONLY,1);
	PermissionsCheck("/boot/yyfile",S_IREAD, O_RDWR,0);

	PermissionsCheck("/boot/yyfile",S_IWRITE, O_WRONLY,1);
	PermissionsCheck("/boot/yyfile",S_IWRITE, O_RDONLY,0);
	PermissionsCheck("/boot/yyfile",S_IWRITE, O_RDWR,0);
	
	PermissionsCheck("/boot/yyfile",S_IREAD | S_IWRITE, O_WRONLY,1);
	PermissionsCheck("/boot/yyfile",S_IREAD | S_IWRITE, O_RDONLY,1);
	PermissionsCheck("/boot/yyfile",S_IREAD | S_IWRITE, O_RDWR,1);

	yaffs_chmod("/boot/yyfile",temp_mode);
	
	//create a zero-length file and unlink it (test for scan bug)
	
	h = yaffs_open("/boot/zlf",O_CREAT | O_TRUNC | O_RDWR,0);
	yaffs_close(h);
	
	yaffs_unlink("/boot/zlf");
	
	
	yaffs_DumpDevStruct("/boot");
	
	fill_disk_and_delete("/boot",20,20);
	
	yaffs_DumpDevStruct("/boot");
	
	fill_files("/boot",1,10000,0);
	fill_files("/boot",1,10000,5000);
	fill_files("/boot",2,10000,0);
	fill_files("/boot",2,10000,5000);
	
	leave_unlinked_file("/data",20000,0);
	leave_unlinked_file("/data",20000,5000);
	leave_unlinked_file("/data",20000,5000);
	leave_unlinked_file("/data",20000,5000);
	leave_unlinked_file("/data",20000,5000);
	leave_unlinked_file("/data",20000,5000);
	
	yaffs_DumpDevStruct("/boot");
	yaffs_DumpDevStruct("/data");
	
		
		
	return 0;

}

int long_test_on_path(char *path)
{

	int f;
	int r;
	char buffer[20];
	
	char str[100];
	char name[100];
	char name2[100];
	
	int h;
	mode_t temp_mode;
	struct yaffs_stat ystat;
	
	yaffs_StartUp();
	
	yaffs_mount(path);
	
	printf("\nDirectory look-up of %s\n",path);
	dumpDir(path);

	//leave_unlinked_file("/flash",20000,0);
	//leave_unlinked_file("/data",20000,0);
	
	leave_unlinked_file(path,20,0);
	

	sprintf(name,"%s/%s",path,"b1");
	f = yaffs_open(name, O_RDONLY,0);
	
	printf("open %s readonly, f=%d\n",name,f);
	
	f = yaffs_open(name, O_CREAT,S_IREAD | S_IWRITE);
	
	printf("open %s O_CREAT, f=%d\n",name,f);
	
	
	r = yaffs_write(f,"hello",1);
	printf("write %d attempted to write to a read-only file\n",r);
	
	r = yaffs_close(f);
	
	printf("close %d\n",r);

	f = yaffs_open(name, O_RDWR,0);
	
	printf("open %s O_RDWR,f=%d\n",name,f);
	
	
	r = yaffs_write(f,"hello",2);
	printf("write %d attempted to write to a writeable file\n",r);
	r = yaffs_write(f,"world",3);
	printf("write %d attempted to write to a writeable file\n",r);
	
	r= yaffs_lseek(f,0,SEEK_END);
	printf("seek end %d\n",r);
	memset(buffer,0,20);
	r = yaffs_read(f,buffer,10);
	printf("read %d \"%s\"\n",r,buffer);
	r= yaffs_lseek(f,0,SEEK_SET);
	printf("seek set %d\n",r);
	memset(buffer,0,20);
	r = yaffs_read(f,buffer,10);
	printf("read %d \"%s\"\n",r,buffer);
	memset(buffer,0,20);
	r = yaffs_read(f,buffer,10);
	printf("read %d \"%s\"\n",r,buffer);

	// Check values reading at end.
	// A read past end of file should return 0 for 0 bytes read.
		
	r= yaffs_lseek(f,0,SEEK_END);
	r = yaffs_read(f,buffer,10);
	printf("read at end returned  %d\n",r); 
	r= yaffs_lseek(f,500,SEEK_END);
	r = yaffs_read(f,buffer,10);
	printf("read past end returned  %d\n",r);       
	
	r = yaffs_close(f);
	
	printf("close %d\n",r);
	
	sprintf(name,"%s/%s",path,"yyfile");
	copy_in_a_file(name,"xxx");
	
	// Create a file with a long name
	sprintf(name,"%s/%s",path,"file with a long name");
	copy_in_a_file(name,"xxx");
	
	
	printf("\nDirectory look-up of %s\n",path);
	dumpDir(path);

	// Check stat
	r = yaffs_stat(name,&ystat);
	
	// Check rename
	sprintf(name2,"%s/%s",path,"r1");
	r = yaffs_rename(name,name2);
	
	printf("\nDirectory look-up of %s\n",path);
	dumpDir(path);
	
	// Check unlink
	r = yaffs_unlink(name2);
	
	
	printf("\nDirectory look-up of %s\n",path);
	dumpDir(path);

	// Check mkdir

	sprintf(name,"%s/%s",path,"directory1");        
	r = yaffs_mkdir(name,0);
	
	
	printf("\nDirectory look-up of %s\n",path);
	dumpDir(path);
	printf("\nDirectory look-up of %s\n",name);
	dumpDir(name);

	// add a file to the directory                  
	sprintf(name2,"%s/%s",name,"/file in dir with a long name");
	copy_in_a_file(name2,"xxx");
	
	
	printf("\nDirectory look-up of %s\n",path);
	dumpDir(path);
	printf("\nDirectory look-up of %s\n",name);
	dumpDir(name);
	
	//  Attempt to delete directory (should fail)
	
	r = yaffs_rmdir(name);
	
	printf("\nDirectory look-up of %s\n",path);
	dumpDir(path);
	printf("\nDirectory look-up of %s\n",name);
	dumpDir(name);

	yaffs_unmount(path);

	return 0;       
	// Delete file first, then rmdir should work
	r = yaffs_unlink(name2);
	r = yaffs_rmdir(name);
	
	printf("\nDirectory look-up of %s\n",path);
	dumpDir(path);
	printf("\nDirectory look-up of %s\n",name);
	dumpDir(name);

#if 0
	fill_disk_and_delete(path,20,20);
	
	printf("\nDirectory look-up of %s\n",path);
	dumpDir(path);
#endif

	yaffs_unmount(path);

	return 0;

	yaffs_symlink("yyfile","/boot/slink");
	
	yaffs_readlink("/boot/slink",str,100);
	printf("symlink alias is %s\n",str);
	
	
	
	
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	printf("\nDirectory look-up of /boot (using stat instead of lstat)\n");
	dumpDirFollow("/boot");
	printf("\nDirectory look-up of /boot/directory1\n");
	dumpDir("/boot/directory1");

	h = yaffs_open("/boot/slink",O_RDWR,0);
	
	printf("file length is %d\n",yaffs_lseek(h,0,SEEK_END));
	
	yaffs_close(h);
	
	yaffs_unlink("/boot/slink");

	
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	
	// Check chmod
	
	yaffs_stat("/boot/yyfile",&ystat);
	temp_mode = ystat.st_mode;
	
	yaffs_chmod("/boot/yyfile",0x55555);
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	
	yaffs_chmod("/boot/yyfile",temp_mode);
	printf("\nDirectory look-up of /boot\n");
	dumpDir("/boot");
	
	// Permission checks...
	PermissionsCheck("/boot/yyfile",0, O_WRONLY,0);
	PermissionsCheck("/boot/yyfile",0, O_RDONLY,0);
	PermissionsCheck("/boot/yyfile",0, O_RDWR,0);

	PermissionsCheck("/boot/yyfile",S_IREAD, O_WRONLY,0);
	PermissionsCheck("/boot/yyfile",S_IREAD, O_RDONLY,1);
	PermissionsCheck("/boot/yyfile",S_IREAD, O_RDWR,0);

	PermissionsCheck("/boot/yyfile",S_IWRITE, O_WRONLY,1);
	PermissionsCheck("/boot/yyfile",S_IWRITE, O_RDONLY,0);
	PermissionsCheck("/boot/yyfile",S_IWRITE, O_RDWR,0);
	
	PermissionsCheck("/boot/yyfile",S_IREAD | S_IWRITE, O_WRONLY,1);
	PermissionsCheck("/boot/yyfile",S_IREAD | S_IWRITE, O_RDONLY,1);
	PermissionsCheck("/boot/yyfile",S_IREAD | S_IWRITE, O_RDWR,1);

	yaffs_chmod("/boot/yyfile",temp_mode);
	
	//create a zero-length file and unlink it (test for scan bug)
	
	h = yaffs_open("/boot/zlf",O_CREAT | O_TRUNC | O_RDWR,0);
	yaffs_close(h);
	
	yaffs_unlink("/boot/zlf");
	
	
	yaffs_DumpDevStruct("/boot");
	
	fill_disk_and_delete("/boot",20,20);
	
	yaffs_DumpDevStruct("/boot");
	
	fill_files("/boot",1,10000,0);
	fill_files("/boot",1,10000,5000);
	fill_files("/boot",2,10000,0);
	fill_files("/boot",2,10000,5000);
	
	leave_unlinked_file("/data",20000,0);
	leave_unlinked_file("/data",20000,5000);
	leave_unlinked_file("/data",20000,5000);
	leave_unlinked_file("/data",20000,5000);
	leave_unlinked_file("/data",20000,5000);
	leave_unlinked_file("/data",20000,5000);
	
	yaffs_DumpDevStruct("/boot");
	yaffs_DumpDevStruct("/data");
	

	return 0;

}

int yaffs_scan_test(const char *path)
{
}



int resize_stress_test(const char *path)
{
   int a,b,i,j;
   int x;
   int r;
   char aname[100];
   char bname[100];
   
   char abuffer[1000];
   char bbuffer[1000];
   
   yaffs_StartUp();
   
   yaffs_mount(path);
   
   sprintf(aname,"%s%s",path,"/a");
   sprintf(bname,"%s%s",path,"/b");
   
   memset(abuffer,'a',1000);
   memset(bbuffer,'b',1000);
   
   a = yaffs_open(aname, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
   b = yaffs_open(bname, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
   
   printf(" %s %d %s %d\n",aname,a,bname,b);
  
   x = 0;
   
   for(j = 0; j < 100; j++)
   {
		yaffs_lseek(a,0,SEEK_END);

		
		for(i = 0; i <20000; i++)
		{
		   //r =        yaffs_lseek(b,i,SEEK_SET);
			//r = yaffs_write(b,bbuffer,1000);
			
			if(x & 0x16)
			{
				// shrink
				int syz = yaffs_lseek(a,0,SEEK_END);
				
				syz -= 500;
				if(syz < 0) syz = 0;
				yaffs_truncate(a,syz);
				
			}
			else
			{
				//expand
				r = yaffs_lseek(a,i * 500,SEEK_SET);
				r = yaffs_write(a,abuffer,1000);
			}
			x++;
			
		}
   }
   
   return 0;
   
}


int resize_stress_test_no_grow_complex(const char *path,int iters)
{
   int a,b,i,j;
   int x;
   int r;
   char aname[100];
   char bname[100];
   
   char abuffer[1000];
   char bbuffer[1000];
   
   yaffs_StartUp();
   
   yaffs_mount(path);
   
   sprintf(aname,"%s%s",path,"/a");
   sprintf(bname,"%s%s",path,"/b");
   
   memset(abuffer,'a',1000);
   memset(bbuffer,'b',1000);
   
   a = yaffs_open(aname, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
   b = yaffs_open(bname, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
   
   printf(" %s %d %s %d\n",aname,a,bname,b);
  
   x = 0;
   
   for(j = 0; j < iters; j++)
   {
		yaffs_lseek(a,0,SEEK_END);

		
		for(i = 0; i <20000; i++)
		{
		   //r =        yaffs_lseek(b,i,SEEK_SET);
			//r = yaffs_write(b,bbuffer,1000);
			
			if(!(x%20))
			{
				// shrink
				int syz = yaffs_lseek(a,0,SEEK_END);
				
				while(syz > 4000)
				{
				
					syz -= 2050;
					if(syz < 0) syz = 0;
					yaffs_truncate(a,syz);
					syz = yaffs_lseek(a,0,SEEK_END);
					printf("shrink to %d\n",syz);
				}
				
				
			}
			else
			{
				//expand
				r = yaffs_lseek(a,500,SEEK_END);
				r = yaffs_write(a,abuffer,1000);
			}
			x++;
			
					
		}
		printf("file size is %d\n",yaffs_lseek(a,0,SEEK_END));

   }
   
   return 0;
   
}

int resize_stress_test_no_grow(const char *path,int iters)
{
   int a,b,i,j;
   int x;
   int r;
   char aname[100];
   char bname[100];
   
   char abuffer[1000];
   char bbuffer[1000];
   
   yaffs_StartUp();
   
   yaffs_mount(path);
   
   sprintf(aname,"%s%s",path,"/a");
   sprintf(bname,"%s%s",path,"/b");
   
   memset(abuffer,'a',1000);
   memset(bbuffer,'b',1000);
   
   a = yaffs_open(aname, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
   b = yaffs_open(bname, O_CREAT | O_TRUNC | O_RDWR, S_IREAD | S_IWRITE);
   
   printf(" %s %d %s %d\n",aname,a,bname,b);
  
   x = 0;
   
   for(j = 0; j < iters; j++)
   {
		yaffs_lseek(a,0,SEEK_END);

		
		for(i = 0; i <20000; i++)
		{
		   //r =        yaffs_lseek(b,i,SEEK_SET);
			//r = yaffs_write(b,bbuffer,1000);
			
			if(!(x%20))
			{
				// shrink
				int syz = yaffs_lseek(a,0,SEEK_END);
				
				while(syz > 4000)
				{
				
					syz -= 2050;
					if(syz < 0) syz = 0;
					yaffs_truncate(a,syz);
					syz = yaffs_lseek(a,0,SEEK_END);
					printf("shrink to %d\n",syz);
				}
				
				
			}
			else
			{
				//expand
				r = yaffs_lseek(a,-500,SEEK_END);
				r = yaffs_write(a,abuffer,1000);
			}
			x++;
			
					
		}
		printf("file size is %d\n",yaffs_lseek(a,0,SEEK_END));

   }
   
   return 0;
   
}

int directory_rename_test(void)
{
	int r;
	yaffs_StartUp();
	
	yaffs_mount("/ram");
	yaffs_mkdir("/ram/a",0);
	yaffs_mkdir("/ram/a/b",0);
	yaffs_mkdir("/ram/c",0);
	
	printf("\nDirectory look-up of /ram\n");
	dumpDir("/ram");
	dumpDir("/ram/a");
	dumpDir("/ram/a/b");

	printf("Do rename (should fail)\n");
		
	r = yaffs_rename("/ram/a","/ram/a/b/d");
	printf("\nDirectory look-up of /ram\n");
	dumpDir("/ram");
	dumpDir("/ram/a");
	dumpDir("/ram/a/b");

	printf("Do rename (should not fail)\n");
		
	r = yaffs_rename("/ram/c","/ram/a/b/d");
	printf("\nDirectory look-up of /ram\n");
	dumpDir("/ram");
	dumpDir("/ram/a");
	dumpDir("/ram/a/b");
	
	
	return 1;
	
}

int cache_read_test(void)
{
	int a,b,c;
	int i;
	int sizeOfFiles = 500000;
	char buffer[100];
	
	yaffs_StartUp();
	
	yaffs_mount("/boot");
	
	make_a_file("/boot/a",'a',sizeOfFiles);
	make_a_file("/boot/b",'b',sizeOfFiles);

	a = yaffs_open("/boot/a",O_RDONLY,0);
	b = yaffs_open("/boot/b",O_RDONLY,0);
	c = yaffs_open("/boot/c", O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);

	do{
		i = sizeOfFiles;
		if (i > 100) i = 100;
		sizeOfFiles  -= i;
		yaffs_read(a,buffer,i);
		yaffs_read(b,buffer,i);
		yaffs_write(c,buffer,i);
	} while(sizeOfFiles > 0);
	
	
	
	return 1;
	
}

int cache_bypass_bug_test(void)
{
	// This test reporoduces a bug whereby YAFFS caching *was* buypassed
	// resulting in erroneous reads after writes.
	// This bug has been fixed.
	
	int a;
	int i;
	char buffer1[1000];
	char buffer2[1000];
	
	memset(buffer1,0,sizeof(buffer1));
	memset(buffer2,0,sizeof(buffer2));
		
	yaffs_StartUp();
	
	yaffs_mount("/boot");
	
	// Create a file of 2000 bytes.
	make_a_file("/boot/a",'X',2000);

	a = yaffs_open("/boot/a",O_RDWR, S_IREAD | S_IWRITE);
	
	// Write a short sequence to the file.
	// This will go into the cache.
	yaffs_lseek(a,0,SEEK_SET);
	yaffs_write(a,"abcdefghijklmnopqrstuvwxyz",20); 

	// Read a short sequence from the file.
	// This will come from the cache.
	yaffs_lseek(a,0,SEEK_SET);
	yaffs_read(a,buffer1,30); 

	// Read a page size sequence from the file.
	yaffs_lseek(a,0,SEEK_SET);
	yaffs_read(a,buffer2,512); 
	
	printf("buffer 1 %s\n",buffer1);
	printf("buffer 2 %s\n",buffer2);
	
	if(strncmp(buffer1,buffer2,20))
	{
		printf("Cache bypass bug detected!!!!!\n");
	}
	
	
	return 1;
}


int free_space_check(void)
{
	int f;
	
		yaffs_StartUp();
		yaffs_mount("/boot");
	    fill_disk("/boot/",2);
	    f = yaffs_freespace("/boot");
	    
	    printf("%d free when disk full\n",f);           
	    return 1;
}

int truncate_test(void)
{
	int a;
	int r;
	int i;
	int l;

	char y[10];

	yaffs_StartUp();
	yaffs_mount("/boot");

	yaffs_unlink("/boot/trunctest");
	
	a = yaffs_open("/boot/trunctest", O_CREAT | O_TRUNC | O_RDWR,  S_IREAD | S_IWRITE);
	
	yaffs_write(a,"abcdefghijklmnopqrstuvwzyz",26);
	
	yaffs_truncate(a,3);
	l= yaffs_lseek(a,0,SEEK_END);
	
	printf("truncated length is %d\n",l);

	yaffs_lseek(a,5,SEEK_SET);
	yaffs_write(a,"1",1);

	yaffs_lseek(a,0,SEEK_SET);
	
	r = yaffs_read(a,y,10);

	printf("read %d bytes:",r);

	for(i = 0; i < r; i++) printf("[%02X]",y[i]);

	printf("\n");

	return 0;

}


int main(int argc, char *argv[])
{
	//return long_test(argc,argv);
	
	//return cache_read_test();
	
	//return resize_stress_test_no_grow("/flash",2);
	

	
	scan_pattern_test("/boot",40000,10);
	//short_scan_test("/flash",40000,200);
	return 0;
	
	long_test_on_path("/flash");
	long_test_on_path("/flash");
	
	// cache_bypass_bug_test();
	
	 free_space_check();
	 
	 return 0;
	
}