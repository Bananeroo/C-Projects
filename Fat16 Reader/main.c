#include <stdio.h>
#include <stdlib.h>
#include  <string.h>
#define BPS 512
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <ctype.h>

typedef uint64_t lba_t;
lba_t volume_start;
int16_t bytes_per_sector;
uint8_t sectors_per_cluster;
uint16_t reserved_sectors;
uint8_t fat_count;
uint16_t sectors_per_fat;
uint16_t root_dir_capacity;
uint16_t logical_sectors16;
uint32_t logical_sectors32;
uint32_t current_dir=0;
FILE* fvolume;
uint8_t br[BPS*2];

size_t readblock(void* buffer, uint32_t first_block, size_t block_count){
	
	if(fvolume==NULL)
	{
			
		fvolume =fopen("fat16.bin", "rb");
		if(fvolume == NULL)return 0;
		
		fseek(fvolume,0, SEEK_SET);
		int sread = fread(buffer, BPS, 1, fvolume);
		if(sread==0) return 0;
						
		bytes_per_sector=(br[0x0C]<<8)+ br[0x0B];
		sectors_per_cluster=br[0x0D];
		reserved_sectors=(br[0x0F]<<8)+ br[0x0E];
		fat_count=br[0x10];
		sectors_per_fat=(br[0x17]<<8)+br[0x16];
		root_dir_capacity=(br[0x12]<<8)+ br[0x11];
		logical_sectors16=(br[0x14]<<8)+ br[0x13];
		if(br[0x13]==0)
		logical_sectors32=(br[0x23]<<24)+(br[0x22]<<16)+(br[0x21]<<8)+br[0x20];
		else logical_sectors32=0;
	return 1;	
	}
	
	if(buffer==NULL || first_block>=logical_sectors16 || first_block<=0 
		|| block_count<=0 || first_block+block_count>=logical_sectors16) return 0;
	
			fseek(fvolume,first_block*BPS, SEEK_SET);
			int sread = fread(buffer, BPS, block_count, fvolume);
			
	
	return sread;
	}

void dir(){
	if(current_dir==0){
			int i=0;
			while(i<32)
			{
				readblock(br,  reserved_sectors+sectors_per_fat*fat_count+i,1);
				int j=0;
				while(j<16)
				{	
					if(br[j*32]==0x00 || br[j*32]==0xE5 ||br[j*32]==0x2E || br[j*32+11]==0x0F ) {j++; continue;}
					int k=0;
					uint8_t tmp,first;
					tmp=br[j*32+17]<<7;
					tmp=tmp>>4;
					first=tmp+(br[j*32+16]>>5);
					tmp=br[j*32+16]<<3;
					tmp=tmp>>3;
					printf("%02d/%02d/%04d",tmp,first,(br[j*32+17]>>1)+1980);
					
					tmp=(br[j*32+15]<<5);
					tmp=tmp>>2;
					first=tmp+(br[j*32+14]>>5);
					printf(" %02d:%02d ",(br[j*32+15]>>3)+2, first);
					
					if(br[j*32+11]==0x10) printf(" <DIRECTORY> ");
					else printf("%12d ",(br[j*32+31]<<24)+(br[j*32+30]<<16)+(br[j*32+29]<<8)+br[j*32+28]);
					int z=0;
					for(z=0;z<11;z++)
					{
						if(z==8&&br[j*32+11]!=0x10 ) printf(".");
						if(br[j*32+z]==' ')continue;
						printf("%c",br[j*32+z]);
					}
					printf("\n");
					k++;j++;
				}			
				i++;
			}
		}		
		else
		{
			int tmp=current_dir;
			while(1){
			int i=0;
			while(i<2)
			{
				readblock(br,  (reserved_sectors+sectors_per_fat*fat_count+i+32+(current_dir-2)*2),1);
				int j=0;
				while(j<16)
				{	
					if(br[j*32]==0x00 || br[j*32]==0xE5 ||br[j*32]==0x2E || br[j*32+11]==0x0F ) {j++; continue;}
					int k=0;
					uint8_t tmp,first;
					tmp=br[j*32+17]<<7;
					tmp=tmp>>4;
					first=tmp+(br[j*32+16]>>5);
					tmp=br[j*32+16]<<3;
					tmp=tmp>>3;
					printf("%02d/%02d/%04d",tmp,first,(br[j*32+17]>>1)+1980);
					
					tmp=(br[j*32+15]<<5);
					tmp=tmp>>2;
					first=tmp+(br[j*32+14]>>5);
					printf(" %02d:%02d ",(br[j*32+15]>>3)+2, first);
					
					if(br[j*32+11]==0x10) printf(" <DIRECTORY> ");
					else printf("%12d ",(br[j*32+31]<<24)+(br[j*32+30]<<16)+(br[j*32+29]<<8)+br[j*32+28]);
					int z=0;
					for(z=0;z<11;z++)
					{
						if(z==8&&br[j*32+11]!=0x10 ) printf(".");
						if(br[j*32+z]==' ')continue;
						printf("%c",br[j*32+z]);
					}
					printf("\n");
					k++;j++;
				}			
				i++;
			}
			readblock(br,(reserved_sectors+(current_dir)/(BPS/2)),1);
			
			if(br[(current_dir%(BPS/2))*2] ==0xFF &&    br[(current_dir%(BPS/2))*2+1]== 0xFF) break;
			else current_dir=(br[(current_dir%(BPS/2))*2+1]<<8)+br[(current_dir%(BPS/2))*2];
		}
		current_dir=tmp;
		}
	}
	
