#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <wiringPi.h>
#include "flasher.h"
#include <bcm2835.h>
#include <sched.h>
#include <errno.h>   // for errno
#include <limits.h>  // for INT_MAX
#include <stdlib.h>  // for strtol




const int rstPin = 17; //hd 11
const int lad0Pin = 22; //hd 15
const int lad1Pin = 23; //hd 16
const int lad2Pin = 24; //hd 18
const int lad3Pin = 25; //hd 22
const int lframePin = 27; //hd 13
const int lclkPin = 18; //hd 12
const int wrPin = 4; //hd 7
/*
#############
#SST49LF020A#
#############
          Address   Data
Manufacturer’s ID   0000H     BFH
Device ID           0001H     52H
###########################################################
Device Access   Address Range         Memory Size
Memory Access   FFFF FFFFH : FFC0 0000H   4 Mchar
Register Access FFBF FFFFH : FF80 0000H   4 Mchar
###########################################################
~Addressing~
--------------------------------------------------------------------
The ID[3:0] bits in the address field are inverse of the hardware strapping. The ID[3:0] correspond to address bits [A21:A18]
--------------------------------------------------------------------
Device #    Hardware    Strapping Address Bits Decoding
      Strapping ID[3:0]   A21 A20 A19 A18
0       0000        1   1   1   1
1       0001        1   1   1   0
.
.
15      1111        0   0   0   0
###########################################################
   A31: A23   b=A22           A21:A18     A17:A0
1111 1111 1b  1=Mem Access  ID[3:0]   Device Memory address
              0=Reg Access
Connections
Arduino   PLCC
A0(PC0) - LAD0
A1(PC1) - LAD1
A2(PC2) - LAD2
A3(PC3) - LAD3
A4(PC4) - LFRAME
9 (PB1) - CLK
TIDA=150 ns
*/











//Pm49FL002
//static unsigned long int base_mem = 0xFFFC0000;
//static unsigned long int base_jdec =0xFFBC0000;
//static unsigned long int size =0x40000;
//SST49LF020A
//static unsigned long int base_mem = 0xFFC00000;
//static unsigned long int base_jdec = 0xFF800000;
//static unsigned long int size =0x40000;
//static unsigned long int block_size=16 *1024;
//static unsigned long int blocks=16; //number of blocks










//SST49LF040
/*
The LPC address sequence is 32bits long. The
SST49LF040 will respond to addresses mapped into the
top of the 4GB  memory space from FFFF FFFFH to
FF00 0000H or bottom of the 4GB memory space from
00000 000H to 00FF FFFFFH. Address bits A18-A0 are
decoded as memory addresses for SST49LF040, A22-A19
are device ID strapping bits, A23 directs Reads and Writes
to memory locations A23 =  1) or to register access locations (A23 = 0).
 */
 
//The LAD[3:0] bus is latched every rising edge of the clock.
//On the cycle in which LFRAME# goes inactive, the last latched value is taken as the Start value
//The data transfer least significant nibble first and most significant nibble last.
//(i.e., DQ[3:0] on LAD[3:0] first, then DQ[7:4] on LAD[3:0] last.)
static unsigned int base_mem = 0xFF800000;
static unsigned int base_jdec = 0xFF040000;
static unsigned int size =0x80000;
static unsigned int block_size=64 *1024;
static unsigned int sector_size=4 *1024;
static unsigned int blocks=8; //number of blocks
static unsigned int sectors=128; //number of sectors
unsigned int mem = 0x00;
unsigned char sdata[4];
int mdata=0x0;
unsigned int flag=0;
int input=0x00;
unsigned int data=0x00;
int data_out[16];
unsigned int addr[4];
//File myFile;

