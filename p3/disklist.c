#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "linkedlist.h"


Node* head = NULL;


int getFileSize(char* p, int cur) {
    int offset = 28;
    int size[4];
    for (int i=0; i<4; i++) {
        size[i] = p[cur+offset+i];
    }
    size[0] = (size[0]&0xff);
    size[1] = ((size[1]&0xff)<<8);
    size[2] = ((size[2]&0xff)<<16);
    size[3] = ((size[3]&0xff)<<24);

    return size[0]+size[1]+size[2]+size[3];
}

// provided on course website
void getDateTime(char* p, int cur){
    int timeOffset = 14;
    int dateOffset = 16;
	int time, date;
	int hours, minutes, day, month, year;
	
	time = *(unsigned short *)(p+cur+timeOffset);
	date = *(unsigned short *)(p+cur+dateOffset);
	
	year = ((date & 0xFE00) >> 9) + 1980;
	month = (date & 0x1E0) >> 5;
	day = (date & 0x1F);
	printf("%d-%02d-%02d ", year, month, day);
    
	hours = (time & 0xF800) >> 11;
	minutes = (time & 0x7E0) >> 5;
	printf("%02d:%02d\n", hours, minutes);
    
    return ;	
}

// gets the first logical cluster from the respective directory entry
int getCluster (char* p, int cur) {
    int offset = 26;
    int cluster[2];
    for (int i=0; i<2; i++) {
        cluster[i] = p[cur+offset+i];
    }
    cluster[1] = (cluster[1]<<8);

    return cluster[0]+cluster[1];
}


// gets FAT entry using passed physical cluster entry
int getFAT(char* p, int entry) {
    int low_bits,eight_bits,high_bits,value;
    if (entry%2==0) { // for even # entry
        low_bits = p[512+1+(3*entry/2)] & 0x0f;
        eight_bits = p[512+(3*entry/2)] & 0xff;
        value = (low_bits << 8) + eight_bits;
    } else { // for odd # entry
        high_bits = p[512+(3*entry/2)] & 0xf0;
        eight_bits = p[512+1+(3*entry/2)] & 0xff;
        value = (high_bits >> 4) + (eight_bits << 4);
    }
    return value;
}

// prints contents of file
void getFile(char* p, int offset) {
    char type = 'F';
    char* filename = malloc(sizeof(char));
    char* extension = malloc(sizeof(char));
    int size = getFileSize(p,offset);
    // for filename
    for (int i=0; i<8; i++) {
        if (p[offset+i] == ' ') {
            break;
        }
        filename[i] = p[offset+i];
    }
    // for extension
    for (int i=0; i<3; i++) {
        if (p[offset+i+8]==' ') {
            break;
        }
        extension[i] = p[offset+i+8];
    }
    strcat(filename,".");
    strcat(filename,extension);
    printf("%c %10d %20s ",type,size,filename);
    getDateTime(p,offset);
}


// prints contents of sub-directory from Root directory
void getSubDirRoot(char* p, int offset) {
    char type = 'D';
    char* filename = malloc(sizeof(char));
    char* extension = malloc(sizeof(char));
    int size = 0;
    // for filename
    for (int i=0; i<8; i++) {
        if (p[offset+i] == ' ') {
            break;
        }
        filename[i] = p[offset+i];
    }
    // for extension
    for (int i=0; i<3; i++) {
        if (p[offset+i+8]==' ') {
            break;
        }
        extension[i] = p[offset+i+8];
    }

    if (strlen(extension)==0) {
        printf("%c %10d %20s ",type,size,filename);

    } else {
        strcat(filename,".");
        strcat(filename,extension);
        printf("%c %10d %20s ",type,size,filename);
    }
    getDateTime(p,offset);
    
}

// traverse through entries in root directory
void traverseDir(char* p, int offset) {
    int index,cluster,next_cluster = 0;
    cluster = getCluster(p,offset);
    next_cluster = getCluster(p,cluster);

   while (offset < 0x4200) {
        if (p[offset+11]!=0x0F && (getCluster(p,offset)!=0) && (getCluster(p,offset)!=1) && (p[offset+11]&0x08)!=1 && p[offset+26]!=0 && p[offset+26]!=1 && p[offset]!=0x00 && p[offset]!=0xe5) {
            if ((p[offset+11]&0x10)==0x10) { // check if it's a subdirectory
                cluster = getCluster(p,offset);
                head = add_newNode(head,cluster,offset,0);
                getSubDirRoot(p,offset);
            } else { // else it's a file
               getFile(p,offset);
            }
        }   
        offset+=32;
    }
    printf("\n");
}   


// prints heading of the each subdirectory
int printHeading(char* p, int offset, int* name, int count) {
    int parent = searchParent(head,offset);
    if (parent==0) {
        printf("/");
        for (int i=0; i<8; i++) {
            if (p[offset+i] == ' ') {
                break;
            }
            printf("%c",p[offset+i]);

        } if (count>=0) {
            for (int j=count-1; j>=0; j--) {
                printf("/");
                for (int i=0; i<8; i++) {
                    if (p[name[j]+i] == ' ') {
                        break;
                    }
                    printf("%c",p[name[j]+i]);
                }
            } 
        } 
        printf("\n==================\n");
        return 0;
    } else {
        name[count] = offset;
        count++;
        return printHeading(p,parent,name,count);
    }
}


// traverses through directories while directory's info is added to linked list
void traverseSubDir(char* p, Node* head) {
    int physical;
    int offset,cluster,parent;
    Node* cur = head;
    int* name;
    
    while (cur!=NULL) {        
        cluster = cur->first_cluster;
        offset = cur->offset;
        name = malloc(sizeof(int));
        printHeading(p,offset,name,0);
        parent = offset;
        offset = 512*(cluster+31);        

        for (int i=0; i<16; i++) {
            if (getCluster(p,offset)!=cluster && (getCluster(p,offset)!=0) && p[offset+11]!=0x0F  && (getCluster(p,offset)!=1) && (p[offset+11]&0x08)!=1 && p[offset+26]!=0 && p[offset+26]!=1 && p[offset]!=0x00 && p[offset]!=0xe5 && p[offset]!='.') {
                if (( p[offset+11]&0x10)==0x10 ) { // check if it's a subdirectory
                    cluster = getCluster(p,offset);
                    head = add_newNode(head,cluster,offset,parent);
                    getSubDirRoot(p,offset);
                } else {
                    getFile(p,offset);
                } 
            } offset+=32;   
        } cur = cur->next;
        printf("\n");
    }
}


int main(int argc, char *argv[]) {
	int fd;
	struct stat sb;

	fd = open(argv[1], O_RDWR);
	fstat(fd, &sb);

	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // p points to the starting pos of your mapped memory
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}

    int root_entry = 0x2600;
    printf("ROOT\n");
    printf("==================\n");
    traverseDir(p,root_entry);
    
    int offset,logical_cluster;
    Node* cur = head; 
    int subdir_root = numSubDir(head);
    traverseSubDir(p,head);

	munmap(p, sb.st_size); // the modifed the memory data would be mapped to the disk image
	close(fd);
	return 0;
}