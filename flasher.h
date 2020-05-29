void println(char * str);
void program_address(unsigned int address, unsigned char bdata);
void chip_erase_sector();
void chip_erase();
void sector_erase(int sector);
void block_erase(int block);
void soft_id();
int write_address(unsigned int address,  unsigned char wdata);
bool toggle_d6();
unsigned char read_address(unsigned int address);
void read_chip_id();
void read_flash_from2length(unsigned long i, unsigned long size);
void read_flash_from2end(unsigned long i);
//void read_flash();
void toggle_lframe();
unsigned int read_lad();
void set_lad_data(unsigned int out);
void set_lad_tristate();
void set_lad_input();
void set_lad_output();
void preparePinMode();
void save_flash(char * file_name, size_t flash_size);
void read_flash(char * buffer, size_t flash_size);
unsigned char read_address_t(unsigned int address);
unsigned char read_gpi();
bool check_gpi();
void flash_bios(unsigned char * file_name);



//void alarmWakeup(int sig_num);
//void wait_clock_neg();



//Reads from address given as argument till the end of space
