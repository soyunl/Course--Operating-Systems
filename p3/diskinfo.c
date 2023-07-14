#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


// prints OS name
void getOSName(char* p) {
    printf("OS NAME: ");
    for (int i=0; i<8; i++) {
        printf("%c",p[i+3]);
    }
}


// prints label of the disk
void getLabel (char* p, char* label) {
    int offset = 0x2600;
    printf("\nLabel of the disk: ");
    while (offset < 0x4200) { // 0x4200 == 33*512, where 33rd sector is a start of Data Area
        int attribute = p[offset+11];
        if (attribute==0x08) {
            for (int i=0; i<11; i++) {
                printf("%c",p[offset+i]);
            }
        }
        offset+=32;
    }
}


// prints total size of the disk
void getTotalSize(int size) {
    printf("\nTotal size of the disk: %d bytes",size);
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
void getFreeSize(char* p, int total_size) {
    int total_sector = total_size/512;
    int free_size,low_bits,high_bits,eight_bits,value = 0;
    printf("\nFree size of the disk: ");

    for (int i=2; i<total_sector-31; i++) {
        if (getFAT(p,i)==0x00) {
            free_size++;
        }
    }
    printf("%d bytes",free_size*512);
}


// prints # of files
void getNumFiles(char* p) {
    int offset = 0x2600;
    int num = 0;
    printf("\n\n==============\n");
    printf("The number of files in the image: ");

    // conditions to check if a directory entry needs to be skipped
    // then the # of skipped conditions will be subtracted from the total # of directories
    while (offset < 0x4200) {
        if (p[offset+11]==0x0F) { //11 = offset for attribute field
            num++;
        } else if ((p[offset+11] & 0x08)==1) {
            num++;
        } else if (p[offset+26]==0 || p[offset+26]==1) { // 26 = offset for first logical cluster field
            num++;
        } else if (p[offset]==0x00 || p[offset]==0xe5) { // offset itself points to the filename field
            num++;
        }
        offset+=32;
    }
    printf("%d",224-num); // 224=(# of sectors)*(# of directories in each sector)=14*16
}


// prints # of FAT copies
void getNumFAT(char* p) {
    printf("\n\n==============\n");
    printf("Number of FAT copies: %d",p[16]);
}

// prints sectors/FAT
void getSecFAT(char* p) {
    printf("\nSectors per FAT: %d\n",p[22]);
}


int main(int argc, char *argv[]) {  

	if (argc < 2) {
		printf("Error: invalid input\n");
	}

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
		
	getOSName(p);
    char label[12];
    getLabel(p,label);
    getTotalSize(totalSize);
    getFreeSize(p,totalSize);
    getNumFiles(p);
    getNumFAT(p);
    getSecFAT(p);

	munmap(p, sb.st_size); // the modifed the memory data would be mapped to the disk image
	close(fd);
	return 0;
}