void cd(char *a){
	
a=a+3;
int e=0;
while(*(a+e)!='\0') e++;
if(current_dir==0){
			int i=0;
			while(i<32)
			{
				readblock(br,  reserved_sectors+sectors_per_fat*fat_count+i,1);
				int j=0;
				while(j<16)
				{	
					if(br[j*32]==0x00 || br[j*32]==0xE5 || br[j*32+11]==0x0F || br[j*32+11]!=0x10 ) {j++; continue;}
					int k=0;
					while(k<e){
					if (toupper(*(a+k)) != br[j*32+k] && tolower(*(a+k)) != br[j*32+k]  ){k=-1;j++;break;}	
					k++;	
					}
					if( k==-1 || ((k<8) && br[j*32+k]!=0x20  )){j++;continue;}
					current_dir=(br[j*32+27]<<8)+br[j*32+26];
					return ;
				}	
				
				i++;
			}
		}
		else{
			int tmp=current_dir;
			while(1){
			int i=0;
			while(i<2)
			{
				readblock(br, (reserved_sectors+sectors_per_fat*fat_count+i+32+(current_dir-2)*2),1);
			
				int j=0;
				
					while(j<16)
				{	
						if(br[j*32]==0x00 || br[j*32]==0xE5 || br[j*32+11]==0x0F  ) {j++; continue;}
					int k=0;
					while(k<e){
					if (toupper(*(a+k)) != br[j*32+k] && tolower(*(a+k)) != br[j*32+k]  ){k=-1;break;}	
					k++;	
					}
					if( k==-1 || ((k<8) && br[j*32+k]!=0x20  )){j++;continue;}
					current_dir=(br[j*32+27]<<8)+br[j*32+26];
					return ;
				j++;
				}	
				i++;
			}
			readblock(br,  (reserved_sectors+(current_dir)/(BPS/2)),1);
			
			
			if(br[(current_dir%(BPS/2))*2] ==0xFF &&    br[(current_dir%(BPS/2))*2+1]== 0xFF) break;
			else current_dir=(br[(current_dir%(BPS/2))*2+1]<<8)+br[(current_dir%(BPS/2))*2];

		}
		current_dir=tmp;
		}
	
	printf("Nie ma takiego folderu lub jestes juz w katalogu nadrzednym\n");
	
	}

void pwd(uint32_t receive){

	uint32_t wysylanko;
	uint32_t tmp=current_dir;
	if(current_dir==0) printf("Twój aktualny katalog to: \\");
	else{
	readblock(br, (reserved_sectors+sectors_per_fat*fat_count+32+(current_dir-2)*2),1);
	
	wysylanko=(br[27]<<8)+br[26];
	current_dir=(br[27+32]<<8)+br[26+32];
	pwd(wysylanko);
			current_dir=tmp;	
}
if(receive>0)
{
	if(current_dir==0)
	{
			int i=0;
			while(i<32)
			{
				
				readblock(br, (reserved_sectors+sectors_per_fat*fat_count+i),1);
				int j=0;
				while(j<16)
				{	
						int tmp1=(br[27+j*32]<<8)+br[26+j*32];
					if (receive ==tmp1){
						int g=0;
						while(g<10)
						{
							if(br[g+j*32]!=' ')	printf("%c",br[g+j*32]);
							g++;
							}
						}
						j++;
				}			
				i++;
			}
		
		
		}

else{
		int i=0;
	while(i<2)
			{
				readblock(br, (reserved_sectors+sectors_per_fat*fat_count+32+i+(current_dir-2)*2),1);
				int j=0;
				
					while(j<16)
				{	
						uint32_t tmp1=(br[27+j*32]<<8)+br[26+j*32];
					if (receive ==tmp1){
						int g=0;
						while(g<10)
						{
							if(br[g+j*32]!=' ')	printf("%c",br[g+j*32]);
							g++;
							}
						}
						
					
				j++;
				}	
				i++;
			}
			
	}
}

if(receive!=0)printf("\\");
else printf("\n");
current_dir=tmp;	
}

