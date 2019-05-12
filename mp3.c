
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
#define SHMKEY 11037
#include <ao/ao.h>
#include <mpg123.h>

#define BITS 8


typedef struct dt
{
    char path[1000];
    char mp3Name[100];
    int same;
}data;

data listMp3[1000];
int count = 0;
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
    for (int i = 0; i < count; i++)
    {
        printf("%d : %s\n", i, listMp3[i].mp3Name);
    }
    
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
    scanf("%d", &songNum);
	pthread_create(&(tid[0]),NULL,&player,NULL);


	while(1){
		int in;
		scanf("%d",&in);

		if(in==1)//play
			pthread_mutex_unlock(&play);
		if(in==2)//pause
			pthread_mutex_lock(&play);
        if(in==3){
            pthread_cancel(tmp_thread);
            songNum++;
            if(songNum >= count) songNum = 0;
            pthread_mutex_unlock(&play);
            pthread_create(&(tid[0]),NULL,&player,NULL);
            pthread_mutex_unlock(&play);
        }
        if(in==4){
            pthread_cancel(tmp_thread);
            songNum--;
            if(songNum <= 0) songNum = count - 1;
            pthread_mutex_unlock(&play);
            pthread_create(&(tid[0]),NULL,&player,NULL);
            pthread_mutex_unlock(&play);
        }
        if(in==5){
            listSong();
        }
        if(in==6){
            scanf("%d", &songNum);
            pthread_cancel(tmp_thread);
            pthread_mutex_unlock(&play);
            pthread_create(&(tid[0]),NULL,&player,NULL);
            pthread_mutex_unlock(&play);
        }

        printf("%d\n",songNum);
	}
	pthread_join(tid[0],NULL);
	return 0;
}