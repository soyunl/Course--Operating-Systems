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


int checkFile (char* p, char* filename) {
    // converts the filename to uppercase
    // int size = sizeof(&filename) / sizeof(filename[0]);
    int size = strlen(filename);
    // printf("length of filename: %d\n",strlen(filename));
    for (int i=0; i<size; i++) {
        filename[i] = toupper(filename[i]);
    }
    
    // extracts filename and its extension
    int offset = 0x2600;
    char* file = malloc(sizeof(char));
    char* extension = malloc(sizeof(char));
    while (offset < 0x4200) {
        if (p[offset+11]!=0x0F && (p[offset+11]&0x08)!=1 && p[offset+26]!=0 && p[offset+26]!=1 && p[offset]!=0x00 && p[offset]!=0xe5) {
            if ((p[offset+11]&0x10)==0x10) { // check if it's a subdirectory
                offset = offset;
            } else {
                // for filename
                for (int i=0; i<8; i++) {
                    if (p[offset+i] == ' ') {
                        break;
                    }
                    file[i] = p[offset+i];
                }
                // for extension
                for (int i=0; i<3; i++) {
                    if (p[offset+i+8]==' ') {
                        break;
                    }
                    extension[i] = p[offset+i+8];
                }
                strcat(file,".");
                strcat(file,extension);
                int count = 0;
                for (int i=0; i<size; i++) {
                    if (filename[i]==file[i]) {
                        count++;
                    }
                }
                if (size==count) {
                    return offset;
                }
            }
        }
        offset+=32;
    }
    return 0;
}


// converts from little endian value
int getSize (char* p, int cur) {
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


// exports the passed filename from disk using pointer
void copyFile(char* p, char* filename, int filesize, int cur_cluster) {
    int physical_val,bytes_left;
    int next_cluster = getFAT(p,cur_cluster);
    FILE *fp = fopen(filename,"wb");

    while (next_cluster!=0xfff) {
        for (int i=0; i<512; i++) {
            physical_val = 512*(cur_cluster+31);
            fputc(p[physical_val+i],fp);
        }
        // update both cluster values
        cur_cluster = next_cluster;
        next_cluster = getFAT(p,cur_cluster);
    }

    bytes_left = filesize-(filesize/512)*512;
    physical_val = 512*(cur_cluster+31);
    for (int i=0; i<bytes_left; i++) {
        fputc(p[physical_val+i],fp);
    }
    fclose(fp);
}


int main(int argc, char *argv[])
{
	int fd;
	struct stat sb;

	fd = open(argv[1], O_RDWR);
	fstat(fd, &sb);

	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // p points to the starting pos of your mapped memory
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}

    
    // returns 0 if file does not exist, otherwise respective offset value is returned
    int check = checkFile(p,argv[2]);

    if (check==0) {
        printf("Error: file not found\n");
        exit(1);
    } else {
        int filesize = getSize(p,check);
        int first_cluster = p[26+check] + (p[27+check]<<8) & 0xff;
        copyFile(p,argv[2],filesize,first_cluster);
    }
    
	munmap(p, sb.st_size); // the modifed the memory data would be mapped to the disk image
	close(fd);
	return 0;
}