void read_file(uint16_t start)
{
	if(start<3) return;
	while(1){
		int i=0;
		while(i<2)
			{
				readblock(br,(reserved_sectors+sectors_per_fat*fat_count+i+32+(start-2)*2),1);
				int j=0;
				
					while(j<16)
				{	
					int k=0;
					while(k<32){
					if(br[j*32+k]==0){printf("\n"); return;}
					printf("%c",br[j*32+k]);k++;}
					
					
				j++;
				}	
				i++;
			}
		readblock(br,(reserved_sectors+(start)/(BPS/2)),1);
				

			
			if(br[(start%(BPS/2))*2] ==0xFF &&    br[(start%(BPS/2))*2+1]== 0xFF) {printf("\n");return;}
			else start=(br[(start%(BPS/2))*2+1]<<8)+br[(start%(BPS/2))*2];
	}	
}

void cat(char *a)
{
	
a=a+4;
int e=0;
while(*(a+e)!='\0') e++;
e=e-4;
if(current_dir==0){
			int i=0;
			while(i<32)
			{
				readblock(br,(reserved_sectors+sectors_per_fat*fat_count+i),1);
				int j=0;
				while(j<16)
				{	
					if(br[j*32]==0x00 || br[j*32]==0xE5 || br[j*32+11]==0x0F ) {j++; continue;}
					int k=0;
					while(k<e){
					if (toupper(*(a+k)) != br[j*32+k] && tolower(*(a+k)) != br[j*32+k]  ){k=-1;j++;break;}	
					k++;	
					}
					if( k==-1 || ((k<8) && br[j*32+k]!=0x20  )){j++;continue;}
					k=8;
					e++;
					while(k<11)
					{
					if (toupper(*(a+e)) != br[j*32+k] && tolower(*(a+e)) != br[j*32+k]  ){k=-1;j++;break;}	
						e++;
						k++;
					}
					if( k==-1 ){j++;continue;}
					
					read_file((br[j*32+27]<<8)+br[j*32+26]);
					return ;
				}	
				
				i++;
			}
		}
		else{
			int tmp=current_dir;
			while(1){
			int i=0;
			while(i<2)
			{
				readblock(br,(reserved_sectors+sectors_per_fat*fat_count+i+32+(current_dir-2)*2),1);
				int j=0;
				
					while(j<16)
				{	
						if(br[j*32]==0x00 || br[j*32]==0xE5 || br[j*32+11]==0x0F  ) {j++; continue;}
					int k=0;
					while(k<e){
					if (toupper(*(a+k)) != br[j*32+k] && tolower(*(a+k)) != br[j*32+k]  ){k=-1;break;}	
					k++;	
					}
					if( k==-1 || ((k<8) && br[j*32+k]!=0x20  )){j++;continue;}
						k=8;
					e++;
					while(k<11)
					{
					if (toupper(*(a+e)) != br[j*32+k] && tolower(*(a+e)) != br[j*32+k]  ){k=-1;j++;break;}	
						e++;
						k++;
					}
					if( k==-1 ){j++;continue;}
					read_file((br[j*32+27]<<8)+br[j*32+26]);
					current_dir=tmp;
					return ;
				j++;
				}	
				i++;
			}
			readblock(br,(reserved_sectors+(current_dir)/(BPS/2)),1);
			
			
			if(br[(current_dir%(BPS/2))*2] ==0xFF &&    br[(current_dir%(BPS/2))*2+1]== 0xFF) break;
			else current_dir=(br[(current_dir%(BPS/2))*2+1]<<8)+br[(current_dir%(BPS/2))*2];

		}
		current_dir=tmp;
		}
	
	printf("Nie ma takiego pliku\n");
	
	
	}

void save_file(uint16_t start,char * a)
{
FILE* zapis;
	zapis=fopen(a,"wb");
if(zapis==NULL) 
{
    printf("Couldn't create file");
    return ;
}


		if(start<3) return;

	
	while(1){
		int i=0;
		while(i<2)
			{
				readblock(br, (reserved_sectors+sectors_per_fat*fat_count+i+32+(start-2)*2),1);
				int j=0;
				
					while(j<16)
				{	
					int k=0;
					while(k<32){
						if(br[j*32+k]!= 0x00)
					    fwrite(&br[j*32+k],1,1,zapis);
					    else {fclose(zapis); return;}
					    k++;
					}
				j++;
				}	
				i++;
			}
			readblock(br, (reserved_sectors+(start)/(BPS/2)),1);
		
			
			if(br[(start%(BPS/2))*2] ==0xFF &&    br[(start%(BPS/2))*2+1]== 0xFF) {fclose(zapis);return;}
			else start=(br[(start%(BPS/2))*2+1]<<8)+br[(start%(BPS/2))*2];
	}	
}

