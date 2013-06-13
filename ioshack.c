#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysctl.h>
#include <errno.h>
#include <stdbool.h>
#include <mach/mach.h>
//#include  <mach/vm_map.h>
#include <assert.h>
#include "mio.h"

#define HELP "ps - show all proccess\nat pro_name - attach to one proccess\nsu - suspend the proccess\nre - resume the proccess\n"

typedef struct kinfo_proc kinfo_proc;

#define R_CODE_NONE -2
#define R_CODE_EXIT -1
#define R_CODE_INFO 0
#define R_CODE_PS 1
#define R_CODE_AT 2
#define R_CODE_SUS 3
#define R_CODE_RES 4
#define R_CODE_SSI 5
#define R_CODE_CSI 6
#define R_CODE_MOD 7

#define MAX_ADDS 1000

int getinput();
static int GetBSDProcessList();

static kinfo_proc *gprocList=NULL;
static size_t gprocCount;
static kinfo_proc *gproc=NULL;
static task_t gtask;
static char *gadds[MAX_ADDS+1];

int main(int argv,char *args[]){

	while(1){
		int r=getinput();

		if(r==R_CODE_EXIT)
			break;
		else if(r==R_CODE_INFO)
			printf("-help info:\n"HELP);
		else if(r==R_CODE_PS||r==R_CODE_AT){
			GetBSDProcessList();

			int i = 0;
			for (i = 0; i < gprocCount; i++) {
				kinfo_proc *pro = (gprocList + i);
				if(r==R_CODE_PS){
					printf("%d pid:%d name:%s user_stack:%p\n", i, pro->kp_proc.p_pid,
						pro->kp_proc.p_comm, pro->kp_proc.user_stack);
				}else{
					pid_t targetpid = pro->kp_proc.p_pid;
					char *a1=MioGetArgByIndex(1);
					if(strcmp(a1,pro->kp_proc.p_comm)==0){
						kern_return_t kr=task_for_pid(current_task(), targetpid, &gtask);
						if(kr==KERN_SUCCESS){
							printf("[attach proccess %s]\n",a1);
							gproc=pro;
						}else{
							printf("task_for_pid fail %d\n",kr);
							gproc=NULL;
						}
						break;
					}
				}
			}
		}else if(r==R_CODE_SUS){
			kern_return_t kr = task_suspend(gtask);
			if(kr==KERN_SUCCESS){
				printf("[suspend]\n");
			}else{
				printf("task_suspend fail %d\n",kr);
			}
		}else if(r==R_CODE_RES){
			kern_return_t kr = task_resume(gtask);
			if(kr==KERN_SUCCESS){
				printf("[resume]\n");
			}else{
				printf("task_resume fail %d\n",kr);
			}
		}else if(r==R_CODE_SSI){
			int num=-1;
			if(MioGetArg2Num(1,&num)!=0){
				printf("arg error");
				continue;
			}

			char *target_add=gproc->kp_proc.user_stack;
			printf("start search %d at %p=%lu\n",num,target_add,target_add);
			int index=0;
			do{
				int *buf;
				uint32_t sz;
				kern_return_t kr=vm_read(gtask,target_add,sizeof(int),&buf,&sz);
				if(kr!=KERN_SUCCESS){
					printf("error %d\n",kr);
					break;
				}

				if((*buf)==num){
					if(index<MAX_ADDS){
						printf("find the var at %p=%lu\n",target_add,target_add);
						gadds[index]=target_add;
						index++;
					}else{
						printf("gadds over flow\n");
					}
				}
				target_add=target_add-sizeof(int);
			}while(1);
			printf("there are %d vars\n",index);
			gadds[index+1]=-1;
			//end of start search int
		}else if(r==R_CODE_CSI){
			int num=-1;
			if(MioGetArg2Num(1,&num)!=0){
				printf("arg error");
				continue;
			}
			char *add=NULL;
			int index=0;
			while((add=gadds[index])!=-1){
				int *buf;
				uint32_t sz;
				kern_return_t kr=vm_read(gtask,add,sizeof(int),&buf,&sz);
				if(kr!=KERN_SUCCESS){
					printf("error %d\n",kr);
					break;
				}

				if((*buf)==num){
					printf("still find the var at %p\n",add);
					int t=0;
					char *tadd=NULL;
					while(1){
						tadd=gadds[t];
						if(tadd=-1){
							gadds[t]=add;
							break;
						}else{
							continue;
						}
					}	
					index++;
				}else{
					gadds[index]=-1;
					index++;
				}
			}
			printf("there are still %d vars\n",index+1);
			gadds[index+1]=-1;
		}else if(r==R_CODE_MOD){
			char *add=-1;
			if(MioGetArg2Long(1,&add)!=0){
				printf("address arg error");
				continue;
			}
			int num=-1;
			if(MioGetArg2Num(2,&num)!=0){
				printf("change to arg error");
				continue;
			}
			printf("mod %p to %d\n",add,num);
			kern_return_t kr=vm_write(gtask,add,(vm_offset_t)&num,sizeof(int));
			if(kr==KERN_SUCCESS){
				printf("OK!\n");
			}else{
				printf("vm_write fail %d\n",kr);
			}
		}

	}
	return 0;
}

