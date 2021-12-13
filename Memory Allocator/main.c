#include <stdio.h>
#include <stdlib.h>
#include "custom_unistd.h"
#include  <string.h>
#define START_CAPACITY 3*sizeof(struct memblock_t) 
#define ADD_CAPACITY  0

//

#define DEBUG 1
#if DEBUG
  #define DEBUG_ARGS , const char * filename , int fileline
  #define DEBUG_ARGS_WO const char * filename , int fileline
  #define DEBUG_RUN_ARGS ,  __FILE__ , __LINE__
  #define DEBUG_RUN_ARGS_WO  __FILE__ , __LINE__
 #else
  #define DEBUG_ARGS  
  #define DEBUG_ARGS_WO  
  #define DEBUG_RUN_ARGS 
  #define DEBUG_RUN_ARGS_WO
#endif


#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

static const u_int32_t crc32Table[256] = {
	0x00000000L, 0xF26B8303L, 0xE13B70F7L, 0x1350F3F4L,
	0xC79A971FL, 0x35F1141CL, 0x26A1E7E8L, 0xD4CA64EBL,
	0x8AD958CFL, 0x78B2DBCCL, 0x6BE22838L, 0x9989AB3BL,
	0x4D43CFD0L, 0xBF284CD3L, 0xAC78BF27L, 0x5E133C24L,
	0x105EC76FL, 0xE235446CL, 0xF165B798L, 0x030E349BL,
	0xD7C45070L, 0x25AFD373L, 0x36FF2087L, 0xC494A384L,
	0x9A879FA0L, 0x68EC1CA3L, 0x7BBCEF57L, 0x89D76C54L,
	0x5D1D08BFL, 0xAF768BBCL, 0xBC267848L, 0x4E4DFB4BL,
	0x20BD8EDEL, 0xD2D60DDDL, 0xC186FE29L, 0x33ED7D2AL,
	0xE72719C1L, 0x154C9AC2L, 0x061C6936L, 0xF477EA35L,
	0xAA64D611L, 0x580F5512L, 0x4B5FA6E6L, 0xB93425E5L,
	0x6DFE410EL, 0x9F95C20DL, 0x8CC531F9L, 0x7EAEB2FAL,
	0x30E349B1L, 0xC288CAB2L, 0xD1D83946L, 0x23B3BA45L,
	0xF779DEAEL, 0x05125DADL, 0x1642AE59L, 0xE4292D5AL,
	0xBA3A117EL, 0x4851927DL, 0x5B016189L, 0xA96AE28AL,
	0x7DA08661L, 0x8FCB0562L, 0x9C9BF696L, 0x6EF07595L,
	0x417B1DBCL, 0xB3109EBFL, 0xA0406D4BL, 0x522BEE48L,
	0x86E18AA3L, 0x748A09A0L, 0x67DAFA54L, 0x95B17957L,
	0xCBA24573L, 0x39C9C670L, 0x2A993584L, 0xD8F2B687L,
	0x0C38D26CL, 0xFE53516FL, 0xED03A29BL, 0x1F682198L,
	0x5125DAD3L, 0xA34E59D0L, 0xB01EAA24L, 0x42752927L,
	0x96BF4DCCL, 0x64D4CECFL, 0x77843D3BL, 0x85EFBE38L,
	0xDBFC821CL, 0x2997011FL, 0x3AC7F2EBL, 0xC8AC71E8L,
	0x1C661503L, 0xEE0D9600L, 0xFD5D65F4L, 0x0F36E6F7L,
	0x61C69362L, 0x93AD1061L, 0x80FDE395L, 0x72966096L,
	0xA65C047DL, 0x5437877EL, 0x4767748AL, 0xB50CF789L,
	0xEB1FCBADL, 0x197448AEL, 0x0A24BB5AL, 0xF84F3859L,
	0x2C855CB2L, 0xDEEEDFB1L, 0xCDBE2C45L, 0x3FD5AF46L,
	0x7198540DL, 0x83F3D70EL, 0x90A324FAL, 0x62C8A7F9L,
	0xB602C312L, 0x44694011L, 0x5739B3E5L, 0xA55230E6L,
	0xFB410CC2L, 0x092A8FC1L, 0x1A7A7C35L, 0xE811FF36L,
	0x3CDB9BDDL, 0xCEB018DEL, 0xDDE0EB2AL, 0x2F8B6829L,
	0x82F63B78L, 0x709DB87BL, 0x63CD4B8FL, 0x91A6C88CL,
	0x456CAC67L, 0xB7072F64L, 0xA457DC90L, 0x563C5F93L,
	0x082F63B7L, 0xFA44E0B4L, 0xE9141340L, 0x1B7F9043L,
	0xCFB5F4A8L, 0x3DDE77ABL, 0x2E8E845FL, 0xDCE5075CL,
	0x92A8FC17L, 0x60C37F14L, 0x73938CE0L, 0x81F80FE3L,
	0x55326B08L, 0xA759E80BL, 0xB4091BFFL, 0x466298FCL,
	0x1871A4D8L, 0xEA1A27DBL, 0xF94AD42FL, 0x0B21572CL,
	0xDFEB33C7L, 0x2D80B0C4L, 0x3ED04330L, 0xCCBBC033L,
	0xA24BB5A6L, 0x502036A5L, 0x4370C551L, 0xB11B4652L,
	0x65D122B9L, 0x97BAA1BAL, 0x84EA524EL, 0x7681D14DL,
	0x2892ED69L, 0xDAF96E6AL, 0xC9A99D9EL, 0x3BC21E9DL,
	0xEF087A76L, 0x1D63F975L, 0x0E330A81L, 0xFC588982L,
	0xB21572C9L, 0x407EF1CAL, 0x532E023EL, 0xA145813DL,
	0x758FE5D6L, 0x87E466D5L, 0x94B49521L, 0x66DF1622L,
	0x38CC2A06L, 0xCAA7A905L, 0xD9F75AF1L, 0x2B9CD9F2L,
	0xFF56BD19L, 0x0D3D3E1AL, 0x1E6DCDEEL, 0xEC064EEDL,
	0xC38D26C4L, 0x31E6A5C7L, 0x22B65633L, 0xD0DDD530L,
	0x0417B1DBL, 0xF67C32D8L, 0xE52CC12CL, 0x1747422FL,
	0x49547E0BL, 0xBB3FFD08L, 0xA86F0EFCL, 0x5A048DFFL,
	0x8ECEE914L, 0x7CA56A17L, 0x6FF599E3L, 0x9D9E1AE0L,
	0xD3D3E1ABL, 0x21B862A8L, 0x32E8915CL, 0xC083125FL,
	0x144976B4L, 0xE622F5B7L, 0xF5720643L, 0x07198540L,
	0x590AB964L, 0xAB613A67L, 0xB831C993L, 0x4A5A4A90L,
	0x9E902E7BL, 0x6CFBAD78L, 0x7FAB5E8CL, 0x8DC0DD8FL,
	0xE330A81AL, 0x115B2B19L, 0x020BD8EDL, 0xF0605BEEL,
	0x24AA3F05L, 0xD6C1BC06L, 0xC5914FF2L, 0x37FACCF1L,
	0x69E9F0D5L, 0x9B8273D6L, 0x88D28022L, 0x7AB90321L,
	0xAE7367CAL, 0x5C18E4C9L, 0x4F48173DL, 0xBD23943EL,
	0xF36E6F75L, 0x0105EC76L, 0x12551F82L, 0xE03E9C81L,
	0x34F4F86AL, 0xC69F7B69L, 0xD5CF889DL, 0x27A40B9EL,
	0x79B737BAL, 0x8BDCB4B9L, 0x988C474DL, 0x6AE7C44EL,
	0xBE2DA0A5L, 0x4C4623A6L, 0x5F16D052L, 0xAD7D5351L
};