void preparePinMode() {
    digitalWrite(rstPin, LOW);
    digitalWrite(lclkPin, HIGH);
    digitalWrite(lframePin, HIGH);
    digitalWrite(wrPin, LOW);
    set_lad_output();
    //pinMode(rstPin, OUTPUT);
    //pinMode(wrPin, OUTPUT);
    pinMode(lframePin, OUTPUT);
    pinMode(lclkPin, OUTPUT);
    usleep(2000);
    digitalWrite(rstPin, HIGH);
    usleep(1000);
}
void set_lad_output() {
    pinMode(lad0Pin, OUTPUT);
    pinMode(lad1Pin, OUTPUT);
    pinMode(lad2Pin, OUTPUT);
    pinMode(lad3Pin, OUTPUT);
}
void set_lad_input(){
    pinMode(lad0Pin, INPUT);
    pinMode(lad1Pin, INPUT);
    pinMode(lad2Pin, INPUT);
    pinMode(lad3Pin, INPUT);
}
void set_lad_tristate(){
	set_lad_input();
	//digitalWrite(lclkPin, LOW);
    //usleep(30);
	//digitalWrite(lclkPin, HIGH);
	//digitalWrite(lad0Pin, LOW);
	//digitalWrite(lad1Pin, LOW);
	//digitalWrite(lad2Pin, LOW);
	//digitalWrite(lad3Pin, LOW);
 }
void set_lad_data(unsigned int out){
	digitalWrite(lclkPin, LOW);
		if(out & 0b0001) {
        digitalWrite(lad0Pin, HIGH);
    } else {
        digitalWrite(lad0Pin, LOW);
    }
    if(out & 0b0010) {
        digitalWrite(lad1Pin, HIGH);
    } else {
        digitalWrite(lad1Pin, LOW);
    }
    if(out & 0b0100) {
        digitalWrite(lad2Pin, HIGH);
    } else {
        digitalWrite(lad2Pin, LOW);
    }
    if(out & 0b1000) {
        digitalWrite(lad3Pin, HIGH);
    } else {
        digitalWrite(lad3Pin, LOW);
    }
	usleep(30);
	digitalWrite(lclkPin, HIGH);
	//usleep(100);
	}
	
unsigned int read_lad(){
    digitalWrite(lclkPin, LOW);
    //usleep(100);
	digitalWrite(lclkPin, HIGH);
	usleep(10);
    unsigned int in=0;
	//debug("Input1");
    if(digitalRead(lad0Pin)){ in |= 0x01;}
	if(digitalRead(lad1Pin)){ in |= 0x02;}
    if(digitalRead(lad2Pin)){ in |= 0x04;}
    if(digitalRead(lad3Pin)){ in |= 0x08;}
	//debug("Input2");
    return in;
}

void toggle_lframe(){
	int cntr2=0;
	set_lad_output();
	set_lad_data(0b0000);
	digitalWrite(lframePin, LOW);
	digitalWrite(lclkPin, LOW);
	//usleep(30);
	digitalWrite(lclkPin, HIGH);
	digitalWrite(lframePin, HIGH);

}

void read_chip_id(){
    mem=base_jdec;
	int temp1=0;
    toggle_lframe();
    set_lad_data(0b0100);//CYCTYPE + DIR
    for(int i=1;i<=8;i++){
		set_lad_data(mem>>32-(4*i));}
	set_lad_data(0x0f);//TAR0
    set_lad_tristate();//TAR1

    if(read_lad()!=0){printf("Error In Sync at Cycle 13a\n");}//Sync
    data=read_lad();//Data ls nibble
    data|=read_lad()<<4;//Data ms nibble

    if(read_lad()!=15){printf("Error In Sync at Cycle 16a\n");}//Sync
    set_lad_output();
    set_lad_data(0b1111);//TAR0
    printf("Manufacturer ID: ");
    printf("%#x",data);printf("\n");

	
    mem=base_jdec+1;
    toggle_lframe();
    set_lad_data(0b0100);//CYCTYPE + DIR
    for(int i=1;i<=8;i++){
		set_lad_data(mem>>32-(4*i));}
	set_lad_data(0b1111);//TAR0
	set_lad_tristate();//TAR1

    if(read_lad()!=0x0){printf("Error In Sync at Cycle 13b\n");}
    data=read_lad();//Data ls nibble
    data|=read_lad()<<4;//Data ms nibble
    if(read_lad()!=0x0F){printf("Error In Sync at Cycle 16b\n");}//Sync
    set_lad_output();
    set_lad_data(0b1111);//TAR0
    printf("Device ID: ");
    printf("%#x",data);printf("\n");
	//read_lad();
  }