void get(char *a){
	
	
a=a+4;
int e=0;
while(*(a+e)!='\0') e++;
e=e-4;
if(current_dir==0){
			int i=0;
			while(i<32)
			{
				readblock(br, (reserved_sectors+sectors_per_fat*fat_count+i),1);
				int j=0;
				while(j<16)
				{	
					if(br[j*32]==0x00 || br[j*32]==0xE5 || br[j*32+11]==0x0F ) {j++; continue;}
					int k=0;
					while(k<e){
					if (toupper(*(a+k)) != br[j*32+k] && tolower(*(a+k)) != br[j*32+k]  ){k=-1;j++;break;}	
					k++;	
					}
					if( k==-1 || ((k<8) && br[j*32+k]!=0x20  )){j++;continue;}
					k=8;
					e++;
					while(k<11)
					{
					if (toupper(*(a+e)) != br[j*32+k] && tolower(*(a+e)) != br[j*32+k]  ){k=-1;j++;break;}	
						e++;
						k++;
					}
					if( k==-1 ){j++;continue;}
					
					save_file((br[j*32+27]<<8)+br[j*32+26],a);
					return ;
				}	
				
				i++;
			}
		}
		else{
			int tmp=current_dir;
			while(1){
			int i=0;
			while(i<2)
			{
				readblock(br, (reserved_sectors+sectors_per_fat*fat_count+i+32+(current_dir-2)*2),1);
				int j=0;
				
					while(j<16)
				{	
						if(br[j*32]==0x00 || br[j*32]==0xE5 || br[j*32+11]==0x0F  ) {j++; continue;}
					int k=0;
					while(k<e){
					if (toupper(*(a+k)) != br[j*32+k] && tolower(*(a+k)) != br[j*32+k]  ){k=-1;break;}	
					k++;	
					}
					if( k==-1 || ((k<8) && br[j*32+k]!=0x20  )){j++;continue;}
						k=8;
					e++;
					while(k<11)
					{
					if (toupper(*(a+e)) != br[j*32+k] && tolower(*(a+e)) != br[j*32+k]  ){k=-1;j++;break;}	
						e++;
						k++;
					}
					if( k==-1 ){j++;continue;}
					save_file((br[j*32+27]<<8)+br[j*32+26],a);
					
					return ;
				j++;
				}	
				i++;
			}
			readblock(br, (reserved_sectors+(current_dir)/(BPS/2)),1);
			
			
			if(br[(current_dir%(BPS/2))*2] ==0xFF &&    br[(current_dir%(BPS/2))*2+1]== 0xFF) break;
			else current_dir=(br[(current_dir%(BPS/2))*2+1]<<8)+br[(current_dir%(BPS/2))*2];

		}
		current_dir=tmp;
		}
	
	printf("Nie ma takiego pliku\n");
	
	
	
	}

void rootinfo(){
		int i=0,k=0;
			while(i<32)
			{
				readblock(br, (reserved_sectors+sectors_per_fat*fat_count+i),1);
				int j=0;
				while(j<16)
				{	
					if(br[j*32]==0x00 || br[j*32]==0xE5 ) {j++; continue;}

					k++;j++;
				}			
				i++;
			}
	printf("Liczba wpisów: %d\nMaksymalna liczba wpisów %d\nProcentowe wypełnienie %.2f%%\n",k,root_dir_capacity,((float)k/root_dir_capacity)*100);
	}

void spaceinfo(){
		uint32_t tmp=2,ff=0,zeros=0,taken=0,damaged=0;
		while(tmp<(logical_sectors16/2)){
			readblock(br, (reserved_sectors+(tmp)/(BPS/2)),1);
			uint16_t temp=  (br[((tmp)%(BPS/2))*2+1]<<8) +   br[((tmp)%(BPS/2))*2];
			if(temp==0x0000) zeros++;
			if(temp==0xFFF7) damaged++;
			if(temp>=0x0003 && temp<= (logical_sectors16/2)) taken++;
			if(temp>=0xFFF8 && temp<=0xFFFF){taken++; ff++;}
		
		tmp++;
		}
		
printf("Liczba klastrów zajętych: %d\nLiczba klastrów wolnych %d\nLiczba klastrów uszkodzonych %d\nLiczba klastrów kończących %d\nWielkość klastra w bajtach: %d Bajtów\nWielkośc klastra w sektorach: %d\n",taken,zeros,damaged,ff,bytes_per_sector*sectors_per_cluster,sectors_per_cluster);
}