static u_int32_t singletable_crc32c(u_int32_t crc, const void *buf, size_t size)
{
	const u_int8_t *p = buf;


	while (size--)
		crc = crc32Table[(crc ^ *p++) & 0xff] ^ (crc >> 8);

	return crc;
}

//


	struct memblock_t{
		struct memblock_t *next;
		struct memblock_t *prev;
		intptr_t size;
		#if DEBUG
		int fileline;
		char filename[32];
		#endif
		u_int64_t crc32;
}*start, *end;

enum pointer_type_t
{
pointer_null,
pointer_out_of_heap,
pointer_control_block,
pointer_inside_data_block,
pointer_unallocated,
pointer_valid
};


int heap_setup(DEBUG_ARGS_WO);
size_t heap_get_free_space(void);
size_t heap_get_used_space(void);
size_t heap_get_largest_used_block_size(void);
size_t heap_get_largest_free_area(void);
size_t heap_get_largest_free_area_wo(void);
u_int64_t heap_get_used_blocks_count();
u_int64_t heap_get_free_gaps_count();
void* heap_malloc(size_t count DEBUG_ARGS);
void view_heap();
void* heap_calloc(size_t number, size_t size DEBUG_ARGS);
void heap_free(void* memblock);
enum pointer_type_t get_pointer_type(const const void* pointer);
void* heap_realloc(void* memblock, size_t size DEBUG_ARGS);
void* heap_get_data_block_start(const void* pointer);
size_t heap_get_block_size(const const void* memblock);
int heap_validate(void);
int heap_reset(DEBUG_ARGS_WO);

