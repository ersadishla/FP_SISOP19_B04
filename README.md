# FP_SISOP_B04

## Ersad Ahmad Ishlahuddin
### 05111740000016
## Philip Antoni Siahaan
### 05111740000111

### Soal FUSE Thread
> Buatlah sebuah music player dengan bahasa C yang memiliki fitur play nama_lagu, pause, next, prev, list lagu. Selain music player juga terdapat FUSE untuk mengumpulkan semua jenis file yang berekstensi .mp3 kedalam FUSE yang tersebar pada direktori /home/user. Ketika FUSE dijalankan, direktori hasil FUSE hanya berisi file .mp3 tanpa ada direktori lain di dalamnya. Asal file tersebut bisa tersebar dari berbagai folder dan subfolder. program mp3 mengarah ke FUSE untuk memutar musik.
Note: playlist bisa banyak,

### Pemahaman Soal
> * Memodifikasi MP3 player dengan menambahkan fungsi-fungsi : play, pause, next, prev, dan menampilkan list lagu
> * Membuat FUSE untuk mengumpulkan semua file yang berekstensi .mp3 dari direktori /home/user ke dalam direktori FUSE

### Langkah-langkah Poin Pertama
#### 1. Membuat fungsi untuk mengumpulkan semua file yang berekstensi .mp3 ke dalam array
```c
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
```
```
variable path berisikan direktori FUSE, kemudian diiterasi dan dimasukkan ke dalam array listMp3.
``` 
#### 2. Memodifikasi untuk melakukan fungsi play dan pause
```c
if(in==1 && flag == 0){ //play
    pthread_mutex_unlock(&play);
    flag = 1;
}
if(in==2 && flag == 1){ //pause
    pthread_mutex_lock(&play);
    flag = 0;
}
```
```
Memanfaatkan mutex untuk melakukan lock dan unlock.
```
#### 3. Memodifikasi untuk melakukan fungsi untuk next dan prev
```c
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
```
```
Jika ada next atau prev, maka thread dicancel terlebih dahulu. Kemudian songNum ditambah/dikurangi, diunlock lagi kemudian membuat thread baru.
```
#### 4. Membuat fungsi untuk menampilkan playlist
```c
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
```
```
Iterasi mp3 yang ada di dalam array.
```
#### 5. Modifikasi tambahan untuk memilih lagu berdasarkan Song Number
```c
if(in==5){ //choose song
    listSong();
    printf("Choose the number of song : ");
    scanf("%d", &songNum);
    pthread_cancel(tmp_thread);
    pthread_mutex_unlock(&play);
    pthread_create(&(tid[0]),NULL,&player,NULL);
    pthread_mutex_unlock(&play);
}
```
```
Melakukan scan kembali untuk Song Number, kemudian membuat thread baru.
```
