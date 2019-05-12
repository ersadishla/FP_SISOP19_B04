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

> Berarti membuat 2 file berbeda, 1 untuk mp3 Player 1 lagi untuk FUSE nya


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
Jika ada next atau prev, maka thread dicancel terlebih dahulu.  
Kemudian songNum ditambah/dikurangi, diunlock lagi kemudian membuat thread baru.
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


### Langkah-langkah Poin Kedua
#### 1. Membuat struktur data yang tepat untuk menyimpan path dan nama file mp3
```c
typedef struct Node 
{ 
    char key[100];
    char lookfor[100];
    char mpath[1000]; 
    struct Node *left, *right, *down; 
}node; 
```
```
Menggunakan tree,  
key untuk menyimpan nama lagu,  
lookfor menyimpan /nama lagu,  
mpath untuk menyimpan direktori asalnya
```
#### 2. Pada saat readdir maka akan melakukan rekursi hanya mengambil file mp3
```c
static int xmp_readagain(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{

    // MOUNT POINT, Important
    if (strcmp(path, "/home/aeris/fp/mounted") == 0) return 0;

    int res = 0;
	DIR *dp;
	struct dirent *dir;

	(void) offset;
	(void) fi;

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((dir = readdir(dp)) != NULL) {
        if(dir->d_type != DT_DIR){ // if the type is not directory just print it with blue
            int length = strlen(dir->d_name);
            if (strncmp(dir->d_name + length - 4, ".mp3", 4) == 0){
                root = insert(root, dir->d_name, path);
                struct stat st;
                memset(&st, 0, sizeof(st));
                st.st_ino = dir->d_ino;
                st.st_mode = dir->d_type << 12;
                res = (filler(buf, dir->d_name, &st, 0));
                    if(res!=0) break;
            }
        }
        else if(dir->d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 && dir->d_name[0] != '.'){ // if it is a directory
            char d_path[1000]; // here I am using sprintf which is safer than strcat
            sprintf(d_path, "%s/%s", path, dir->d_name);
            xmp_readagain(d_path, buf, filler, offset, fi); // recall with the new path
        }
	}

	closedir(dp);

    return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
	if(strcmp(path, "/") == 0)
	{
		path = dirpath;
		sprintf(fpath, "%s", path);
	}
	else sprintf(fpath, "%s%s", dirpath, path);
	int res = 0;
    
    res = xmp_readagain(fpath, buf, filler, offset, fi);
	return 0;
}
```
```
Ada 2 fungsi untuk melakukan readdir  
xmp_readdir, untuk masuk ke dalam direktori  
xmp_readagain, untuk melakukan rekursi
Pada xmp_readagain
if pertama, jika tidak direktori dan merupakan file mp3 maka akan diinsert ke dalam tree,   
kemudian melakukan filler  
if kedua, jika direktori maka akan melakukan rekursi dengan path yang telah ditambah dengan dir->d_name
```
#### 3. Pada saat read getattr dan statfs melakukan pencarian path aslinya
```c
static int xmp_getattr(const char *path, struct stat *stbuf)
{
	char fpath[1000];
    char *temp;

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else {
        temp = search(root, path);
        sprintf(fpath, "%s", temp);
    }
    int res;
    res = lstat(fpath, stbuf);
    if (res == -1){
        return -errno;
    }

	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
    char fpath[1000];
    char *temp;
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else {
        temp = search(root, path);
        sprintf(fpath, "%s", temp);
    }
	int res = 0;
  	int fd = 0 ;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1){
		return -errno;
    }

	res = pread(fd, buf, size, offset);
	if (res == -1){
		res = -errno;
    }

	close(fd);
    return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;
    char fpath[1000];
    char *temp;
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else {
        temp = search(root, path);
        sprintf(fpath, "%s", temp);
    }
	res = statvfs(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}
```
```c
char *search(node* root, const char key[]) 
{ 
    // Base Cases: root is null or key is present at root 
    if (root == NULL)
        return "";

    if(strcmp(root->lookfor, key) == 0) 
        return root->mpath; 
     
    // Key is greater than root's key 
    if (strcmp(root->lookfor, key) < 0) 
       return search(root->right, key); 
  
    // Key is smaller than root's key 
    return search(root->left, key); 
} 
```
```
Setiap menjalankan fungsi di atas  
maka akan melakukan pencarian path aslinya dengan menggunakan fungsi search  
yang mengembalikan nilai path aslinya, jika file mp3 ada di tree
```