int heap_setup(DEBUG_ARGS_WO)
{
	if(start!=NULL && end!=NULL) return -1;
	if (START_CAPACITY < 3 * sizeof(struct memblock_t)) return -1;
	void *e =custom_sbrk(START_CAPACITY);
	if ((intptr_t )e==-1) return -1;
	void *el =custom_sbrk(0);

	start=(struct memblock_t *)e;
	struct memblock_t *middle =(struct memblock_t *)(e + sizeof(struct memblock_t ) );
	end=(struct memblock_t *)(el - sizeof(struct memblock_t ));
	

	end->next=NULL;
	end->prev=middle;
	end->size=0;
	#if DEBUG
	end->fileline=fileline;
	if(filename!=NULL)strncpy(end->filename,filename,30);
	#endif
	start->next=middle;
	start->prev=NULL;
	start->size=0;
	#if DEBUG 
	start->fileline=fileline;
	if(filename!=NULL)strncpy(start->filename,filename,30);
	#endif
	middle->prev= start;
	middle->next= end;
	middle->size=START_CAPACITY - 3 * sizeof(struct memblock_t);
	middle->size *=-1;
	#if DEBUG 
	middle->fileline=fileline;
	if(filename!=NULL)strncpy(middle->filename,filename,30);
	#endif
	start->crc32=singletable_crc32c(0xFFFFFFFF,start,sizeof(struct memblock_t)-sizeof(u_int64_t));
	end->crc32=singletable_crc32c(0xFFFFFFFF,end,sizeof(struct memblock_t)-sizeof(u_int64_t));
	middle->crc32=singletable_crc32c(0xFFFFFFFF,middle,sizeof(struct memblock_t)-sizeof(u_int64_t));

	return 0;
}
size_t heap_get_free_space(void)
{
		struct memblock_t* tmp;
		tmp=start->next;
		int suma=0;
		if (start->size <0) suma+= start->size *-1;
		while (tmp!=NULL)
		{
			if (tmp->size <0) suma+= tmp->size *-1;
			tmp=tmp->next;
		}
	return suma ;
}