void fileinfo_read(uint16_t start){
	if(start<3) return ;
	int licznik=0;
	printf("Klastry: ");
	while(1){

		readblock(br,(reserved_sectors+(start)/(BPS/2)),1);
			printf("%d ",start);
			licznik++;	
			if(br[(start%(BPS/2))*2] ==0xFF &&    br[(start%(BPS/2))*2+1]== 0xFF) {printf("\nLiczba klastrów %d\n",licznik); return ;}
			else {start=(br[(start%(BPS/2))*2+1]<<8)+br[(start%(BPS/2))*2];}

	}	
	
	}


void fileinfo(char * a)
{

a=a+9;
int e=0;
while(*(a+e)!='\0') e++;
e=e-4;
if(current_dir==0){
			int i=0;
			while(i<32)
			{
				readblock(br,(reserved_sectors+sectors_per_fat*fat_count+i),1);
				int j=0;
				while(j<16)
				{	
					if(br[j*32]==0x00 || br[j*32]==0xE5 || br[j*32+11]==0x0F ) {j++; continue;}
					int k=0;
					while(k<e){
					if (toupper(*(a+k)) != br[j*32+k] && tolower(*(a+k)) != br[j*32+k]  ){k=-1;j++;break;}	
					k++;	
					}
					if( k==-1 || ((k<8) && br[j*32+k]!=0x20  )){j++;continue;}
					k=8;
					e++;
					while(k<11)
					{
					if (toupper(*(a+e)) != br[j*32+k] && tolower(*(a+e)) != br[j*32+k]  ){k=-1;j++;break;}	
						e++;
						k++;
					}
					if( k==-1 ){j++;continue;}
										
					pwd(0);
					printf("%s\n",a);
					printf("Rozmiar: %d Bajtów\n",(br[j*32+31]<<24)+(br[j*32+30]<<16)+(br[j*32+29]<<8)+br[j*32+28]);
					
					k=0;
					uint8_t tmp,first;
					tmp=br[j*32+25]<<7;
					tmp=tmp>>4;
					first=tmp+(br[j*32+24]>>5);
					tmp=br[j*32+24]<<3;
					tmp=tmp>>3;
					printf("Ostatni zapis: %02d/%02d/%04d",tmp,first,(br[j*32+25]>>1)+1980);
					
					tmp=(br[j*32+23]<<5);
					tmp=tmp>>2;
					first=tmp+(br[j*32+22]>>5);
					printf(" %02d:%02d\n",(br[j*32+23]>>3)+2, first);
					
					k=0;
					tmp=0;first=0;
					tmp=br[j*32+19]<<7;
					tmp=tmp>>4;
					first=tmp+(br[j*32+18]>>5);
					tmp=br[j*32+18]<<3;
					tmp=tmp>>3;
					printf("Ostatni dostęp: %02d/%02d/%04d",tmp,first,(br[j*32+19]>>1)+1980);
					
					tmp=(br[j*32+23]<<5);
					tmp=tmp>>2;
					first=tmp+(br[j*32+22]>>5);
					printf(" %02d:%02d\n",(br[j*32+23]>>3)+2, first);
					
					k=0;
					tmp=0;first=0;
					tmp=br[j*32+17]<<7;
					tmp=tmp>>4;
					first=tmp+(br[j*32+16]>>5);
					tmp=br[j*32+16]<<3;
					tmp=tmp>>3;
					printf("Utworzono: %02d/%02d/%04d",tmp,first,(br[j*32+17]>>1)+1980);
					
					tmp=(br[j*32+15]<<5);
					tmp=tmp>>2;
					first=tmp+(br[j*32+14]>>5);
					printf(" %02d:%02d\n",(br[j*32+15]>>3)+2, first);
					
					tmp=br[j*32+11];
					printf("Atrybuty: R");
					if(tmp%2==1) printf("+");
					else printf("-");
					tmp=tmp>>1;
					printf(" H");
					if(tmp%2==1) printf("+");
					else printf("-");
					tmp=tmp>>1;
					printf(" S");
					if(tmp%2==1) printf("+");
					else printf("-");
					tmp=tmp>>1;
					printf(" V");
					if(tmp%2==1) printf("+");
					else printf("-");
					tmp=tmp>>1;
					printf(" D");
					if(tmp%2==1) printf("+");
					else printf("-");
					tmp=tmp>>1;
					printf(" A");
					if(tmp%2==1) printf("+\n");
					else printf("-\n");

					fileinfo_read((br[j*32+27]<<8)+br[j*32+26]);
					return ;
				}	
				
				i++;
			}
		}
		else{
			int tmp=current_dir;
			while(1){
			int i=0;
			while(i<2)
			{
				readblock(br,(reserved_sectors+sectors_per_fat*fat_count+i+32+(current_dir-2)*2),1);
				int j=0;
				
					while(j<16)
				{	
						if(br[j*32]==0x00 || br[j*32]==0xE5 || br[j*32+11]==0x0F  ) {j++; continue;}
					int k=0;
					while(k<e){
					if (toupper(*(a+k)) != br[j*32+k] && tolower(*(a+k)) != br[j*32+k]  ){k=-1;break;}	
					k++;	
					}
					if( k==-1 || ((k<8) && br[j*32+k]!=0x20  )){j++;continue;}
						k=8;
					e++;
					while(k<11)
					{
					if (toupper(*(a+e)) != br[j*32+k] && tolower(*(a+e)) != br[j*32+k]  ){k=-1;j++;break;}	
						e++;
						k++;
					}
					if( k==-1 ){j++;continue;}
					pwd(0);
					readblock(br,(reserved_sectors+sectors_per_fat*fat_count+i+32+(current_dir-2)*2),1);
					printf("%s\n",a);
					printf("Rozmiar: %d Bajtów\n",(br[j*32+31]<<24)+(br[j*32+30]<<16)+(br[j*32+29]<<8)+br[j*32+28]);
					k=0;
					uint8_t tmp,first;
					tmp=br[j*32+25]<<7;
					tmp=tmp>>4;
					first=tmp+(br[j*32+24]>>5);
					tmp=br[j*32+24]<<3;
					tmp=tmp>>3;
					printf("Ostatni zapis: %02d/%02d/%04d",tmp,first,(br[j*32+25]>>1)+1980);
					
					tmp=(br[j*32+23]<<5);
					tmp=tmp>>2;
					first=tmp+(br[j*32+22]>>5);
					printf(" %02d:%02d\n",(br[j*32+23]>>3)+2, first);
					
					k=0;
					tmp=0;first=0;
					tmp=br[j*32+19]<<7;
					tmp=tmp>>4;
					first=tmp+(br[j*32+18]>>5);
					tmp=br[j*32+18]<<3;
					tmp=tmp>>3;
					printf("Ostatni dostęp: %02d/%02d/%04d",tmp,first,(br[j*32+19]>>1)+1980);
					
					tmp=(br[j*32+23]<<5);
					tmp=tmp>>2;
					first=tmp+(br[j*32+22]>>5);
					printf(" %02d:%02d\n",(br[j*32+23]>>3)+2, first);
					
					k=0;
					tmp=0;first=0;
					tmp=br[j*32+17]<<7;
					tmp=tmp>>4;
					first=tmp+(br[j*32+16]>>5);
					tmp=br[j*32+16]<<3;
					tmp=tmp>>3;
					printf("Utworzono: %02d/%02d/%04d",tmp,first,(br[j*32+17]>>1)+1980);
					
					tmp=(br[j*32+15]<<5);
					tmp=tmp>>2;
					first=tmp+(br[j*32+14]>>5);
					printf(" %02d:%02d\n",(br[j*32+15]>>3)+2, first);
					
					tmp=br[j*32+11];
					printf("Atrybuty: R");
					if(tmp%2==1) printf("+");
					else printf("-");
					tmp=tmp>>1;
					printf(" H");
					if(tmp%2==1) printf("+");
					else printf("-");
					tmp=tmp>>1;
					printf(" S");
					if(tmp%2==1) printf("+");
					else printf("-");
					tmp=tmp>>1;
					printf(" V");
					if(tmp%2==1) printf("+");
					else printf("-");
					tmp=tmp>>1;
					printf(" D");
					if(tmp%2==1) printf("+");
					else printf("-");
					tmp=tmp>>1;
					printf(" A");
					if(tmp%2==1) printf("+\n");
					else printf("-\n");

					fileinfo_read((br[j*32+27]<<8)+br[j*32+26]);
					return ;
				j++;
				}	
				i++;
			}
			readblock(br,(reserved_sectors+(current_dir)/(BPS/2)),1);
			
			
			if(br[(current_dir%(BPS/2))*2] ==0xFF &&    br[(current_dir%(BPS/2))*2+1]== 0xFF) break;
			else current_dir=(br[(current_dir%(BPS/2))*2+1]<<8)+br[(current_dir%(BPS/2))*2];

		}
		current_dir=tmp;
		}
	
	printf("Nie ma takiego pliku\n");
	
}