unsigned char read_address(unsigned int address){
    unsigned char rdata=0x00;
    address=address+base_mem;
    toggle_lframe();
    set_lad_data(0b0100);//CYCTYPE + DIR. “010Xb” indicates memory Read
    for(int i=1;i<=8;i++){
    set_lad_data(address>>32-4*i);}
	
    set_lad_data(0b1111);//TAR0
    set_lad_tristate();//TAR1
    if(read_lad()!=0x0){printf("Error In Sync at Cycle 13R\n"); }//Sync
    rdata=read_lad();//Data ls nibble
    rdata|=read_lad()<<4;//Data ms nibble
    if(read_lad()!=0x0F){printf("Error In Sync at Cycle 16R\n"); }//Sync
	//read_lad();
    return rdata;  
  }

unsigned char read_address_t(unsigned int address){
    unsigned char rdata=0x00;
    address=address+base_mem;
    toggle_lframe();
    set_lad_data(0b0100);//CYCTYPE + DIR. “010Xb” indicates memory Read
    for(int i=1;i<=8;i++){
    set_lad_data(address>>32-4*i);}
	
    set_lad_data(0b1111);//TAR0
    set_lad_tristate();//TAR1
    if(read_lad()!=0x0){printf("Toggle Error In Sync at Cycle 13R\n"); }//Sync
    rdata=read_lad();//Data ls nibble
    rdata|=read_lad()<<4;//Data ms nibble
    if(read_lad()!=0x0F){printf("Toggle Error In Sync at Cycle 16R\n"); }//Sync
	read_lad();
    return rdata;  
  }

unsigned char read_gpi(){
    unsigned char rdata=0x00;
    unsigned int address=base_jdec+0x0100;
    toggle_lframe();
    set_lad_data(0b0100);//CYCTYPE + DIR. “010Xb” indicates memory Read
    for(int i=1;i<=8;i++){
    set_lad_data(address>>32-4*i);}
	
    set_lad_data(0b1111);//TAR0
    set_lad_tristate();//TAR1
    if(read_lad()!=0x0){printf("Toggle Error In Sync at Cycle 13R\n"); }//Sync
    rdata=read_lad();//Data ls nibble
    rdata|=read_lad()<<4;//Data ms nibble
    if(read_lad()!=0x0F){printf("Toggle Error In Sync at Cycle 16R\n"); }//Sync
	read_lad();
    return rdata;  
  }

void read_flash_2screen(){
    unsigned int i=0;
    while(i<=size-1){
      if(i % 16==0 && i>0){
        for(int j=0;j<=15;j++){
          sprintf(sdata,"%02x ",data_out[j]);
          printf(sdata);
        }
        printf("\n");
     }
      data_out[(i % 16)]=read_address(i);
      i++;
    }
  }
void read_flash(char * buffer, size_t flash_size){
	size_t i=0;
    while(i<=flash_size-1){
		buffer[i]=read_address(i);
		i++;
	}
	//for(unsigned int j=0; j<512*1024;j++){printf("%#x ",buffer[j]);if(j % 16==0 && j>0){printf("\n");}}
}
void save_flash(char *file_name, size_t flash_size){
	unsigned char *buffer; 
	buffer = (char *)malloc(flash_size);
	
	read_flash(buffer, flash_size);
	int written = 0;
	FILE *f = fopen(file_name, "wb");
	written = fwrite(buffer, sizeof(char), flash_size/sizeof(char), f);
	if (written == 0) {printf("Error during writing to file !");}
	if(!fclose(f))printf("File %s saved successfully.\n",file_name);
	else printf("Could not save file!\n");
	free(buffer);
}