size_t heap_get_used_space(void)
{	
		struct memblock_t* tmp;
		tmp=start->next;
		int suma=sizeof(struct memblock_t);
		if (start->size >0) suma+= start->size;
		while (tmp!=NULL)
		{
			if (tmp->size >0) suma+= tmp->size ;
			suma+= sizeof(struct memblock_t);
			tmp=tmp->next;
	
		}
	return suma;
}
size_t heap_get_largest_free_area(void)
{
		struct memblock_t* tmp;
		tmp=start->next;
		int suma=0;
		if (start->size <0 && suma > start->size) suma= start->size;
while (tmp!=NULL)
		{
			if (tmp->size <0 && suma > tmp->size) suma= tmp->size ;
			tmp=tmp->next;
		}

	return suma*-1;
}
size_t heap_get_largest_free_area_wo(void)
{
		struct memblock_t* tmp;
		tmp=start->next;
		int suma=0;
		if (start->size <0 && suma > start->size) suma= start->size;
while (tmp!=NULL)
		{
			if (tmp->size <0 && suma > tmp->size) suma= tmp->size ;
			tmp=tmp->next;
		}


	return suma*-1;
}
size_t heap_get_largest_used_block_size(void)
{
		struct memblock_t* tmp;
		tmp=start->next;
		int suma=0;
		if (start->size >0 && suma < start->size) suma= start->size;
		while (tmp!=NULL)
		{
			if (tmp->size >0 && suma < tmp->size) suma= tmp->size ;
			tmp=tmp->next;
		}
	return suma;

}
void* heap_malloc(size_t count  DEBUG_ARGS)
{
	if (count==0) return NULL;
	size_t rozmiar= heap_get_largest_free_area_wo();
	struct memblock_t  * tmp;
	if (rozmiar < count+sizeof(struct memblock_t))
	{
			if ((intptr_t)custom_sbrk(count+sizeof(struct memblock_t)+ADD_CAPACITY)==-1) return NULL;
			
			void *e =custom_sbrk(0);
			end->prev->size -=count + sizeof(struct memblock_t)+ADD_CAPACITY;
			
			tmp=end;
			end=(struct memblock_t *)(e - sizeof(struct memblock_t ));

			end->next=NULL;
			end->prev=tmp->prev;
			end->prev->next= end;
			end->size=0;
			#if DEBUG
			end->fileline=fileline;
			if(filename!=NULL)strncpy(end->filename,filename,30);
			end->prev->fileline=fileline;
			if(filename!=NULL)strncpy(end->prev->filename,filename,30);
			#endif

			end->prev->crc32=singletable_crc32c(0xFFFFFFFF,end->prev,sizeof(struct memblock_t)-sizeof(u_int64_t));
			end->crc32=singletable_crc32c(0xFFFFFFFF,end,sizeof(struct memblock_t)-sizeof(u_int64_t));	

}
tmp=start;
	while (1)
		{
	if (tmp->size>0) {
			tmp=tmp->next;
			if (tmp == NULL) return NULL;
			continue;
		}     
		
	if (abs(tmp->size) >= count + sizeof(struct memblock_t) ) break;
			tmp=tmp->next;
			if (tmp == NULL) 	return NULL;
	}
	int size=tmp->size;
	tmp->size= count;
///
	struct memblock_t *e =(void *)tmp +( sizeof(struct memblock_t ) + count);
	
		
	e->next = tmp ->next;
	e->prev = tmp;
	tmp->next= e;
	e->next->prev=e;
	e->size= size+ count+sizeof(struct memblock_t);	
	#if DEBUG
	e->fileline=fileline;
	if(filename!=NULL)strncpy(e->filename,filename,30);
	e->next->fileline=fileline;
	if(filename!=NULL)strncpy(e->next->filename,filename,30);
	e->prev->fileline=fileline;
	if(filename!=NULL)strncpy(e->prev->filename,filename,30);
	#endif
	e->crc32=singletable_crc32c(0xFFFFFFFF,e,sizeof(struct memblock_t)-sizeof(u_int64_t));
	e->next->crc32=singletable_crc32c(0xFFFFFFFF,e->next,sizeof(struct memblock_t)-sizeof(u_int64_t));
	e->prev->crc32=singletable_crc32c(0xFFFFFFFF,e->prev,sizeof(struct memblock_t)-sizeof(u_int64_t));

	return ((void*)tmp + sizeof(struct memblock_t ));
}
u_int64_t heap_get_used_blocks_count(void){
	struct memblock_t* tmp;
		tmp=start->next;
		int suma=0;
		if (start->size >0) suma++;
		while (tmp!=NULL)
		{
			if (tmp->size >0 ) suma++ ;
			tmp=tmp->next;
		}

	return suma;
}