void save_file_two(uint16_t first, uint16_t second, char *a)
{
	if(first<3) {save_file(second,a);return ;}
	if(second<3) {save_file(first,a);return ;}
	FILE* zapis;
	zapis=fopen(a,"wb");
	if(zapis==NULL) 
	{
		printf("Couldn't create file");
		return ;
	}
	int first_empty=0,second_empty=0,first_entry=1;
	
	while(1){
		int i=0;
		while(i<2)
			{
				readblock(br, (reserved_sectors+sectors_per_fat*fat_count+i+32+(first-2)*2),1);
				int j=0;
				
					while(j<16)
				{	
					int k=0;
					while(k<32){
					if(br[j*32+k]!= 0x00){
					    if(br[j*32+k]== '\n' && second_empty==0)
					    {
							fwrite(&br[j*32+k],1,1,zapis);
							if(first_entry)   {first_entry=0; goto LOOP;}
							else if(second_empty==0) goto LOOP2;
							else goto LOOP3;
					    }
					    fwrite(&br[j*32+k],1,1,zapis);
						
					}
					else {
						first_empty=1;
						if(first_entry)   {first_entry=0; goto LOOP;}
						else if(second_empty==0) goto LOOP2;
						else goto LOOP4;
						 }
						 LOOP3:
					    k++;
					}
				j++;
				}	
				i++;
			}
			readblock(br, (reserved_sectors+(first)/(BPS/2)),1);
		
			
			if(br[(first%(BPS/2))*2] ==0xFF &&    br[(first%(BPS/2))*2+1]== 0xFF) {
				first_empty=1;
				if(first_entry)   {first_entry=0; goto LOOP;}
							else if(second_empty==0) goto LOOP2;
							else goto LOOP4;
				}
			else first=(br[(first%(BPS/2))*2+1]<<8)+br[(first%(BPS/2))*2];
	}	
	LOOP:
	
		while(1){
		int i=0;
		while(i<2)
			{
				readblock(br+BPS, (reserved_sectors+sectors_per_fat*fat_count+i+32+(second-2)*2),1);
				int j=0;
				
					while(j<16)
				{	
					int k=0;
					while(k<32){
					if(br[j*32+k+BPS]!= 0x00){
					    if(br[j*32+k+BPS]== '\n' && second_empty==0)
					    {
							fwrite(&br[j*32+k+BPS],1,1,zapis);
							if(first_empty==0) goto LOOP3;
							else goto LOOP2;
					    }
					    fwrite(&br[j*32+k+BPS],1,1,zapis);
						
					}
					else {
						second_empty=1;
						 if(first_empty==0) goto LOOP3;
						else goto LOOP4;
						 }
						 LOOP2:
					    k++;
					}
				j++;
				}	
				i++;
			}
			readblock(br+BPS, (reserved_sectors+(second)/(BPS/2)),1);
		
			
			if(br[(second%(BPS/2))*2+BPS] ==0xFF &&    br[(second%(BPS/2))*2+1+BPS]== 0xFF) {
						
							if(first_empty==0) goto LOOP3;
							else goto LOOP4;
				}
			else second=(br[(second%(BPS/2))*2+1+BPS]<<8)+br[(second%(BPS/2))*2+BPS];
	}	
	
	LOOP4:
	fclose(zapis);
	
	}

