#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <dirent.h>
#include <errno.h>
#include <ao/ao.h>
#include <mpg123.h>
#define SHMKEY 11037

#define BITS 8


typedef struct dt
{
    char path[1000];
    char mp3Name[100];
    int same;
}data;

data listMp3[1000];
int count = 0;
int flag = 0;
int songNum;

pthread_mutex_t play;
pthread_t tmp_thread;

void playlist(char path[]){
    	DIR *dp;
	struct dirent *dir;

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((dir = readdir(dp)) != NULL) {
        char fpath[10000];
        sprintf(fpath, "%s%s", path, dir->d_name);
        strcpy(listMp3[count].path, fpath); 
        strcpy(listMp3[count++].mp3Name, dir->d_name);
	}
	closedir(dp);
}

void listSong(){
    system("clear");
    printf("-------------------------------------\n");
    printf("No\t| Song Name\n");
    printf("-------------------------------------\n");
    for (int i = 0; i < count; i++)
    {
        printf("%d.\t| %s\n", i, listMp3[i].mp3Name);
    }
    printf("-------------------------------------\n");
    
}

void help(){
    system("clear");
    printf("-------------------------------------\n");
    printf("HELP\n");
    printf("-------------------------------------\n");
    printf("1. Play\n");
    printf("2. Pause\n");
    printf("3. Next\n");
    printf("4. Previous\n");
    printf("5. Choose Song\n");
    printf("8. List of Song\n");
    printf("9. Help\n");
}

void* player(void* input)
{   
    tmp_thread = pthread_self();  
    mpg123_handle *mh;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;
    int err;

    int driver;
    ao_device *dev;

    ao_sample_format format;
    int channels, encoding;
    long rate;

    /* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));
    /* open the file and get the decoding format */
    mpg123_open(mh, listMp3[songNum].path);
    
    // printf("%s",listMp3[nolagu].path);
    mpg123_getformat(mh, &rate, &channels, &encoding);

    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);

    /* decode and play */
    while (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK){
			pthread_mutex_lock(&play);
			ao_play(dev, buffer, done);
			pthread_mutex_unlock(&play);
		}

    /* clean up */
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();


}
int main(int argc, char **argv){
	char path[1000];
	strcpy(path, argv[1]);

	playlist(path);

	pthread_t tid[1];
	listSong();
	printf("Choose the number of song : ");
	scanf("%d", &songNum);
	printf("Now Playing : %d. %s\n", songNum, listMp3[songNum].mp3Name);
	printf("Type 9 For HELP\n");
	pthread_create(&(tid[0]),NULL,&player,NULL);


	while(1){
		int in;
		scanf("%d",&in);

		if(in==1 && flag == 0){ //play
			pthread_mutex_unlock(&play);
            		flag = 1;
        	}
		if(in==2 && flag == 1){ //pause
			pthread_mutex_lock(&play);
            		flag = 0;
        	}
		if(in==3){ //next
		    pthread_cancel(tmp_thread);
		    songNum++;
		    if(songNum >= count) songNum = 0;
		    pthread_mutex_unlock(&play);
		    pthread_create(&(tid[0]),NULL,&player,NULL);
		    pthread_mutex_unlock(&play);
		}
		if(in==4){ //prev
		    pthread_cancel(tmp_thread);
		    songNum--;
		    if(songNum <= 0) songNum = count - 1;
		    pthread_mutex_unlock(&play);
		    pthread_create(&(tid[0]),NULL,&player,NULL);
		    pthread_mutex_unlock(&play);
		}
		if(in==5){ //choose song
		    listSong();
		    printf("Choose the number of song : ");
		    scanf("%d", &songNum);
		    pthread_cancel(tmp_thread);
		    pthread_mutex_unlock(&play);
		    pthread_create(&(tid[0]),NULL,&player,NULL);
		    pthread_mutex_unlock(&play);
		}
		if(in==8){
		    listSong();
		}
		if(in==9){
		    help();
		}
		printf("Now Playing : %d. %s\n", songNum, listMp3[songNum].mp3Name);
		printf("Type 9 For HELP\n");
	}
	pthread_join(tid[0],NULL);
	return 0;
}