u_int64_t heap_get_free_gaps_count(void){
	struct memblock_t* tmp;
		tmp=start->next;
		int suma=0;
		if (start->size <0 && (sizeof(void *)+64) <= abs(start->size)) suma++;
		while (tmp!=NULL)
		{
			if (tmp->size >0 && (sizeof(void *)+64) <= abs(tmp->size))  suma++ ;
			tmp=tmp->next;
		}

	return suma;

}

void view_heap()
{
		struct memblock_t* tmp;
		tmp=start;
		while (tmp!=NULL)
		{
		printf("add: %ld, size:%014ld prev:%014ld next:%014ld ",(intptr_t)tmp,(intptr_t)tmp->size,(intptr_t)tmp->prev,(intptr_t)tmp->next);
		#if DEBUG
		printf("Line:%d FileName:%s ",tmp->fileline,tmp->filename);
		#endif
		printf("crc:"BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(tmp->crc32));
tmp=tmp->next;
		
		}
	printf("end: %ld\n\n",(intptr_t)custom_sbrk(0));
}

void* heap_calloc(size_t number, size_t size_ori DEBUG_ARGS){
	size_t count=number*size_ori;
	if (count==0) return NULL;
	
	size_t rozmiar= heap_get_largest_free_area_wo();
	struct memblock_t  * tmp;
	if (rozmiar < count+sizeof(struct memblock_t))
	{
			if ((intptr_t)custom_sbrk(count+sizeof(struct memblock_t)+ADD_CAPACITY)==-1) return NULL;
			void *e =custom_sbrk(0);
			end->prev->size -=count + sizeof(struct memblock_t)+ADD_CAPACITY;

			tmp=end;
			end=(struct memblock_t *)(e - sizeof(struct memblock_t ));

			end->next=NULL;
			end->prev=tmp->prev;
			end->prev->next= end;
			end->size=0;
			#if DEBUG
			end->fileline=fileline;
			if(filename!=NULL)strncpy(end->filename,filename,30);
			end->prev->fileline=fileline;
			if(filename!=NULL)strncpy(end->prev->filename,filename,30);
			#endif
		
			end->prev->crc32=singletable_crc32c(0xFFFFFFFF,end->prev,sizeof(struct memblock_t)-sizeof(u_int64_t));
			end->crc32=singletable_crc32c(0xFFFFFFFF,end,sizeof(struct memblock_t)-sizeof(u_int64_t));	

	}
tmp=start;
	while (1)
		{
	if (tmp->size>0) {
			tmp=tmp->next;
			if (tmp == NULL) return NULL;
			continue;
		}     
		
	if (abs(tmp->size) >= count + sizeof(struct memblock_t) ) break;
			tmp=tmp->next;
			if (tmp == NULL) 	return NULL;
	}
	int size=tmp->size;
	tmp->size= count;
///
	struct memblock_t *e =(void *)tmp +( sizeof(struct memblock_t ) + count);
	
		
	e->next = tmp ->next;
	e->prev = tmp;
	tmp->next= e;
	e->next->prev=e;
	e->size= size+ count+sizeof(struct memblock_t);
	#if DEBUG
	e->prev->fileline=fileline;
	if(filename!=NULL)strncpy(e->prev->filename,filename,30);
	e->next->fileline=fileline;
	if(filename!=NULL)strncpy(e->next->filename,filename,30);
	e->fileline=fileline;
	if(filename!=NULL)strncpy(e->filename,filename,30);
	#endif
	e->crc32=singletable_crc32c(0xFFFFFFFF,e,sizeof(struct memblock_t)-sizeof(u_int64_t));
	e->next->crc32=singletable_crc32c(0xFFFFFFFF,e->next,sizeof(struct memblock_t)-sizeof(u_int64_t));
	e->prev->crc32=singletable_crc32c(0xFFFFFFFF,e->prev,sizeof(struct memblock_t)-sizeof(u_int64_t));

	int i=0;
	char * init = (void*)tmp + sizeof(struct memblock_t );
	while (i<number*size_ori) 
	{
		*(init+i)=0;
		i++;
	}

	return ((void*)tmp + sizeof(struct memblock_t ));

}