int getinput(){
	int re_code=0;
	
	printf(">");
	MioGetArg();

	int r=MioGetArgCount();
	if(r==0)
		return R_CODE_NONE;
	char *cmd=MioGetArgByIndex(0);

	if(r==1&&strcmp(cmd,"q")==0){
		re_code=R_CODE_EXIT;
	}else if(r==1&&strcmp(cmd,"ps")==0){
		re_code=R_CODE_PS;
	}else if(r==2&&strcmp(cmd,"at")==0){
		re_code=R_CODE_AT;
	}else if(gproc!=NULL&&r==1&&strcmp(cmd,"su")==0){
		re_code=R_CODE_SUS;
	}else if(gproc!=NULL&&r==1&&strcmp(cmd,"re")==0){
		re_code=R_CODE_RES;
	}else if(gproc!=NULL&&r==2&&strcmp(cmd,"ssi")==0){
		re_code=R_CODE_SSI;
	}else if(gproc!=NULL&&r==2&&strcmp(cmd,"csi")==0){
		re_code=R_CODE_CSI;
	}else if(gproc!=NULL&&r==3&&strcmp(cmd,"mod")==0){
		re_code=R_CODE_MOD;
	}

	return re_code;
}

static int GetBSDProcessList()
	// Returns a list of all BSD processes on the system.  This routine
	// allocates the list and puts it in *procList and a count of the
	// number of entries in *procCount.  You are responsible for freeing
	// this list (use "free" from System framework).
	// On success, the function returns 0.
	// On error, the function returns a BSD errno value.
{
	if(gprocList!=NULL){
		free(gprocList);
		gprocList=NULL;
		gprocCount=0;
	}

	kinfo_proc **procList=&gprocList;
	size_t *procCount=&gprocCount;

	int err;
	kinfo_proc * result;
	bool done;
	static const int name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
	// Declaring name as const requires us to cast it when passing it to
	// sysctl because the prototype doesn't include the const modifier.
	size_t length;

	assert( procList != NULL);
	assert(*procList == NULL);
	assert(procCount != NULL);

	*procCount = 0;

	// We start by calling sysctl with result == NULL and length == 0.
	// That will succeed, and set length to the appropriate length.
	// We then allocate a buffer of that size and call sysctl again
	// with that buffer.  If that succeeds, we're done.  If that fails
	// with ENOMEM, we have to throw away our buffer and loop.  Note
	// that the loop causes use to call sysctl with NULL again; this
	// is necessary because the ENOMEM failure case sets length to
	// the amount of data returned, not the amount of data that
	// could have been returned.

	result = NULL;
	done = false;
	do {
		assert(result == NULL);

		// Call sysctl with a NULL buffer.

		length = 0;
		err = sysctl((int *) name, (sizeof(name) / sizeof(*name)) - 1, NULL,
				&length, NULL, 0);
		if (err == -1) {
			err = errno;
		}

		// Allocate an appropriately sized buffer based on the results
		// from the previous call.

		if (err == 0) {
			result = malloc(length);
			if (result == NULL ) {
				err = ENOMEM;
			}
		}

		// Call sysctl again with the new buffer.  If we get an ENOMEM
		// error, toss away our buffer and start again.

		if (err == 0) {
			err = sysctl((int *) name, (sizeof(name) / sizeof(*name)) - 1,
					result, &length, NULL, 0);
			if (err == -1) {
				err = errno;
			}
			if (err == 0) {
				done = true;
			} else if (err == ENOMEM) {
				assert(result != NULL);
				free(result);
				result = NULL;
				err = 0;
			}
		}
	} while (err == 0 && !done);

	// Clean up and establish post conditions.

	if (err != 0 && result != NULL ) {
		free(result);
		result = NULL;
	}
	*procList = result;
	if (err == 0) {
		*procCount = length / sizeof(kinfo_proc);
	}

	assert( (err == 0) == (*procList != NULL));

	return err;
}