void zip(char *a)
{
a=a+4;
int e=0,e1=1;
while(*(a+e)!=' ') e++;
while(*(a+e+e1+1)!=' ') e1++;
e1=e1-4;
e=e-4;

uint16_t first=0;
if(current_dir==0){
			int i=0;
			while(i<32)
			{
				readblock(br,(reserved_sectors+sectors_per_fat*fat_count+i),1);
				int j=0;
				while(j<16)
				{	
					if(br[j*32]==0x00 || br[j*32]==0xE5 || br[j*32+11]==0x0F ) {j++; continue;}
					int k=0;
					while(k<e){
					if (toupper(*(a+k)) != br[j*32+k] && tolower(*(a+k)) != br[j*32+k]  ){k=-1;j++;break;}	
					k++;	
					}
					if( k==-1 || ((k<8) && br[j*32+k]!=0x20  )){j++;continue;}
					k=8;
					e++;
					while(k<11)
					{
					if (toupper(*(a+e)) != br[j*32+k] && tolower(*(a+e)) != br[j*32+k]  ){k=-1;j++;break;}	
						e++;
						k++;
					}
					if( k==-1 ){j++;continue;}
					a=a+e+1;
					first=(br[j*32+27]<<8)+br[j*32+26];
					
					i=0;
			while(i<32)
			{
				readblock(br,(reserved_sectors+sectors_per_fat*fat_count+i),1);
				int j=0;
				while(j<16)
				{	
					if(br[j*32]==0x00 || br[j*32]==0xE5 || br[j*32+11]==0x0F ) {j++; continue;}
					int k=0;
					while(k<e1){
					if (toupper(*(a+k)) != br[j*32+k] && tolower(*(a+k)) != br[j*32+k]  ){k=-1;j++;break;}	
					
					k++;	
					}
					if( k==-1 || ((k<8) && br[j*32+k]!=0x20  )){j++;continue;}
					k=8;
					e1++;
					while(k<11)
					{
					if (toupper(*(a+e1)) != br[j*32+k] && tolower(*(a+e1)) != br[j*32+k]  ){k=-1;j++;break;}	
						e1++;
						k++;
					}
					if( k==-1 ){j++;continue;}
					a=a+e1+1;
					
					save_file_two(first,(br[j*32+27]<<8)+br[j*32+26],a);
					
					return ;
				}	
				
				i++;
			}
					
					
					return ;
				}	
				
				i++;
			}
		}
		else{
			int tmp=current_dir;
			while(1){
			int i=0;
			while(i<2)
			{
				readblock(br,(reserved_sectors+sectors_per_fat*fat_count+i+32+(current_dir-2)*2),1);
				int j=0;
				
					while(j<16)
				{	
						if(br[j*32]==0x00 || br[j*32]==0xE5 || br[j*32+11]==0x0F  ) {j++; continue;}
					int k=0;
					while(k<e){
					if (toupper(*(a+k)) != br[j*32+k] && tolower(*(a+k)) != br[j*32+k]  ){k=-1;break;}	
					k++;	
					}
					if( k==-1 || ((k<8) && br[j*32+k]!=0x20  )){j++;continue;}
						k=8;
					e++;
					while(k<11)
					{
					if (toupper(*(a+e)) != br[j*32+k] && tolower(*(a+e)) != br[j*32+k]  ){k=-1;j++;break;}	
						e++;
						k++;
					}
					if( k==-1 ){j++;continue;}
					a=a+e+1;
					first=(br[j*32+27]<<8)+br[j*32+26];
					
					i=0;
					current_dir=tmp;
			while(i<2)
			{
				readblock(br,(reserved_sectors+sectors_per_fat*fat_count+i+32+(current_dir-2)*2),1);
				int j=0;
				while(j<16)
				{	
					if(br[j*32]==0x00 || br[j*32]==0xE5 || br[j*32+11]==0x0F ) {j++; continue;}
					int k=0;
					while(k<e1){
					if (toupper(*(a+k)) != br[j*32+k] && tolower(*(a+k)) != br[j*32+k]  ){k=-1;j++;break;}	
					
					k++;	
					}
					if( k==-1 || ((k<8) && br[j*32+k]!=0x20  )){j++;continue;}
					k=8;
					e1++;
					while(k<11)
					{
					if (toupper(*(a+e1)) != br[j*32+k] && tolower(*(a+e1)) != br[j*32+k]  ){k=-1;j++;break;}	
						e1++;
						k++;
					}
					if( k==-1 ){j++;continue;}
					a=a+e1+1;
					
					save_file_two(first,(br[j*32+27]<<8)+br[j*32+26],a);
						current_dir=tmp;
					return ;
				}	
				
				i++;
			}
			current_dir=tmp;
					return ;
				j++;
				}	
				i++;
			}
			readblock(br,(reserved_sectors+(current_dir)/(BPS/2)),1);
			
			
			if(br[(current_dir%(BPS/2))*2] ==0xFF &&    br[(current_dir%(BPS/2))*2+1]== 0xFF) break;
			else current_dir=(br[(current_dir%(BPS/2))*2+1]<<8)+br[(current_dir%(BPS/2))*2];

		}
		current_dir=tmp;
		}
	
	printf("Nie ma takiego pliku\n");
	
	
	}


int main(int argc, char **argv)
{
assert(readblock(br, 0,1)==1);
  char *a;
    a=calloc(100,sizeof(char));
    assert(a != NULL);
	int i=0;  



while(1){
  while(1){
		i=-1;
    printf("$ ");
   	do{
i++;
*(a+i)=fgetc(stdin);
}while(*(a+i)!='\n');
*(a+i)='\0';
	if (strcmp(a,"exit")==0) {fclose(fvolume);return -1;}		
	if (strcmp(a,"dir")==0) {dir();break;}	
	if (strncmp(a,"cd ",3)==0) {cd(a);break;}	
	if (strcmp(a,"pwd")==0) {pwd(0);break;}	
	if (strncmp(a,"cat ",4)==0) {cat(a);break;}	
	if (strncmp(a,"get ",4)==0) {get(a);break;}	
	if (strncmp(a,"zip ",4)==0) {zip(a);break;}
	if (strcmp(a,"rootinfo")==0) {rootinfo();break;}		
	if (strcmp(a,"spaceinfo")==0) {spaceinfo();break;}		
	if (strncmp(a,"fileinfo ",9)==0){fileinfo(a); break;}
	printf("Złe polecenie\n");	

}

	


}
free(a);
fclose(fvolume);
return 0;
}