void heap_free(void* memblock)
{
	if (get_pointer_type(memblock)!=pointer_valid) return ;
	struct memblock_t *tmp= start;

	while(tmp!=NULL)
	{
		if((void*) tmp+sizeof(struct memblock_t) == (void*)memblock) 
		{
			tmp->size= tmp->size * -1;
			
			tmp->crc32=singletable_crc32c(0xFFFFFFFF,tmp,sizeof(struct memblock_t)-sizeof(u_int64_t));

			break;	
		}

		tmp=tmp->next;
	}	
	if(tmp==NULL) return ;
	tmp=start->next;
	int flag=0;
	while(tmp!=NULL)
	{
		if (tmp->size>0) {flag=0;tmp=tmp->next; continue;}
		if (tmp->size<=0 && flag==1 && (void*)tmp != (void*)end ) 
		{
			tmp->prev->size+=tmp->size-sizeof(struct memblock_t);
			tmp->prev->next=tmp->next;
			tmp->next->prev=tmp->prev;
			tmp=tmp->prev;

			tmp->crc32=singletable_crc32c(0xFFFFFFFF,tmp,sizeof(struct memblock_t)-sizeof(u_int64_t));
			tmp->next->crc32=singletable_crc32c(0xFFFFFFFF,tmp->next,sizeof(struct memblock_t)-sizeof(u_int64_t));
			flag=0;
			continue;
		}
		if(tmp->size<=0) {flag=1;tmp=tmp->next; continue;}
		tmp=tmp->next;
	}
	 if(end->prev->size<0)
		{
			struct memblock_t  * tmp;
			tmp=end;
			end=(struct memblock_t *)((void *)tmp->prev + sizeof(struct memblock_t ));

			end->next=NULL;
			end->prev=tmp->prev;
			end->prev->next= end;
			custom_sbrk(end->prev->size);
			end->size=0;
			end->prev->size=0;
			
			end->prev->crc32=singletable_crc32c(0xFFFFFFFF,end->prev,sizeof(struct memblock_t)-sizeof(u_int64_t));
			end->crc32=singletable_crc32c(0xFFFFFFFF,end,sizeof(struct memblock_t)-sizeof(u_int64_t));
	
			
		}


}

enum pointer_type_t get_pointer_type(const const void* pointer)
{
	if (pointer==NULL) return pointer_null;
	if (pointer <  (void *)start || pointer > (void *)end + sizeof(struct memblock_t)) return pointer_out_of_heap;
	struct memblock_t *tmp= start;
	struct memblock_t *compare= (struct memblock_t *)pointer;

	while(tmp!=NULL)
	{
		if (tmp==compare) return pointer_control_block;
		if (tmp<compare) {tmp=tmp->next; continue; }
	if (tmp>compare)
		{
			tmp=tmp->prev;
			if ((void*)tmp + sizeof(struct memblock_t) > (void*)compare) return pointer_control_block;
			if (tmp->size<0) return pointer_unallocated;
			if ((void*)tmp + sizeof(struct memblock_t) == (void*)compare) return pointer_valid;
			return pointer_inside_data_block;

		}
	}
return pointer_control_block;
}