void flash_bios(unsigned char *file_name){
	if( access( file_name, F_OK ) != -1) {
		FILE *fileptr;
		char *buffer;
		fileptr = fopen(file_name, "rb");  // Open the file in binary mode
		fseek(fileptr, 0, SEEK_END);      // Jump to the end of the file
		size_t filelen = ftell(fileptr);  // Get the current byte offset in the file
		rewind(fileptr);                  // Jump back to the beginning of the file
		
		buffer = (char *)malloc(filelen * sizeof(char)); // Enough memory for the file
		fread(buffer, filelen, 1, fileptr); // Read in the entire file
		fclose(fileptr); // Close the file
		printf("File %s is %u bytes.\n",file_name,filelen);
		for(size_t i=0;i<filelen;i++){
			program_address(i,buffer[i]);
		}
		free(buffer);
		// file exists
	} else {
		printf("Can't open %s. Aborting \n",file_name);
}

	
}

//Reads from address given as argument till the end of space
void read_flash_from2end(unsigned long i){
    unsigned long f=i;
    while(i<=size-1){
      if(i % 16==0 && i>f){
        //cli();
        for(int j=0;j<=15;j++){
          sprintf(sdata,"%02x ",data_out[j]);
          printf(sdata);
        }
        printf("\n");
        //sei();
     }
      data_out[(i % 16)]=read_address(i);
      i++;
    }
  }
  
//Reads from address i till address size
void read_flash_from2length(unsigned long i, unsigned long size){
    unsigned long f=i;
    while(i<size-1){
      if(i % 16==0 && i>f){
        //cli();
        for(char j=0;j<=15;j++){
          sprintf(sdata,"%02x ",data_out[j]);
          printf(sdata);
        }
        printf("\n");
        //sei();
     }
      data_out[(i % 16)]=read_address(i);
      i++;
    }
  }


bool toggle_d6(){
  char temp=0x0;
  char cnt=0;
  char temp2=0;
  temp=read_address_t(base_mem) & 0x40;
  while(cnt<100){
      usleep(8000);
      temp2=read_address_t(base_mem) & 0x40;
	  cnt++;
      if(temp==temp2){return false;}
    }return true;
  }
    
int write_address(unsigned int address,  unsigned char wdata){
    address=base_mem+address;
    toggle_lframe();
    set_lad_data(0b0111);//CYCTYPE + DIR write command.  “011xb” indicates memory Write cycle
    for(int i=1;i<=8;i++){
		set_lad_data(address>>32-4*i);}
    
    set_lad_data(wdata);//ls nibble first
    set_lad_data(wdata>>4);//ms nibble last
    set_lad_data(0b1111);//TAR0
    set_lad_tristate();//TAR1
	usleep(1000);
    if(read_lad()!=0x00){printf("Error In Sync at WCycle 13\n");}//Sync
    if(read_lad()!=0x0F){printf("Error In Sync at WCycle 16\n");}//Sync
    set_lad_tristate();//TAR1 
    set_lad_output();
    set_lad_data(0b1111);//TAR0
	//read_lad();
     
  }
  
void soft_id(){
    write_address(0x5555, 0xAA);
    write_address(0x2AAA, 0x55);
    write_address(0x5555, 0x90);
    data=read_address(0x0000);
    printf("Manufacturer ID: %#x\n",data);
    data=read_address(0x0001);
    printf("Device ID: %#x\n",data);
    write_address(0x5555, 0xF0);
    if(toggle_d6()) printf("Unknown Error Occured");
    }
    
void block_erase(int block){
    unsigned long block_addr=block*block_size;//calculation of block base address(block_address=block_number*block_size)
    write_address(0x5555, 0xAA);
    write_address(0x2AAA, 0x55);
    write_address(0x5555, 0x80);
    write_address(0x5555, 0xAA);
    write_address(0x2AAA, 0x55);
    write_address(block_addr, 0x50);
    if(!toggle_d6()){
    printf("Block %d Erased.",block);}
    else printf("Unknown Error Occured");
  }
