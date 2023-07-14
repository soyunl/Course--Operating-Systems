#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h> 
#include <time.h>
#include "linkedlist.h"


Node* head = NULL;


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

// returns the number of subdirectories from parsing the input by "/"
int parseInput(char* inputfile, int* fileinfo) {
	int length = strlen(inputfile);
	int count = 0;

	for (int i=0; i<length; i++) {
		if (inputfile[i]=='/') {
			fileinfo[count] = i;
			count++;
		}
	}
	return count;
}


char* checkFile(int* fileinfo, char* filename, char* input, int size) {
	int offset = fileinfo[size-1];
	int index = 0;

	for (int i=offset+1; i<strlen(input); i++) {
		filename[index] = input[i];
		index++;
	}
	return filename;

}

// concatenates subdirectories name
int getHeading(char* p, int offset, int* name, char* og_subdir, int index, int count) {
    int parent = searchParent(head,offset);
    if (parent==0) {
        og_subdir[index] = '/';
		index++;
        for (int i=0; i<8; i++) {
            if (p[offset+i] == ' ') {
                break;
                }
			og_subdir[index] = p[offset+i];
			index++;
        } if (count>=0) {

            for (int j=count-1; j>=0; j--) {
                og_subdir[index] = '/';
				index++;
                for (int i=0; i<8; i++) {
                    if (p[name[j]+i] == ' ') {
                        break;
                    }
					og_subdir[index] = p[name[j]+i];
					index++;
                }
            } 
        } 
        return 0;
        
    } else {
        name[count] = offset;
        count++;
        return getHeading(p,parent,name,og_subdir,index,count);
    }
}


void traverseRootDir(char* p, int offset) {
    int index = 0;
    int cluster = 0;
    int physical_cluster;
    int next_cluster;
    cluster = getCluster(p,offset);
    next_cluster = getCluster(p,cluster);
   while (offset < 0x4200) {
        if (p[offset+11]!=0x0F && (getCluster(p,offset)!=0) && (getCluster(p,offset)!=1) && (p[offset+11]&0x08)!=1 && p[offset+26]!=0 && p[offset+26]!=1 && p[offset]!=0x00 && p[offset]!=0xe5) {
			if ((p[offset+11]&0x10)==0x10) { // check if it's a subdirectory
                cluster = getCluster(p,offset);
                head = add_newNode(head,cluster,offset,0);
            } else { // else it's a file
               offset = offset;
            }
        }   
        offset+=32;
    }
    printf("\n");
}   

int traverseSubDir(char* p, Node* head, char* subdir) {
    int physical;
    int offset,cluster,parent;
    Node* cur = head;
    int* name;
	char* existing_subdir = malloc(sizeof(char));

    while (cur!=NULL) {        
        cluster = cur->first_cluster;
        offset = cur->offset;
        name = malloc(sizeof(int));
        getHeading(p,offset,name,existing_subdir,0,0);
		if (strcmp(existing_subdir,subdir)==0) {
			return 1;
		}
        parent = offset;
        offset = 512*(cluster+31);
        
    for (int i=0; i<16; i++) {
        if (getCluster(p,offset)!=cluster && (getCluster(p,offset)!=0) && p[offset+11]!=0x0F  && (getCluster(p,offset)!=1) && (p[offset+11]&0x08)!=1 && p[offset+26]!=0 && p[offset+26]!=1 && p[offset]!=0x00 && p[offset]!=0xe5 && p[offset]!='.') {
            if (( p[offset+11]&0x10)==0x10 ) { // check if it's a subdirectory
                cluster = getCluster(p,offset);
                head = add_newNode(head,cluster,offset,parent);
            } 
        } 
    offset+=32;   
    } 
    cur = cur->next;
    printf("\n");
    }
	return 0;
}

void checkSubDir(char* p, int* fileinfo, char* input, int size, char* subdir) {
	int starting_byte = 1;
	int delim_offset;
	int offset = fileinfo[size-1];

	for (int i=0; i<offset; i++) {
		subdir[i] = toupper(input[i]);
	}
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

// prints free size of the disk
int getFreeSize(char* p, int total_size) {
    int total_sector = total_size/512;
    int free_size,low_bits,high_bits,eight_bits,value = 0;

    for (int i=2; i<total_sector-31; i++) {
        if (getFAT(p,i)==0x00) {
            free_size++;
        }
    }
	return free_size*512;
}


void updateRoot(char* p, char* filename, int filesize, int empty_sector) {
	int offset = 0x2600;
	while (p[offset]!=0x00){
		offset+=32;
	}
	int delim = 0;
	for (int i=0; i<strlen(filename); i++) {
		if (filename[i]=='.') {
			delim = i;
		}
	}
	for (int i=0; i<delim; i++) {
		p[offset+i] = filename[i];
	}
	for (int j=0; j<strlen(filename)-delim; j++) {
		p[offset+8+j] = filename[delim+j];
	}
	p[offset+11] = 0x00;

}


void copyFile(char* p, char* filename, char* subdir, int filesize, int totalsize) {
	int total_sector = totalsize/512;
	int empty_sector;
	int count = 0;
	int cur_bytes = filesize;


	if (subdir==NULL) { // for cases with no subdirectory
		for (int i=2; i<total_sector-31; i++) {
			if (getFAT(p,i)==0x00) {
				empty_sector = i;
				count++;
			}
			
		}
		updateRoot(p,filename,filesize,empty_sector);
		
	} else { // for cases with subdirectories
		usleep(0);
	}
}


int main(int argc, char *argv[]) {
	int fd;
	struct stat sb;

	fd = open(argv[1], O_RDWR);
	fstat(fd, &sb);
	int totalSize;
	totalSize = (uint64_t)sb.st_size;

	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // p points to the starting pos of your mapped memory
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}

	int freeSize = getFreeSize(p,totalSize);

	int size = strlen(argv[2]);
	int* fileinfo = malloc(sizeof(int));
	char* filename = malloc(sizeof(char));
	int numSubDir = parseInput(argv[2],fileinfo);

	if (numSubDir>0) {	// input given with subdirectories
		filename = checkFile(fileinfo,filename,argv[2],numSubDir);
	} else { // input given without subdirectories
		filename = argv[2];
		
	}

	// checks if file exists in the current Linux directory
	if (access(filename,F_OK)==0) {
		filename = filename;
	} else {
		printf("File not found.\n");
		exit(1);
	}

	char* subdir;
	if (numSubDir>0) { // if subdirectories entered in input
		subdir = malloc(sizeof(char));
		checkSubDir(p,fileinfo,argv[2],numSubDir,subdir);
		int root_entry = 0x2600;
  	 	traverseRootDir(p,root_entry);

		int offset,logical_cluster;
		if (traverseSubDir(p,head,subdir)==0) {
			printf("The directory not found.\n");
			exit(1);
		}
	} else {
		subdir = NULL;
	}

	struct stat fp;
	stat(filename, &fp);
	int filesize = fp.st_size;

	if (freeSize >= filesize) {
		copyFile(p, filename, subdir, filesize, totalSize);
	} else {
		printf("Not enough free space in the disk image.\n");
		exit(1);
	}

	
	munmap(p, sb.st_size); // the modifed the memory data would be mapped to the disk image
	close(fd);

	return 0;
}