void* heap_realloc(void* memblock, size_t size DEBUG_ARGS)
{
	if(memblock==NULL) return heap_malloc(size DEBUG_RUN_ARGS );
	if( size <= 0) return NULL;
	if(get_pointer_type(memblock) != pointer_valid) return NULL;
	struct memblock_t  * tmp;	
	tmp=(struct memblock_t *)(memblock - sizeof(struct memblock_t ));
	if (tmp->size == size)	return memblock;	
	if (tmp->size >= size+sizeof(struct memblock_t))
	{
		struct memblock_t  * tmp_vol2;
		tmp_vol2=(struct memblock_t *)(memblock + size);
		tmp_vol2->prev=tmp;
		tmp_vol2->next=tmp->next;
		tmp->next=tmp_vol2;
		tmp_vol2->next->prev=tmp_vol2;
		tmp_vol2->size=-1 * (tmp->size - size -sizeof(struct memblock_t));
		tmp->size=size;
		
		tmp->crc32=singletable_crc32c(0xFFFFFFFF,tmp,sizeof(struct memblock_t)-sizeof(u_int64_t));
		tmp_vol2->crc32=singletable_crc32c(0xFFFFFFFF,tmp_vol2,sizeof(struct memblock_t)-sizeof(u_int64_t));
	
		return 	memblock;
	}
	void * e= heap_malloc(size DEBUG_RUN_ARGS);
	if(e==NULL) return NULL;
	if(memcpy( memblock, e, tmp->size )==NULL) {heap_free(e); return NULL;}
	tmp->size *=-1;	
	
	tmp->crc32=singletable_crc32c(0xFFFFFFFF,tmp,sizeof(struct memblock_t)-sizeof(u_int64_t));
	
return e;


}

void* heap_get_data_block_start(const void* pointer)
{
		if (get_pointer_type(pointer) == pointer_valid) return (void *)pointer;
		if (get_pointer_type(pointer) == pointer_inside_data_block)
		{
			struct memblock_t *tmp= start;
			struct memblock_t *compare= (struct memblock_t *)pointer;
			
			while(tmp!=NULL)
			{
				if (tmp>compare)return (void *)tmp->prev + sizeof(struct memblock_t);
				tmp=tmp->next;
			}
		}
	return NULL;
}
size_t heap_get_block_size(const const void* memblock)
{
	if(get_pointer_type(memblock)!= pointer_valid) return 0;
	struct memblock_t  * tmp;	
	tmp=(struct memblock_t *)(memblock - sizeof(struct memblock_t ));
	return tmp->size;
}

int heap_validate(void)
{
	if(start==NULL || end==NULL ||start->size!= 0 || end->size!=0 || (void*)start->next == (void*) end) return -1;
	struct memblock_t  * tmp=start;	
	while(tmp!=NULL)
	{
		if((intptr_t)start>(intptr_t)tmp || (intptr_t)tmp>(intptr_t)end) return -1;
		if(tmp->crc32 != singletable_crc32c(0xFFFFFFFF,tmp,sizeof(struct memblock_t)-sizeof(u_int64_t))) return -1;
		tmp=tmp->next;
	}
	return 0;
}


int heap_reset( DEBUG_ARGS_WO)
{
	if(start==NULL || end==NULL ||start->size!= 0 || end->size!=0 || (void*)start->next == (void*) end) return -1;	
	intptr_t a = (intptr_t)start;
	intptr_t b = (intptr_t)custom_sbrk(0);
	if(a>b) return -1;
	intptr_t c=a-b;
	c=abs(c);
	if(start->crc32!=singletable_crc32c(0xFFFFFFFF,start,sizeof(struct memblock_t)-sizeof(u_int64_t)) ) return -1;
	if((intptr_t)custom_sbrk(-c)==-1) return -1;
	start=NULL;end=NULL;
	return heap_setup(DEBUG_RUN_ARGS_WO);
}