void sector_erase(int sector){
    unsigned long sector_addr=sector*sector_size;//calculation of sector base address(sector_address=sector_number*sector_size)
    write_address(0x5555, 0xAA);
    write_address(0x2AAA, 0x55);
    write_address(0x5555, 0x80);
    write_address(0x5555, 0xAA);
    write_address(0x2AAA, 0x55);
    write_address(sector_addr, 0x30);
    //usleep(15000);
	//check_gpi();

    if(!toggle_d6()){
    printf("Sector %d Erased.\n",sector);}
    else printf("Unknown Error Occured\n");
  }
bool check_gpi(){
	int cntr3=0;
	unsigned char temp4;
	while(cntr3++<100){
		temp4=read_gpi();
		if(temp4!=0x1f)usleep(10);
		else return 0;
		}
	return 1;
		
}

void chip_erase(){
  for(int i=0;i<blocks;i++){
    block_erase(i);
    }
}

void chip_erase_sector(){
  for(int i=0;i<sectors;i++){
	//usleep(150000);
    sector_erase(i);
    }
}
void program_address(unsigned int address, unsigned char bdata){
    write_address(0x5555, 0xAA);
    write_address(0x2AAA, 0x55);
    write_address(0x5555, 0xA0);
    write_address(address, bdata);
    if(toggle_d6()) printf("Unknown Error Occured\n");
    //else println("Address Programed ");
  }

int main( int argc, char **argv ) {
	wiringPiSetupGpio();
	preparePinMode();
	const struct sched_param priority = {1};
	sched_setscheduler(0, SCHED_FIFO, &priority);
	//printf("GPI: %#x\n",read_gpi());
    //chip_erase();
    //chip_erase_sector();
    //read_chip_id();
    //soft_id();
	//chip_erase_sector();
    //block_erase(7);
    //block_erase(7);
    //read_flash_2screen();
    //sector_erase(7);
    //flash_bios("nf7.bin");
    //delay(100);
    //read_flash();
	//save_flash("bios2.bin",size);
    //test_bios(0x70000);
    //read_bios_addr();
	int i = 1;
	char *p;
	errno = 0;
    if(argc == 1) {
        i = 0;
    }
    while(i < argc) {
		if((strcmp(argv[i], "-w") == 0) &&  (i+1 < argc)) {
            flash_bios(argv[i+1]);
        }
        else if(strcmp(argv[i], "-r") == 0){
			if(i+1 < argc) {
				size_t start=strtol(argv[i+1], NULL, 10);;
				save_flash(argv[i+i], start);}
			else if(i+2 > argc) {
				printf("Saving flash to file %s\n", argv[i+1]);
				save_flash(argv[++i],size);
			}
        }
		  
        else if((strcmp(argv[i], "-es") == 0)) {
            chip_erase_sector();
        }
        else if((strcmp(argv[i], "-eb") == 0)) {
            chip_erase();
        }
		else if((strcmp(argv[i], "-sec") == 0) &&  (i+1 < argc)){
			long sec=strtol(argv[i+1], &p, 10);
			if (errno != 0 || *p != '\0' || sec > INT_MAX) {printf("Sector number error: %d\n",sec);exit(1);}
			else{
				printf("Erasing sector %d\n",sec);
				sector_erase(sec);}
        }
		else if((strcmp(argv[i], "-fs") == 0)) {
			read_flash_2screen();}
		else if((strcmp(argv[i], "-id") == 0)) {
			read_chip_id();}
		else if((strcmp(argv[i], "-jid") == 0)) {
			soft_id();}
        else {
            printf("LPC PLCC Flash Programmer\n");
            printf("Usage: %s -[switch] [parameter]\n", argv[0]);
            printf("Switch:\n");
            printf(" -w   [filename]      Write to flash (flash should be erased)\n");
            printf(" -es                  Erase flash sector by sector\n");
			printf(" -eb                  Erase flash block by block\n");
			printf(" -sec [Sector Number] Erase specific sector\n");
            printf(" -r   [filename]      Save the flash to file\n");
			printf(" -id                  Read the Chip ID\n");
			printf(" -jid                 Read the JDEC Chip ID\n");
			printf(" -fs                  Read the flash to screen\n");

            exit(0);
        }
        i++;
    }

}