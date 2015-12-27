#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct fat_struct
{
    int id;
    int childs[10];
} fat;

typedef struct meta_struct
{
    int id;
    char name[64];
    char path[1000];
    char content[1000];
    int isExists;
} meta;

typedef struct filesystem_struct 
{
    fat cells[64];
    meta files[64];
} filesystem;

filesystem fs;
char currentdir[64];

char* parseName(const char* path){
char str[100];
char sep [10]="/";
   // Переменная, в которую будут заноситься начальные адреса частей
   // строки str
   strcpy(str,path);
   char *istr;
   char *tmp;
   // Выделение первой части строки
   istr = strtok (str,sep);
   // Выделение последующих частей
   while (istr != NULL)
   {
      tmp=istr;
      // Выделение очередной части строки
      istr = strtok (NULL,sep);
   }
   return tmp;
}

static int freeChild(int num)
{
	int i=0;
	while(i<64)
	{
		if(fs.cells[num].childs[i]==0)
			return i;
		i++;
	}
	return -1;
}

static int freeFile()
{
    int i=0;
    while(i<64)
    {
        if(fs.files[i].isExists==0)
            return i;
        i++;
    }
    return -1;
}

static int seekFile(const char *path)
{
    int i=0;
    while(i<64)
    {
        if(strcmp(fs.files[i].path,path)==0)
            return i;
		i++;
    }
    return -1;
}

static void createFreeFs()
{
	meta file;
	fat cell;
	int i=0;
	while(i<64)
	{
		file.id=i;
		strcpy(file.name,"");
		strcpy(file.path,"");
		strcpy(file.content,"");
		file.isExists=0;
		cell.id=i;
		int j=0;
		while(j<10)
		{
			cell.childs[j]=0;
			j++;
		}
		fs.files[i]=file;
		i++;
	}
}

static int countChilds(int num)
{
    int count=0;
    int i=0;
    while(fs.cells[num].childs[i]!=0)
    {
        count++;
        i++;
    }
    return count;
}

static void writeStruct()
{
	FILE *f;
	f=fopen("home/denis/fuse/wstruct.txt","w");
	int i=0;
	int j=0;
	int count=0;
	while(j<64)
	{
		if(fs.files[j].isExists==1)
			count++;
		j++;
	}
	fprintf(f,"%d",count);
	fprintf(f,"\n");
	while(i<64)
	{
	if(fs.files[i].isExists==1){
	fprintf(f,"%d",fs.files[i].id);
	fprintf(f,"$*&");
	fprintf(f,fs.files[i].name);
	fprintf(f,"$*&");
	fprintf(f,fs.files[i].path);
	fprintf(f,"$*&");
	fprintf(f,fs.files[i].content);
	}
	i++;
	}
	fclose(f);
}

static void writeFat()
{
	FILE *f;
	f=fopen("home/denis/fuse/wfat.txt","w");	
	int i=0;
	int j=0;
	int count=0;
	while(i<64)
	{
		if(fs.cells[i].childs[0]!=0)
				count++;
		i++;
	}	
	fprintf(f,"%d",count);
        fprintf(f,"\n");
	i=0;
	while(i<64)
	{
		j=0;		
		if(fs.cells[i].childs[0]!=0)
		{		
			fprintf(f,"%d",i);			
			while(j<10)
			{
				if(fs.cells[i].childs[j]!=0)
				{
					fprintf(f,"$*&");
					fprintf(f,"%d",fs.cells[i].childs[j]);
				}
				j++;
			}
			fprintf(f,"\n");
		}
		i++;
	}
	fclose(f);
}