int main(int argc, char **argv)
{
	// Podstawowy
	int status = heap_setup(DEBUG_RUN_ARGS_WO);
	assert(status == 0);

	size_t free_bytes = heap_get_free_space();
	size_t used_bytes = heap_get_used_space();

	void* p1 = heap_malloc(8 * 1024 * 1024 DEBUG_RUN_ARGS); // 8MB
	void* p2 = heap_malloc(8 * 1024 * 1024 DEBUG_RUN_ARGS); // 8MB
	void* p3 = heap_malloc(8 * 1024 * 1024 DEBUG_RUN_ARGS); // 8MB
	void* p4 = heap_malloc(45 * 1024 * 1024 DEBUG_RUN_ARGS); // 45MB
	assert(p1 != NULL); // malloc musi się udać
	assert(p2 != NULL); // malloc musi się udać
	assert(p3 != NULL); // malloc musi się udać
	assert(p4 == NULL); // nie ma prawa zadziałać
  	status = heap_validate();
  	assert(status == 0); // sterta nie może być uszkodzona
	assert(heap_get_used_blocks_count() == 3);
	assert(
        heap_get_used_space() >= 24 * 1024 * 1024 &&
        heap_get_used_space() <= 24 * 1024 * 1024 + 2000
        );
	heap_free(p1);
	heap_free(p2);
	heap_free(p3);
	assert(heap_get_free_space() == free_bytes);
	assert(heap_get_used_space() == used_bytes);
	assert(heap_get_used_blocks_count() == 0);
	heap_reset(__FILE__, __LINE__);
	// Normalna alokacja i zwolnienie
	int * a[200];
	
	for(int i=0;i<200;i++)		a[i]=heap_malloc(sizeof(int) DEBUG_RUN_ARGS);
	for(int i=0;i<200;i++)		heap_free(a[i]);
	
	assert(heap_get_free_space() == free_bytes);
	assert(heap_get_used_space() == used_bytes);
	assert(heap_get_used_blocks_count() == 0);
	heap_reset(__FILE__, __LINE__);
	// Alokacja i odwrócone zwolnienie
	
	for(int i=0;i<200;i++)		a[i]=heap_malloc(sizeof(int) DEBUG_RUN_ARGS);
	for(int i=199;i>=0;i--)		heap_free(a[i]);
	
	assert(heap_get_free_space() == free_bytes);
	assert(heap_get_used_space() == used_bytes);
	assert(heap_get_used_blocks_count() == 0);
	heap_reset(__FILE__, __LINE__);
	
	// Alokacja i zwolnienie co drugiego
	for(int i=0;i<200;i++)			a[i]=heap_malloc(sizeof(int) DEBUG_RUN_ARGS);
	for(int i=0;i<200;i=i+2)		heap_free(a[i]);
	
	assert(heap_validate()==0);
	assert(heap_get_used_blocks_count()==100);
	assert(heap_get_free_space()==100*sizeof(int));
	heap_reset(DEBUG_RUN_ARGS_WO);
	// Alokacja i zwolnienie co drugiego a później co pierwszego
	for(int i=0;i<200;i++)			a[i]=heap_malloc(sizeof(int) DEBUG_RUN_ARGS);
	for(int i=0;i<200;i=i+2)		heap_free(a[i]);
	for(int i=1;i<200;i=i+2)		heap_free(a[i]);
	
	assert(heap_get_free_space() == free_bytes);
	assert(heap_get_used_space() == used_bytes);
	assert(heap_validate()==0);
	heap_reset( DEBUG_RUN_ARGS_WO);
	// Alokacja i zwolnienie co pierwszego a później co drugiego
	for(int i=0;i<200;i++)			a[i]=heap_malloc(sizeof(int) DEBUG_RUN_ARGS);
	for(int i=1;i<200;i=i+2)		heap_free(a[i]);
	for(int i=0;i<200;i=i+2)		heap_free(a[i]);
	assert(heap_validate()==0);
	assert(heap_get_free_space() == free_bytes);
	assert(heap_get_used_space() == used_bytes);
	heap_reset(DEBUG_RUN_ARGS_WO);
	//Uszkodzenie
	for(int i=0;i<2;i++)			a[i]=heap_malloc(sizeof(char) DEBUG_RUN_ARGS);
	char *e=heap_malloc(sizeof(char) DEBUG_RUN_ARGS);
	*(e+1)=12;
	assert(heap_validate()==-1);
	heap_reset(DEBUG_RUN_ARGS_WO);
	
	e=heap_malloc(sizeof(char) DEBUG_RUN_ARGS);
	*(e-1)=12;
	assert(heap_validate()==-1);
	heap_reset(DEBUG_RUN_ARGS_WO);
	
	e=heap_malloc(sizeof(char) DEBUG_RUN_ARGS);
	start=start+1;
	assert(heap_validate()==-1);
	start=start-1;
	heap_reset(DEBUG_RUN_ARGS_WO);
	
	
	return 0;
}
