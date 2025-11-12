#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int main(){
	int ptc[2];
	int cpt[2];
	char buf[1];
	int i;

	if(pipe(ptc)<0 || pipe(ctp)<0){
		fprintf(2,"piper decleration failed\n");
		exit(1);
	}
	int pid = fork();
	if (pid == 0){
		close(ptc[1]);	
		close(ctp[0]);
	}
	for (i=0;i<10;i++){
		read(ptc[0],buf,1);
		printf("Child recieved ping", i+1);
		printf("Child recieved pong", i+1);
		write(ctp[1],"x",1);
	}

	close(ptc[0]);
	close(ctp[1]);
	exit(0);

	for(i=0;i<10;i++){
		printf("Parent recieved ping",i+1);
		write(ptc[1],"x",1);
		read(ctp[0],buf,1);
		printf("Parent received Pong",i+1);
	}
	wait(0);
	close(ptc[1]);
	close(ctp[0]);
	exit(0);}
}