static void readStruct()
{
    meta file;        
    FILE *f;
	f=fopen("wstruct.txt","r");
    int length;
    char temp[1000];
    fgets(temp,1000,f);
    length=atoi(temp);
    int i=0;
    while(i<length)
    {
        char tmp[1000];
	    fgets(tmp,1000,f);
	    char sep[10]="$*&";
        char *istr;
	    int count;
	    istr=strtok(tmp,sep);
	    count=atoi(istr);
        file.id=count;
        int j=0;
        while(istr!=NULL)
        {
		    istr=strtok(NULL,sep);
            if(istr!=NULL)
		    {
                if(j==0)
				strcpy(file.name,istr);
                if(j==1)
                strcpy(file.path,istr);
                if(j==2)
                strcpy(file.content,istr);
			    j++;
		    }
	    }
        int temp1;
        temp1=freeFile();
        file.isExists=1;
        fs.files[temp1]=file;
        i++;            
    }  
    fclose(f); 
}

static void readFat()
{
	FILE *f;
	f=fopen("wfat.txt","r");
    int length;
    char temp[1000];
    fgets(temp,1000,f);
    length=atoi(temp);
    int i=0;
    while(i<length)
    {
        char tmp[1000];
	    fgets(tmp,1000,f);
	    char sep[10]="$*&";
        char *istr;
	    int id;
	    int temp;
	    char *temp1;
	    istr=strtok(tmp,sep);
	    id=atoi(istr);
        int j=0;
        while(j<10)
        {
		istr=strtok(NULL,sep);
		if(istr!=NULL){
		temp1=istr;
		temp=atoi(temp1);
		fs.cells[id].childs[j]=temp;}		
		    j++;
	}
        i++;            
    }  
    fclose(f); 
}

////////////////////////////////////////////////FUSE///////////////////////////////////////////////////////////////
static int my_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
    int j=seekFile(path);
    if(strcmp("\n",fs.files[j].content) == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;\
    }
    else if(strcmp(path,fs.files[j].path) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(fs.files[j].content);
    }
    else
        res = -ENOENT;
	return res;
}

static int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;
    int j=seekFile(path);
	if (strcmp("\n",fs.files[j].content) != 0)
		return -ENOENT;
    int count;
    count=countChilds(j);
	strcpy(currentdir,fs.files[j].path);
    int i=0;
    while(i<count)
    {
        int k=fs.cells[j].childs[i];	    
        filler(buf, fs.files[k].name, NULL, 0);
		i++;
    }
    return 0;
}

static int my_open(const char *path, struct fuse_file_info *fi)
{
	/*if (strcmp(path, hello_path) != 0)
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;
*/
	return 0;
}

static int my_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
    int j=seekFile(path);
	len = strlen(fs.files[j].content);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, fs.files[j].content + offset, size);
	} else
		size = 0;
	return size;
}

static int my_mkdir(const char *path, mode_t mode)
{
	meta file;
	char *name;
	char *newpath;
	name=parseName(path);
	int j=freeFile();
	file.id=j;
	strcpy(file.name,name);
	strcpy(file.path,path);
	strcpy(file.content,"\n");
	file.isExists=1;
	fs.files[j]=file;
	int i=seekFile(currentdir);
	int k=freeChild(i);
	fs.cells[i].childs[k]=j;
	return 0;
}

static int my_mknod(const char* path, mode_t mode, dev_t dev)
{
    meta file;
	char *name;
	char *newpath;
	name=parseName(path);
	int j=freeFile();
	file.id=j;
	strcpy(file.name,name);
	strcpy(file.path,path);
	strcpy(file.content,"--new file--\n");
	file.isExists=1;
	fs.files[j]=file;
	int i=seekFile(currentdir);
	int k=freeChild(i);
	fs.cells[i].childs[k]=j;
    return 0;
}

static void my_destroy(void* private_data)
{
	writeStruct();
	writeFat();
}

static struct fuse_operations my_oper;
 
int main(int argc, char *argv[])
{
	createFreeFs();    
	readStruct();
    readFat();
    my_oper.getattr = my_getattr;
    my_oper.readdir = my_readdir;
    my_oper.open = my_open;
    my_oper.read = my_read;
	my_oper.mkdir = my_mkdir;
	my_oper.destroy=my_destroy;
	my_oper.mknod=my_mknod;
    return fuse_main(argc, argv, &my_oper, 0);
}
